#ifndef ORG_SIMPLE_ALIGNEDDATA_H
#define ORG_SIMPLE_ALIGNEDDATA_H
/*
 * org-simple/Memory.h
 *
 * Added by michel on 2024-01-04
 * Copyright (C) 2015-2024 Michel Fleur.
 * Source https://github.com/emmef/org-simple
 * Email org-simple@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AlignedAllocator.h"
#include "Index.h"
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <span>
#include <vector>

namespace org::simple {

// TODO Make this a constexpr instantiatable class instead of all these
// static constexpr members.

template <class Container> class AlignedContainerInfo {
  struct Void {};
  template <class Base> class AlignedAllocatorInfo {

    // SFINAE substitution checks to see if the class has a typedef called
    // allocator_type, used by \c has member.

    template <class X>
    [[maybe_unused]] static constexpr X::allocator_type
        allocator_type_subst(size_t);

    template <class>
    [[maybe_unused]] static constexpr Void allocator_type_subst(...);

    static constexpr bool has_ =
        !std::is_same_v<Void, decltype(allocator_type_subst<Base>(
                                  std::declval<size_t>()))>;

    template <class X>
    [[maybe_unused]] static constexpr X::value_type value_type_subst(size_t);

    template <class>
    [[maybe_unused]] static constexpr bool value_type_subst(...);

    /**
     * If the class Z has a typedef called allocator_type and that is an
     * AlignedAllocator, the alignment value of that allocator is set in
     * \c alignment. In all other cases, \c alignment will be zero.
     * @tparam Z The class to check.
     */
    template <class Z, bool = AlignedAllocatorInfo<Z>::has_>
    class GetAllocatorAlignmentValue {
    public:
      // In the case of no allocator_type, the result is always zero
      static constexpr size_t alignment = 0;
    };

    template <class Z> class GetAllocatorAlignmentValue<Z, true> {
      // SFINAE substitution checks to determine that the allocator_type
      // represents an org::simple::AlignedAllocator and if so, to return the
      // used alignment.
      template <class T, size_t AL>
      static constexpr size_t subst(const AlignedAllocator<T, AL> *) {
        return AL;
      }

      static constexpr size_t subst(...) { return 0; }
      typedef typename Z::allocator_type AllocatorType;

    public:
      static constexpr size_t alignment =
          subst(static_cast<const AllocatorType *>(nullptr));
    };

  public:
    static constexpr size_t alignment =
        GetAllocatorAlignmentValue<Base>::alignment;
    static constexpr bool isValid = has_ && alignment > 0;
    typedef decltype(allocator_type_subst<Base>(
        std::declval<size_t>())) valueType;
  };

  template <typename Sub> class AlignedArrayInfo {

    // SFINAE substitution checks to see if the class is a std::array an if so
    // to return its size.

    template <typename T, size_t N>
    static constexpr size_t getArraySize(const std::array<T, N> *) {
      return N;
    }

    static constexpr size_t getArraySize(...) { return 0; }

    template <typename T, size_t N>
    static constexpr T getArrayValueType(const std::array<T, N> *);

    static constexpr Void getArrayValueType(...);

  public:
    static constexpr size_t fixedSize =
        getArraySize(static_cast<const Sub *>(nullptr));
    static constexpr bool isValid = fixedSize > 0;
    typedef decltype(getArrayValueType(std::declval<const Sub *>())) valueType;
    static constexpr size_t alignment =
        isValid && alignof(Sub) >= alignof(valueType) ? alignof(Sub) : 0;
  };

  template <typename Sub,
            int select = AlignedArrayInfo<Container>::isValid       ? 1
                         : AlignedAllocatorInfo<Container>::isValid ? 2
                                                                    : 0>
  struct GetInfo;

  template <typename Sub>
  struct GetInfo<Sub, 0> {
    static constexpr bool isValid = false;
    static constexpr bool isArray = false;
    static constexpr bool hasAllocator = false;
    static constexpr size_t alignment = 0;
    static constexpr size_t fixedSize = 0;
    typedef Void valueType;
  };
  template <typename Sub>
  struct GetInfo<Sub, 1> {
    static constexpr bool isValid = true;
    static constexpr bool isArray = true;
    static constexpr bool hasAllocator = false;
    static constexpr size_t alignment = AlignedArrayInfo<Sub>::alignment;
    static constexpr size_t fixedSize = AlignedArrayInfo<Sub>::fixedSize;
    typedef typename AlignedArrayInfo<Sub>::valueType valueType;
  };
  template <typename Sub>
  struct GetInfo<Sub, 2> {
    static constexpr bool isValid = true;
    static constexpr bool isArray = false;
    static constexpr bool hasAllocator = true;
    static constexpr size_t alignment = AlignedAllocatorInfo<Sub>::alignment;
    static constexpr size_t fixedSize = 0;
    typedef typename AlignedAllocatorInfo<Sub>::valueType valueType;
  };

public:
  static constexpr bool isValid = GetInfo<Container>::isValid;
  static constexpr bool isArray = GetInfo<Container>::isArray;
  static constexpr bool hasAllocator = GetInfo<Container>::hasAllocator;
  static constexpr size_t alignment = GetInfo<Container>::alignment;
  static constexpr size_t fixedSize = GetInfo<Container>::fixedSize;
  typedef typename GetInfo<Container>::valueType valueType;
};

template <typename Type, size_t ALIGNMENT>
class AlignedVector
    : public std::vector<Type, AlignedAllocator<Type, ALIGNMENT>> {

public:
  typedef std::vector<Type, AlignedAllocator<Type, ALIGNMENT>> Base;
  typedef AlignedType<Type, ALIGNMENT> Alignment;

  AlignedVector() = default;

  AlignedVector(Base::size_type length, const Base::value_type &value)
      : Base(length, value) {}

  template <class Alloc>
  AlignedVector(const std::vector<typename Base::value_type, Alloc> &source)
      : Base(source) {}

  AlignedVector(AlignedVector &&original) noexcept = default;

  AlignedVector &operator=(const AlignedVector &source) {
    Base::operator=(source);
    return *this;
  }

  AlignedVector &operator=(AlignedVector &&original) {
    Base::operator=(static_cast<Base>(original));
    return *this;
  }

  AlignedVector &
  operator=(std::initializer_list<typename Base::value_type> list) {
    Base::operator=(list);
    return *this;
  }

  void assign(Base::size_type count, const Base::value_type &value) {
    Base::assign(count, value);
  }

  template <typename _InputIterator,
            typename = std::_RequireInputIter<_InputIterator>>
  void assign(_InputIterator first, _InputIterator last) {
    Base::assign(first, last);
  }

  void assign(std::initializer_list<typename Base::value_type> list) {
    Base::assign(list);
  }

  Base::pointer data() noexcept {
    return std::assume_aligned<Alignment::alignment>(Base::data());
  }

  Base::const_pointer data() const noexcept {
    return std::assume_aligned<Alignment::alignment>(Base::data());
  }

  Base::reference front() noexcept { return data(); }

  Base::const_reference front() const noexcept { return data(); }

  Base::iterator begin() noexcept { return iterator(data()); }

  Base::const_iterator begin() const noexcept { return const_iterator(data()); }

  Base::const_iterator cbegin() const noexcept { return begin(); }

  Base::reference operator[](Base::size_type i) noexcept { return data()[i]; }

  Base::const_reference operator[](Base::size_type i) const noexcept {
    return data()[i];
  }

  Base::reference at(Base::size_type i) noexcept {
    return data()[Index::checked(i, Base::size())];
  }

  Base::const_reference at(Base::size_type i) const noexcept {
    return data()[Index::checked(i, Base::size())];
  }

  // All the rest is inherited
};

template <typename T, size_t Size, size_t ALIGNMENT>
class alignas(ALIGNMENT) AlignedArray : public std::array<T, Size> {
  static_assert(Align::isValid<T>(ALIGNMENT));

public:
  typedef std::array<T, Size> Base;
  typedef AlignedType<T, ALIGNMENT> Alignment;

  template <typename... Args>
  AlignedArray(Args... args) : std::array<T, Size>(args...) {}

  AlignedArray() = default;
  AlignedArray(const AlignedArray &) = default;
  AlignedArray(AlignedArray &&) noexcept = default;

  Base::pointer data() noexcept {
    return std::assume_aligned<Alignment::alignment>(Base::data());
  }

  Base::const_pointer data() const noexcept {
    return std::assume_aligned<Alignment::alignment>(Base::data());
  }

  Base::reference front() noexcept { return data(); }

  Base::const_reference front() const noexcept { return data(); }

  Base::iterator begin() noexcept { return iterator(data()); }

  Base::const_iterator begin() const noexcept { return const_iterator(data()); }

  Base::const_iterator cbegin() const noexcept { return begin(); }

  Base::reference operator[](Base::size_type i) noexcept { return data()[i]; }

  Base::const_reference operator[](Base::size_type i) const noexcept {
    return data()[i];
  }

  Base::reference at(Base::size_type i) noexcept {
    return data()[Index::checked(i, Base::size())];
  }

  Base::const_reference at(Base::size_type i) const noexcept {
    return data()[Index::checked(i, Base::size())];
  }

  // all the rest is inherited
};

enum class AlignedAccessType { SUBCLASS, PARENT_POINTER, REFERENCE };
/**
 * Determines the value of alignment of the type with a member Alignment that
 * has that alignment as a size_t member, or zero if the types does not have
 * such a member.
 * @tparam Base The type to check.
 * @tparam isValidAlignedAccessBase
 */
template <typename T, size_t Size, size_t ALIGNMENT, typename Parent,
          AlignedAccessType accessType>
class AlignedAccess;

template <typename T, size_t Size, size_t ALIGNMENT, typename BASE>
class AlignedAccess<T, Size, ALIGNMENT, BASE, AlignedAccessType::SUBCLASS>
    : public BASE {
  static_assert(AlignedContainerInfo<BASE>::alignment == ALIGNMENT);

public:
  typedef AlignedType<T, ALIGNMENT> Alignment;
  typedef BASE Base;

  Base::pointer data() noexcept {
    return std::assume_aligned<Alignment::alignment>(Base::data());
  }

  Base::const_pointer data() const noexcept {
    return std::assume_aligned<Alignment::alignment>(Base::data());
  }
};

template <typename T, size_t Size, size_t ALIGNMENT> class AlignedPointer {
  static_assert(Align::isValid<T>(ALIGNMENT));

public:
  typedef std::array<T, Size> Base;
  typedef AlignedType<T, ALIGNMENT> Alignment;

  template <typename... Args>
  AlignedPointer(Args... args) : base_(new Base(args...)) {}

  AlignedPointer() = default;
  AlignedPointer(const AlignedPointer &) = default;
  AlignedPointer(AlignedPointer &&) noexcept = default;

  Base::pointer data() noexcept {
    return std::assume_aligned<Alignment::alignment>(base_.get().data());
  }

  Base::const_pointer data() const noexcept {
    return std::assume_aligned<Alignment::alignment>(base_.get().data());
  }

  Base::reference front() noexcept { return data(); }

  Base::const_reference front() const noexcept { return data(); }

  Base::iterator begin() noexcept { return iterator(data()); }

  Base::const_iterator begin() const noexcept { return const_iterator(data()); }

  Base::const_iterator cbegin() const noexcept { return begin(); }

  Base::reference operator[](Base::size_type i) noexcept { return data()[i]; }

  Base::const_reference operator[](Base::size_type i) const noexcept {
    return data()[i];
  }

  Base::reference at(Base::size_type i) noexcept {
    return data()[Index::checked(i, Base::size())];
  }

  Base::const_reference at(Base::size_type i) const noexcept {
    return data()[Index::checked(i, Base::size())];
  }

  Base &base() noexcept { return base_.get(); }

  const Base &base() const noexcept { return base_.get(); }
  // all the rest is inherited

private:
  std::unique_ptr<Base> base_;
};

// template <class T, size_t ELEMENTS, size_t ALIGNMENT, class Storage>
// struct AlignedAccess {
//   static constexpr size_t elements = ELEMENTS;
//   typedef T Type;
//   static constexpr size_t alignment = ALIGNMENT;
//
//   T *data() noexcept {
//     return std::assume_aligned<ALIGNMENT>(
//         static_cast<Storage *>(this)->getInternalStorage().data());
//   }
//
//   const T *data() const noexcept {
//     return std::assume_aligned<ALIGNMENT>(
//         static_cast<const Storage *>(this)->getInternalStorage().data());
//   }
//
//   T *begin() noexcept { return data(); }
//   const T *begin() const noexcept { return data(); }
//   T *end() noexcept { return data() + ELEMENTS; }
//   const T *end() const noexcept { return data() + ELEMENTS; }
//   T &operator[](size_t i) { return data()[i]; }
//   const T &operator[](size_t i) const { return data()[i]; }
//   T &at(size_t i) { return data()[Index::checked(i, ELEMENTS)]; }
//   const T &at(size_t i) const { return data()[Index::checked(i, ELEMENTS)]; }
//
//   [[nodiscard]] constexpr size_t capacity() const {
//     if constexpr (elements > 0) {
//       return elements;
//     } else {
//       static_cast<const Storage *>(this)->getInternalCapacity();
//     }
//   }
//   [[nodiscard]] constexpr size_t size() const { return capacity(); }
//
//   [[nodiscard]] constexpr size_t getAlignment() const { return alignment; }
//
//   template <class FROM, class TO>
//     requires(std::is_base_of_v<AlignedAccess, FROM> &&
//              std::is_base_of_v<AlignedAccess, TO>)
//   static constexpr inline void alignedStorageTypeCopy(const FROM &from,
//                                                       TO &to) noexcept {
//     std::copy(std::assume_aligned<ALIGNMENT>(from.getInternalStorage().begin()),
//               std::assume_aligned<ALIGNMENT>(from.getInternalStorage().end()),
//               std::assume_aligned<ALIGNMENT>(to.getInternalStorage().begin()));
//   }
//
//   template <class FROM, class TO>
//     requires(std::is_base_of_v<AlignedAccess, FROM> &&
//              std::is_base_of_v<AlignedAccess, TO>)
//   static constexpr inline void alignedStorageTypeMove(const FROM &from,
//                                                       TO &to) noexcept {
//     std::move(std::assume_aligned<ALIGNMENT>(from.getInternalStorage().begin()),
//               std::assume_aligned<ALIGNMENT>(from.getInternalStorage().end()),
//               std::assume_aligned<ALIGNMENT>(to.getInternalStorage().begin()));
//   }
// };
//
// enum class AlignedDataType { NONE, ARRAY, ALLOCATED_ARRAY, REFERENCE };
//
// template <class T, size_t ELEMENTS, size_t ALIGNMENT, AlignedDataType TYPE,
//           bool isConst>
// struct AlignedData;
//
// struct AlignedDataInfo {
//   bool isAlignedData = false;
//   size_t alignment = 0;
//   size_t elements = 0;
//   AlignedDataType type = AlignedDataType::NONE;
//   bool isConst = false;
//
//   template <class C> static constexpr AlignedDataInfo get() {
//     return check(static_cast<const C *>(nullptr));
//   }
//
//   template <class C> static constexpr bool is() {
//     return get<C>().isAlignedData;
//   }
//
// private:
//   static constexpr AlignedDataInfo check(...) { return {}; }
//
//   template <class T, size_t ELEMENTS, size_t ALIGNMENT, AlignedDataType TYPE,
//             bool isConst>
//   static constexpr AlignedDataInfo
//   check(const AlignedData<T, ELEMENTS, ALIGNMENT, TYPE, isConst> *const) {
//     return {true, ALIGNMENT, ELEMENTS, TYPE, isConst};
//   }
// };
//
// template <class T, size_t ELEMENTS, size_t ALIGNMENT>
// struct AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY, false>
//     : public AlignedAccess<
//           T, ELEMENTS, ALIGNMENT,
//           AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY, false>>
//           {
//   static constexpr size_t elements =
//       std::max(ELEMENTS, static_cast<size_t>(1u));
//   typedef T Type;
//   typedef AlignedAccess<
//       T, elements, ALIGNMENT,
//       AlignedData<T, elements, ALIGNMENT, AlignedDataType::ARRAY, false>>
//       Access;
//   typedef std::array<Type, elements> DataType;
//   using Access::alignment;
//
//   DataType &getInternalStorage() noexcept { return data_; }
//   const DataType &getInternalStorage() const noexcept { return data_; }
//
//   AlignedData() = default;
//
//   template <AlignedDataType type, bool isConst>
//   explicit AlignedData(const AlignedData<T, elements, alignment, type,
//   isConst>
//                            &source) noexcept {
//     Access::alignedStorageTypeCopy(source, *this);
//   }
//
//   AlignedData(AlignedData &&original) noexcept {
//     Access::alignedStorageTypeCopy(original, *this);
//   }
//
//   template <class Source> explicit AlignedData(const Source &source) {
//     if (source.size() != elements) {
//       throw std::invalid_argument(
//           "AlignedStorage: Source size does not match my size");
//     }
//     std::copy(source.begin(), source.end(), getInternalStorage().begin());
//   }
//
// private:
//   alignas(AlignedType<T, alignment>::alignment) DataType data_;
// };
//
// template <class T, size_t ELEMENTS, size_t ALIGNMENT>
// struct AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ALLOCATED_ARRAY,
//                    false>
//     : public AlignedAccess<
//           T, ELEMENTS, ALIGNMENT,
//           AlignedData<T, ELEMENTS, ALIGNMENT,
//           AlignedDataType::ALLOCATED_ARRAY,
//                       false>> {
//   static constexpr size_t elements = ELEMENTS;
//   typedef T Type;
//   typedef AlignedAccess<T, elements, ALIGNMENT,
//                         AlignedData<T, elements, ALIGNMENT,
//                                     AlignedDataType::ALLOCATED_ARRAY, false>>
//       Access;
//   using Access::alignment;
//   typedef typename AlignedData<T, ELEMENTS, ALIGNMENT,
//   AlignedDataType::ARRAY,
//                                false>::DataType DataType;
//
//   DataType &getInternalStorage() noexcept { return *data_.get(); }
//   const DataType &getInternalStorage() const noexcept { return *data_.get();
//   }
//
//   AlignedData() = default;
//
//   template <AlignedDataType type, bool isConst>
//   explicit AlignedData(
//       const AlignedData<T, elements, alignment, type, isConst> &source) {
//     Access::alignedStorageTypeCopy(source, *this);
//   }
//
//   AlignedData(AlignedData &&original) noexcept : data_(original.data_) {}
//
//   template <class Source> explicit AlignedData(const Source &source) {
//     if (source.size() != elements) {
//       throw std::invalid_argument(
//           "AlignedStorage: Source size does not match my size");
//     }
//     std::copy(source.begin(), source.end(), getInternalStorage().begin());
//   }
//
// private:
//   typedef AlignedData<T, elements, alignment, AlignedDataType::ARRAY, false>
//       InternalDataType;
//   std::unique_ptr<InternalDataType> data_ =
//       std::unique_ptr<InternalDataType>(new InternalDataType);
// };
//
// template <class T, size_t ELEMENTS, bool isConst> struct
// AlignedReferenceData; template <class T, size_t ELEMENTS> struct
// AlignedReferenceData<T, ELEMENTS, false> {
//   typedef std::span<T, ELEMENTS> DataType;
//
//   DataType data_;
//
//   explicit AlignedReferenceData(T *externalData)
//       : data_(create(externalData)) {}
//
//   void assign(T *externalData) { data_ = create(externalData); }
//
//   static DataType create(T *p) {
//     struct Iterator {
//       T *begin() { return p; }
//       T *end() { return p + ELEMENTS; }
//       T *p;
//     };
//     Iterator i = {p};
//     DataType d{i};
//     return d;
//   }
// };
//
// template <class T, size_t ELEMENTS>
// struct AlignedReferenceData<T, ELEMENTS, true> {
//   typedef std::span<const T, ELEMENTS> DataType;
//
//   DataType data_;
//
//   explicit AlignedReferenceData(const T *externalData)
//       : data_(create(externalData)) {}
//
//   void assign(const T *externalData) { data_ = create(externalData); }
//
//   static DataType create(T *p) {
//     struct Iterator {
//       T *begin() { return p; }
//       T *end() { return p + ELEMENTS; }
//       T *p;
//     };
//     Iterator i = {p};
//     DataType d{i};
//     return d;
//   }
// };
//
// template <class T, size_t ELEMENTS, size_t ALIGNMENT, bool isConst>
// struct AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE,
// isConst>
//     : public AlignedAccess<T, ELEMENTS, ALIGNMENT,
//                            AlignedData<T, ELEMENTS, ALIGNMENT,
//                                        AlignedDataType::REFERENCE, isConst>>,
//       private AlignedReferenceData<T, ELEMENTS, isConst> {
//
//   static constexpr size_t elements = ELEMENTS;
//   typedef T Type;
//   typedef AlignedAccess<
//       T, ELEMENTS, ALIGNMENT,
//       AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE,
//       isConst>> Access;
//   typedef
//       typename AlignedReferenceData<T, ELEMENTS, isConst>::DataType DataType;
//   template <size_t ownerElements, AlignedDataType type, bool constOwner>
//   using OwnerStorage =
//       AlignedAccess<T, ownerElements, ALIGNMENT,
//                     AlignedData<T, ownerElements, ALIGNMENT, type,
//                     constOwner>>;
//
//   DataType &getInternalStorage() noexcept {
//     return AlignedReferenceData<T, ELEMENTS, isConst>::data_;
//   }
//   const DataType &getInternalStorage() const noexcept {
//     return AlignedReferenceData<T, ELEMENTS, isConst>::data_;
//   }
//
//   template <class O> static constexpr bool isAssignableOwner() {
//     const AlignedDataInfo info = AlignedDataInfo::get<O>();
//     if (!info.isAlignedData) {
//       return false;
//     }
//     const bool sameAligned = info.alignment == ALIGNMENT;
//     const bool validElements = info.elements >= ELEMENTS;
//     return sameAligned && validElements && (!info.isConst || isConst);
//   }
//
//   template <class O>
//   static constexpr bool isValidElementIndex(const size_t elementIndex) {
//     const auto ownerElements = AlignedDataInfo::get<O>().elements;
//     return ownerElements >= ELEMENTS &&
//            ownerElements - ELEMENTS >= elementIndex &&
//            AlignedType<T, ALIGNMENT>::alignedElements() % elementIndex == 0;
//   }
//
//   template <typename P> static constexpr bool isConstPointer() {
//     return std::is_same_v<const T *, P>;
//   }
//
//   template <typename P> static constexpr bool isNonConstPointer() {
//     return std::is_same_v<T *, P>;
//   }
//
//   template <typename P> static constexpr bool isPointerAssignable() {
//     return isNonConstPointer<P>() ? true : isConstPointer<P>() && isConst;
//   }
//
//   template <typename P>
//     requires(isPointerAssignable<P>())
//   explicit AlignedData(P pointer)
//       : AlignedReferenceData<T, ELEMENTS, isConstPointer<P>()>(
//             {checkAligned(pointer)}) {}
//
//   template <typename P>
//     requires(isPointerAssignable<P>() && !isConst)
//   void set(P pointer) {
//     AlignedReferenceData<T, ELEMENTS,
//     isConst>::assign(checkAligned(pointer));
//   }
//
//   template <class O>
//     requires(isAssignableOwner<O>())
//   AlignedData(O &owner, size_t elementIndex)
//       : AlignedReferenceData<T, ELEMENTS, isConst>(
//             owner.data() + checkValidElementIndex<O>(elementIndex)) {}
//
//   template <class O>
//     requires(isAssignableOwner<O>() && !isConst)
//   void set(O &owner, size_t elementIndex) {
//     AlignedReferenceData<T, ELEMENTS, isConst>::data_ =
//         owner.data() + isValidElementIndex<O>(elementIndex);
//   }
//
// private:
//   template <class O>
//     requires(AlignedDataInfo::get<O>().isAlignedData)
//   static constexpr bool isValidOwnerElements() {
//     return AlignedDataInfo::get<O>().elements >= ELEMENTS;
//   }
//
//   template <typename P> static constexpr P checkAligned(P pointer) {
//     if (AlignedType<T, ALIGNMENT>::isAlignedPointer(pointer)) {
//       return pointer;
//     }
//     throw std::invalid_argument("AlignedStorage: pointer is not aligned");
//   }
//
//   template <class O>
//   static constexpr size_t checkValidElementIndex(size_t elementIndex) {
//     if (isValidElementIndex<O>(elementIndex)) {
//       return elementIndex;
//     }
//     throw std::invalid_argument(
//         "AlignedStorage: elementIndex not on aligned boundary");
//   }
// };
//
// template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
// using AlignedAllocatedStorage =
//     AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ALLOCATED_ARRAY,
//                 false>;
//
// template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
// using AlignedLocalStorage =
//     AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY, false>;
//
// template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
// using AlignedReferencedStorage =
//     AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE, false>;
//
// template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
// using AlignedConstReferencedStorage =
//     AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE, true>;

} // namespace org::simple

#endif // ORG_SIMPLE_ALIGNEDDATA_H
