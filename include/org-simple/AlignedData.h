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

#include "Align.h"
#include "Index.h"
#include <algorithm>
#include <iterator>
#include <memory>
#include <span>

namespace org::simple {

template <class T, size_t ELEMENTS, size_t ALIGNMENT, class Storage>
struct AlignedAccess {
  static constexpr size_t elements = ELEMENTS;
  typedef T Type;
  static constexpr size_t alignment = ALIGNMENT;

  T *data() noexcept {
    return std::assume_aligned<ALIGNMENT>(
        static_cast<Storage *>(this)->getInternalStorage().data());
  }

  const T *data() const noexcept {
    return std::assume_aligned<ALIGNMENT>(
        static_cast<const Storage *>(this)->getInternalStorage().data());
  }

  T *begin() noexcept { return data(); }
  const T *begin() const noexcept { return data(); }
  T *end() noexcept { return data() + ELEMENTS; }
  const T *end() const noexcept { return data() + ELEMENTS; }
  T &operator[](size_t i) { return data()[i]; }
  const T &operator[](size_t i) const { return data()[i]; }
  T &at(size_t i) { return data()[Index::checked(i, ELEMENTS)]; }
  const T &at(size_t i) const { return data()[Index::checked(i, ELEMENTS)]; }

  [[nodiscard]] constexpr size_t capacity() const {
    if constexpr (elements > 0) {
      return elements;
    } else {
      static_cast<const Storage *>(this)->getInternalCapacity();
    }
  }
  [[nodiscard]] constexpr size_t size() const { return capacity(); }

  [[nodiscard]] constexpr size_t getAlignment() const { return alignment; }

  template <class FROM, class TO>
    requires(std::is_base_of_v<AlignedAccess, FROM> &&
             std::is_base_of_v<AlignedAccess, TO>)
  static constexpr inline void alignedStorageTypeCopy(const FROM &from,
                                                      TO &to) noexcept {
    std::copy(std::assume_aligned<ALIGNMENT>(from.getInternalStorage().begin()),
              std::assume_aligned<ALIGNMENT>(from.getInternalStorage().end()),
              std::assume_aligned<ALIGNMENT>(to.getInternalStorage().begin()));
  }

  template <class FROM, class TO>
    requires(std::is_base_of_v<AlignedAccess, FROM> &&
             std::is_base_of_v<AlignedAccess, TO>)
  static constexpr inline void alignedStorageTypeMove(const FROM &from,
                                                      TO &to) noexcept {
    std::move(std::assume_aligned<ALIGNMENT>(from.getInternalStorage().begin()),
              std::assume_aligned<ALIGNMENT>(from.getInternalStorage().end()),
              std::assume_aligned<ALIGNMENT>(to.getInternalStorage().begin()));
  }
};

enum class AlignedDataType { NONE, ARRAY, ALLOCATED_ARRAY, REFERENCE };

template <class T, size_t ELEMENTS, size_t ALIGNMENT, AlignedDataType TYPE,
          bool isConst>
struct AlignedData;

struct AlignedDataInfo {
  bool isAlignedData = false;
  size_t alignment = 0;
  size_t elements = 0;
  AlignedDataType type = AlignedDataType::NONE;
  bool isConst = false;

  template <class C> static constexpr AlignedDataInfo get() {
    return check(static_cast<const C *>(nullptr));
  }

  template <class C> static constexpr bool is() {
    return get<C>().isAlignedData;
  }

private:
  static constexpr AlignedDataInfo check(...) { return {}; }

  template <class T, size_t ELEMENTS, size_t ALIGNMENT, AlignedDataType TYPE,
            bool isConst>
  static constexpr AlignedDataInfo
  check(const AlignedData<T, ELEMENTS, ALIGNMENT, TYPE, isConst> *const) {
    return {true, ALIGNMENT, ELEMENTS, TYPE, isConst};
  }
};

template <class T, size_t ELEMENTS, size_t ALIGNMENT>
struct AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY, false>
    : public AlignedAccess<
          T, ELEMENTS, ALIGNMENT,
          AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY, false>> {
  static constexpr size_t elements =
      std::max(ELEMENTS, static_cast<size_t>(1u));
  typedef T Type;
  typedef AlignedAccess<
      T, elements, ALIGNMENT,
      AlignedData<T, elements, ALIGNMENT, AlignedDataType::ARRAY, false>>
      Access;
  typedef std::array<Type, elements> DataType;
  using Access::alignment;

  DataType &getInternalStorage() noexcept { return data_; }
  const DataType &getInternalStorage() const noexcept { return data_; }

  AlignedData() = default;

  template <AlignedDataType type, bool isConst>
  explicit AlignedData(const AlignedData<T, elements, alignment, type, isConst>
                           &source) noexcept {
    Access::alignedStorageTypeCopy(source, *this);
  }

  AlignedData(AlignedData &&original) noexcept {
    Access::alignedStorageTypeCopy(original, *this);
  }

  template <class Source> explicit AlignedData(const Source &source) {
    if (source.size() != elements) {
      throw std::invalid_argument(
          "AlignedStorage: Source size does not match my size");
    }
    std::copy(source.begin(), source.end(), getInternalStorage().begin());
  }

private:
  alignas(AlignedType<T, alignment>::alignment) DataType data_;
};

template <class T, size_t ELEMENTS, size_t ALIGNMENT>
struct AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ALLOCATED_ARRAY,
                   false>
    : public AlignedAccess<
          T, ELEMENTS, ALIGNMENT,
          AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ALLOCATED_ARRAY,
                      false>> {
  static constexpr size_t elements = ELEMENTS;
  typedef T Type;
  typedef AlignedAccess<T, elements, ALIGNMENT,
                        AlignedData<T, elements, ALIGNMENT,
                                    AlignedDataType::ALLOCATED_ARRAY, false>>
      Access;
  using Access::alignment;
  typedef typename AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY,
                               false>::DataType DataType;

  DataType &getInternalStorage() noexcept { return *data_.get(); }
  const DataType &getInternalStorage() const noexcept { return *data_.get(); }

  AlignedData() = default;

  template <AlignedDataType type, bool isConst>
  explicit AlignedData(
      const AlignedData<T, elements, alignment, type, isConst> &source) {
    Access::alignedStorageTypeCopy(source, *this);
  }

  AlignedData(AlignedData &&original) noexcept : data_(original.data_) {}

  template <class Source> explicit AlignedData(const Source &source) {
    if (source.size() != elements) {
      throw std::invalid_argument(
          "AlignedStorage: Source size does not match my size");
    }
    std::copy(source.begin(), source.end(), getInternalStorage().begin());
  }

private:
  typedef AlignedData<T, elements, alignment, AlignedDataType::ARRAY, false>
      InternalDataType;
  std::unique_ptr<InternalDataType> data_ =
      std::unique_ptr<InternalDataType>(new InternalDataType);
};

template <class T, size_t ELEMENTS, bool isConst> struct AlignedReferenceData;
template <class T, size_t ELEMENTS>
struct AlignedReferenceData<T, ELEMENTS, false> {
  typedef std::span<T, ELEMENTS> DataType;

  DataType data_;

  explicit AlignedReferenceData(T *externalData)
      : data_(create(externalData)) {}

  void assign(T *externalData) { data_ = create(externalData); }

  static DataType create(T *p) {
    struct Iterator {
      T *begin() { return p; }
      T *end() { return p + ELEMENTS; }
      T *p;
    };
    Iterator i = {p};
    DataType d{i};
    return d;
  }
};

template <class T, size_t ELEMENTS>
struct AlignedReferenceData<T, ELEMENTS, true> {
  typedef std::span<const T, ELEMENTS> DataType;

  DataType data_;

  explicit AlignedReferenceData(const T *externalData)
      : data_(create(externalData)) {}

  void assign(const T *externalData) { data_ = create(externalData); }

  static DataType create(T *p) {
    struct Iterator {
      T *begin() { return p; }
      T *end() { return p + ELEMENTS; }
      T *p;
    };
    Iterator i = {p};
    DataType d{i};
    return d;
  }
};

template <class T, size_t ELEMENTS, size_t ALIGNMENT, bool isConst>
struct AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE, isConst>
    : public AlignedAccess<T, ELEMENTS, ALIGNMENT,
                           AlignedData<T, ELEMENTS, ALIGNMENT,
                                       AlignedDataType::REFERENCE, isConst>>,
      private AlignedReferenceData<T, ELEMENTS, isConst> {

  static constexpr size_t elements = ELEMENTS;
  typedef T Type;
  typedef AlignedAccess<
      T, ELEMENTS, ALIGNMENT,
      AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE, isConst>>
      Access;
  typedef
      typename AlignedReferenceData<T, ELEMENTS, isConst>::DataType DataType;
  template <size_t ownerElements, AlignedDataType type, bool constOwner>
  using OwnerStorage =
      AlignedAccess<T, ownerElements, ALIGNMENT,
                    AlignedData<T, ownerElements, ALIGNMENT, type, constOwner>>;

  DataType &getInternalStorage() noexcept {
    return AlignedReferenceData<T, ELEMENTS, isConst>::data_;
  }
  const DataType &getInternalStorage() const noexcept {
    return AlignedReferenceData<T, ELEMENTS, isConst>::data_;
  }

  template <class O> static constexpr bool isAssignableOwner() {
    const AlignedDataInfo info = AlignedDataInfo::get<O>();
    if (!info.isAlignedData) {
      return false;
    }
    const bool sameAligned = info.alignment == ALIGNMENT;
    const bool validElements = info.elements >= ELEMENTS;
    return sameAligned && validElements && (!info.isConst || isConst);
  }

  template <class O>
  static constexpr bool isValidElementIndex(const size_t elementIndex) {
    const auto ownerElements = AlignedDataInfo::get<O>().elements;
    return ownerElements >= ELEMENTS &&
           ownerElements - ELEMENTS >= elementIndex &&
           AlignedType<T, ALIGNMENT>::alignedElements() % elementIndex == 0;
  }

  template <typename P> static constexpr bool isConstPointer() {
    return std::is_same_v<const T *, P>;
  }

  template <typename P> static constexpr bool isNonConstPointer() {
    return std::is_same_v<T *, P>;
  }

  template <typename P> static constexpr bool isPointerAssignable() {
    return isNonConstPointer<P>() ? true : isConstPointer<P>() && isConst;
  };

  template <typename P>
    requires(isPointerAssignable<P>())
  explicit AlignedData(P pointer)
      : AlignedReferenceData<T, ELEMENTS, isConstPointer<P>()>(
            {checkAligned(pointer)}) {}

  template <typename P>
    requires(isPointerAssignable<P>() && !isConst)
  void set(P pointer) {
    AlignedReferenceData<T, ELEMENTS, isConst>::assign(checkAligned(pointer));
  }

  template <class O>
    requires(isAssignableOwner<O>())
  AlignedData(O &owner, size_t elementIndex)
      : AlignedReferenceData<T, ELEMENTS, isConst>(
            owner.data() + checkValidElementIndex<O>(elementIndex)) {}

  template <class O>
    requires(isAssignableOwner<O>() && !isConst)
  void set(O &owner, size_t elementIndex) {
    AlignedReferenceData<T, ELEMENTS, isConst>::data_ =
        owner.data() + isValidElementIndex<O>(elementIndex);
  }

private:
  template <class O>
    requires(AlignedDataInfo::get<O>().isAlignedData)
  static constexpr bool isValidOwnerElements() {
    return AlignedDataInfo::get<O>().elements >= ELEMENTS;
  }

  template <typename P> static constexpr P checkAligned(P pointer) {
    if (AlignedType<T, ALIGNMENT>::isAlignedPointer(pointer)) {
      return pointer;
    }
    throw std::invalid_argument("AlignedStorage: pointer is not aligned");
  }

  template <class O>
  static constexpr size_t checkValidElementIndex(size_t elementIndex) {
    if (isValidElementIndex<O>(elementIndex)) {
      return elementIndex;
    }
    throw std::invalid_argument(
        "AlignedStorage: elementIndex not on aligned boundary");
  }
};

template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
using AlignedAllocatedStorage =
    AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ALLOCATED_ARRAY,
                false>;

template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
using AlignedLocalStorage =
    AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::ARRAY, false>;

template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
using AlignedReferencedStorage =
    AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE, false>;

template <typename T, size_t ELEMENTS, size_t ALIGNMENT = alignof(T)>
using AlignedConstReferencedStorage =
    AlignedData<T, ELEMENTS, ALIGNMENT, AlignedDataType::REFERENCE, true>;

} // namespace org::simple

#endif // ORG_SIMPLE_ALIGNEDDATA_H
