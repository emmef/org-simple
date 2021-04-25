#ifndef ORG_SIMPLE_ARRAY_H
#define ORG_SIMPLE_ARRAY_H
/*
 * org-simple/util/BaseArray.h
 *
 * Added by michel on 2020-10-22
 * Copyright (C) 2015-2020 Michel Fleur.
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

#include <cstddef>
#include <org-simple/core/Index.h>
#include <org-simple/core/Size.h>
#include <org-simple/core/align.h>
#include <stdexcept>
#include <memory>

namespace org::simple::util {

template <typename T, size_t S, size_t A, typename C> class AbstractArray;
template <typename T, size_t A = 0> class ArrayDataRef;
template <typename T, size_t A = 0> class ArrayAllocated;
template <typename T, size_t S, size_t A = 0> class Array;

template <class T> class test_helper_is_base_array {

  template <typename T1, size_t S1, size_t A1, typename C1>
  static constexpr bool try_subst(const AbstractArray<T1, S1, A1, C1> *) {
    return true;
  }

  template <typename... A> static constexpr bool try_subst(...) {
    return false;
  }

public:
  static constexpr bool value = try_subst(static_cast<const T *>(nullptr));
};

template <class Array, bool = test_helper_is_base_array<Array>::value>
struct concept_base_array {
  static constexpr size_t FIXED_CAPACITY = 0;
  static constexpr size_t ALIGNAS = 0;
  static constexpr bool value = false;
  using data_type = void;

  template <typename Target> static constexpr bool compatible_type() {
    return false;
  }
};

template <class Array> struct concept_base_array<Array, true> {
  static constexpr size_t FIXED_CAPACITY = Array::FIXED_CAPACITY;
  static constexpr size_t ALIGNAS = Array::ALIGNAS;
  static constexpr bool value = true;
  using data_type = typename Array::data_type;

  template <typename Target>
  static constexpr bool is_type_compatible =
      std::is_nothrow_convertible_v<data_type, Target>;
};

template <class Array> concept is_base_array = concept_base_array<Array>::value;

template <class Array, typename T>
concept is_type_compat_array =
    concept_base_array<Array>::template is_type_compatible<T>;

template <class Array, typename T, size_t FIXED_CAPACITY>
concept is_type_compat_fixed_arrays =
    concept_base_array<Array>::template is_type_compatible<T>
            &&concept_base_array<Array>::FIXED_CAPACITY != 0 &&
    FIXED_CAPACITY != 0;

template <class Array, typename T, size_t FIXED_CAPACITY>
concept is_type_compat_same_size_arrays =
    concept_base_array<Array>::template is_type_compatible<T>
            &&FIXED_CAPACITY != 0 &&
    concept_base_array<Array>::FIXED_CAPACITY == FIXED_CAPACITY;

template <class Array, typename T, size_t FIXED_CAPACITY>
concept is_type_compat_le_size_arrays =
    concept_base_array<Array>::template is_type_compatible<T>
            &&FIXED_CAPACITY != 0 &&
    concept_base_array<Array>::FIXED_CAPACITY <= FIXED_CAPACITY;

template <class Array, typename T, size_t FIXED_CAPACITY>
concept is_type_compat_lt_size_arrays =
    concept_base_array<Array>::template is_type_compatible<T>
            &&FIXED_CAPACITY != 0 &&
    concept_base_array<Array>::FIXED_CAPACITY < FIXED_CAPACITY;

template <class Array, typename T, size_t FIXED_CAPACITY>
concept is_type_compat_ge_size_arrays =
    concept_base_array<Array>::template is_type_compatible<T>
            &&FIXED_CAPACITY != 0 &&
    concept_base_array<Array>::FIXED_CAPACITY >= FIXED_CAPACITY;

template <class Array, typename T, size_t FIXED_CAPACITY>
concept is_type_compat_gt_size_arrays =
    concept_base_array<Array>::template is_type_compatible<T>
            &&FIXED_CAPACITY != 0 &&
    concept_base_array<Array>::FIXED_CAPACITY > FIXED_CAPACITY;

using org::simple::core::alignment_is_valid;
using org::simple::core::Index;
using org::simple::core::SizeMetric;

/**
 * AbstractArray implements basic array behavior that a subclass can inherit. To
 * do that, the subclass must pass a delegate class as the last parameter \c C
 * and implement the following three methods: <table>
 *   <tr><td><code>array_capacity()</code></td><td>Returns the current capacity
 * of the array and is ignored if #FIXED_CAPACITY is non-zero. The method must
 * be const.</td></tr>
 *   <tr><td><code>array_data()</code></td><td>Returns the location of the data
 * with a const qualifier. The method must be declared const.</td></tr>
 *   <tr><td><code>array_data()</code></td><td>Returns the location of the data
 * without a const qualifier.</td></tr>
 * </table>
 * @tparam T The element type
 * @tparam S The fixed capacity or zero if the capacity is not fixed.
 * @tparam A The alignment value or zero if no specific alignment is required.
 * coincide with the data of another array, \c false otherwise.
 * @tparam C The delegate class that needs to implement the
 */
template <typename T, size_t S, size_t A, typename C> class AbstractArray {
  static_assert(A == 0 || alignment_is_valid<T>(A));
  static_assert(S == 0 || SizeMetric::Elements<sizeof(T)>::IsValid::value(S));

public:
  static constexpr size_t ALIGNAS =
      org::simple::core::alignment_get_correct<T>(A);
  static constexpr size_t FIXED_CAPACITY = S;
  template <size_t OFFSET>
  static constexpr size_t
      offset_alignment = ALIGNAS != 0 && (OFFSET % ALIGNAS == 0) ? ALIGNAS : 0;

  using data_type = T;
  using delegate_type = C;
  typedef SizeMetric::Elements<sizeof(T)> Size;

  // raw data access

  inline const T *begin() const {
    const T *ptr = static_cast<const delegate_type *>(this)->array_data();
    if constexpr (ALIGNAS != 0) {
      return std::assume_aligned<ALIGNAS, const T>(ptr);
    } else {
      return ptr;
    }
  }

  inline T *begin() {
    T *ptr = static_cast<delegate_type *>(this)->array_data();
    if constexpr (ALIGNAS != 0) {
      return std::assume_aligned<ALIGNAS, T>(ptr);
    }
    return ptr;
  }

  inline const T *end() const {
    if constexpr (ALIGNAS != 0 && FIXED_CAPACITY != 0 &&
                  (FIXED_CAPACITY % ALIGNAS == 0)) {
      return std::assume_aligned<ALIGNAS, const T>(begin() + FIXED_CAPACITY);
    }
    return begin() + capacity();
  }

  inline T *end() {
    if constexpr (ALIGNAS != 0 && FIXED_CAPACITY != 0 &&
                  (FIXED_CAPACITY % ALIGNAS == 0)) {
      return std::assume_aligned<ALIGNAS, T>(begin() + FIXED_CAPACITY);
    }
    return begin() + capacity();
  }

  // capacity

  size_t capacity() const {
    if constexpr (FIXED_CAPACITY != 0) {
      return FIXED_CAPACITY;
    } else {
      return static_cast<const delegate_type *>(this)->array_capacity();
    }
  }

  static constexpr bool is_valid_capacity(size_t capacity) {
    return Size::IsValid::value(capacity);
  }

  // Raw element access

  const T &data(size_t offset) const {
    return begin()[Index::unsafe(offset, capacity())];
  }
  T &data(size_t offset) { return begin()[Index::unsafe(offset, capacity())]; }
  const T &operator[](size_t offset) const { return data(offset); }
  T &operator[](size_t offset) { return data(offset); }

  // Offset-checked element access

  const T &at(size_t offset) const {
    return begin()[Index::safe(offset, capacity())];
  }
  T &at(size_t offset) { return begin()[Index::safe(offset, capacity())]; }

  /**
   * Copies all the elements from \c source to this array if they both have the
   * same capacity and returns \c true if that is the case. If a difference in
   * capacity can be established compile-time, compilation will fail.
   * @tparam Array The source array type
   * @param source The source array
   * @return \c true if copy was successful, \c false otherwise.
   */
  template <class Array>
  requires is_type_compat_array<Array, T> AbstractArray &assign(const Array &source) {
    if ((const void*)&source == (const void*)this) {
      return *this;
    }
    if constexpr (FIXED_CAPACITY != 0) {
      if constexpr (Array::FIXED_CAPACITY != 0) {
        static_assert(FIXED_CAPACITY == Array::FIXED_CAPACITY);
      } else if (FIXED_CAPACITY != source.capacity()) {
        throw std::invalid_argument(
            "org::simple::util::BaseArray<FIXED_CAPACITY>.assign(Array<dynamic "
            "capacity>)");
      }
    } else if constexpr (Array::FIXED_CAPACITY != 0) {
      if (capacity() != Array::FIXED_CAPACITY) {
        throw std::invalid_argument("org::simple::util::BaseArray<dynamic "
                                    "capacity>.assign(Array<FIXED_CAPACITY>)");
      }
    } else {
      if (capacity() != source.capacity()) {
        throw std::invalid_argument("org::simple::util::BaseArray<dynamic "
                                    "capacity>.assign(Array<FIXED_CAPACITY>)");
      }
      T *__restrict destination = begin();
      const T *__restrict src = source.begin();
      for (size_t i = 0; i < capacity(); i++) {
        destination[i] = src[i];
      }
      return *this;
    }
    const size_t MOVES =
        FIXED_CAPACITY != 0 ? FIXED_CAPACITY : Array::FIXED_CAPACITY;
    T *__restrict to = begin();
    const T *__restrict from = source.begin();
    for (size_t i = 0; i < MOVES; i++) {
      to[i] = from[i];
    }
    return *this;
  }

  /**
   * Copies the array \c source to position \c dest and returns \c true if that
   * was successful. Fails if \c source too large or the combination of \c dest
   * and the capacity of \c would exceed this array.
   * @tparam Array The array type.
   * @param dest The destination (offset) for the first element of the source.
   * @param source The source.array.
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <class Array>
  requires is_type_compat_array<Array, T> void copy_to(size_t dest,
                                                       const Array &source) {
    if (dest >= capacity() || this->capacity() - dest < source.capacity()) {
      throw std::invalid_argument(
          "org::simple::util::BaseArray.copy_to(dest, source_array)");
    }
    T *__restrict to = begin();
    const size_t end = dest + source.capacity();
    const T *__restrict wrapper = source.begin();
    for (size_t src = 0, dst = dest; dst < end; src++, dst++) {
      to[dst] = wrapper[src];
    }
  }

  /**
   * Copies the array \c source to position \c DEST. Compilation fails if \c
   * source too large or the combination of \c DEST and the capacity of \c
   * source would exceed this array
   * @tparam Array The array type.
   * @tparam DEST The destination (offset) for the first element of the source.
   * @param source The source.array.
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <size_t DEST, class Array>
  requires is_type_compat_fixed_arrays<Array, T, FIXED_CAPACITY> void
  copy_to(const Array &source) {
    static_assert(DEST < FIXED_CAPACITY &&
                  FIXED_CAPACITY - DEST >= Array::FIXED_CAPACITY);
    T *__restrict to = begin();
    const size_t end = DEST + Array::FIXED_CAPACITY;
    const T *__restrict from = source.begin();
    for (size_t src = 0, dst = DEST; dst < end; src++, dst++) {
      to[dst] = from[src];
    }
  }

  /**
   * Copies the the inclusive range \c start to \end from array \c source to
   * position \c dest and returns \c true if that was successful. Fails if \c
   * end is not smaller than the capacity of \c source, if \c start is larger
   * than end or if the combination of \c dest, \c start and the capacity of the
   * range would exceed this array.
   * @tparam Array The array type.
   * @param dest The destination (offset) for the first element of the source.
   * @param source The source array.
   * @param start The starting element to copy from the source array.
   * @param end The last element to copy from the source array.
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <typename Array>
  requires is_type_compat_array<Array, T> void
  copy_range_to(size_t dest, const Array &source, size_t start, size_t end) {
    if (end >= source.capacity() || end < start) {
      throw std::invalid_argument("org::simple::util::BaseArray.copy_range_to("
                                  "dest, source, !start, !end)");
    }
    if (dest >= capacity()) {
      throw std::invalid_argument("org::simple::util::BaseArray.copy_range_to(!"
                                  "dest, source, start, end)");
    }
    const size_t last = dest + (end - start);
    if (last >= capacity()) {
      throw std::invalid_argument("org::simple::util::BaseArray.copy_range_to(!"
                                  "dest, source, !start, !end)");
    }
    T *__restrict to = begin();
    const T *__restrict from = source.begin();

    for (size_t src = start, dst = dest; dst <= last; src++, dst++) {
      to[dst] = from[src];
    }
  }

  /**
   * Copies the the inclusive range \c start to \end from array \c source to
   * position \c dest and returns \c true if that was successful. Fails if \c
   * end is not smaller than the capacity of \c source, if \c start is larger
   * than end or if the combination of \c dest, \c start and the capacity of the
   * range would exceed this array.
   * @tparam Array The array type.
   * @tparam DEST The destination (offset) for the first element of the source.
   * @tparam START The starting element to copy from the source array.
   * @tparam END The last element to copy from the source array.
   * @param source The source array.
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <size_t DEST, size_t START, size_t END, typename Array>
  requires is_type_compat_fixed_arrays<Array, T, FIXED_CAPACITY> void
  copy_range_to(const Array &source) {
    static_assert(!(END >= Array::FIXED_CAPACITY || END < START));
    static_assert(DEST < FIXED_CAPACITY);
    static constexpr size_t last = DEST + (END - START);
    static_assert(last < FIXED_CAPACITY);
    T *__restrict to = begin();
    const T *__restrict from = source.begin();

    for (size_t src = START, dst = DEST; dst <= last; src++, dst++) {
      to[dst] = from[src];
    }
  }

  /**
   * Returns an Array whose elements reference the elements in the range that
   * starts at element \c start and ends at element \c end. Throws
   * std::out_of_range when the range exceeds the array boundaries.
   * @param start The first element to reference.
   * @param end The last element to reference.
   * @return an Array slice representing the range.
   */
  ArrayDataRef<T> range_ref(size_t start, size_t end) {
    if (end < capacity() && start <= end) {
      return ArrayDataRef<T>(begin() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }

  const ArrayDataRef<T> range_ref(size_t start, size_t end) const {
    if (end < capacity() && start <= end) {
      return ArrayDataRef<T>(begin() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }

  /**
   * Returns an Array whose elements reference the elements in the range that
   * starts at element \c START and ends at element \c END. Compile fails when
   * the range exceeds the array boundaries.
   * @tparam START The first element to reference.
   * @tparam END The last element to reference.
   * @return an Array slice representing the range.
   */
  template <size_t START, size_t END>
  requires(FIXED_CAPACITY !=
           0) ArrayDataRef<T, offset_alignment<START>> range_ref() {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return ArrayDataRef<T, offset_alignment<START>>(begin() + START,
                                                    END + 1 - START);
  }

  template <size_t START, size_t END>
  requires(FIXED_CAPACITY !=
           0) const ArrayDataRef<T, offset_alignment<START>> range_ref() const {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return ArrayDataRef<T, offset_alignment<START>>(begin() + START,
                                                    END + 1 - START);
  }

  /**
   * Returns an array whose elements are a copy of the elements in the range
   * that starts at element \c start and ends at element \c end. Throws
   * std::out_of_range if the range exceeds the array boundaries.
   * @param start The first element to copy.
   * @param end The last element to copy.
   * @return an Array representing the range.
   */
  ArrayAllocated<T, ALIGNAS> range_copy(size_t start, size_t end) const {
    if (end < capacity() && start <= end) {
      return ArrayAllocated<T, ALIGNAS>(begin() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }

  /**
   * Returns an array whose elements are a copy of the elements in the range
   * that starts at element \c START and ends at element \c END. Compilation
   * fails if the range exceeds the array boundaries.
   * @tparam START The first element to copy.
   * @tparam END The last element to copy.
   * @return an Array representing the range.
   */
  template <size_t START, size_t END>
  requires(FIXED_CAPACITY != 0)
      Array<T, END + 1 - START, offset_alignment<START>> range_copy_array()
          const {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return Array<T, END + 1 - START, offset_alignment<START>>(begin() + START);
  }

  /**
   * Returns an array whose elements are a copy of the elements in the range
   * that starts at element \c START and ends at element \c END. Compilation
   * fails if the range exceeds the array boundaries.
   * @tparam START The first element to copy.
   * @tparam END The last element to copy.
   * @return an Array representing the range.
   */
  template <size_t START, size_t END>
  requires(FIXED_CAPACITY !=
           0) const ArrayAllocated<T, ALIGNAS> range_copy_heap() const {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return ArrayAllocated<T, ALIGNAS>(begin() + START, END + 1 - START);
  }

  AbstractArray &operator<<(const T *source) {
    const T *__restrict src = core::Dereference::safe(source);
    T *__restrict dst = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      dst[i] += src[i];
    }
    return *this;
  }

  const AbstractArray &operator>>(T *destination) const {
    T *__restrict dst = core::Dereference::safe(destination);
    const T *__restrict src = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      dst[i] += src[i];
    }
    return *this;
  }
};

namespace helper {

template <typename T> static constexpr size_t eff_align(size_t A) {
  return A == 0 ? A : org::simple::core::alignment_get_correct<T>(A);
}

template <typename T, size_t S> static constexpr size_t eff_capacity() {
  static_assert(
      org::simple::core::SizeMetric::Elements<sizeof(T)>::IsValid::value(S));
  return S;
}

} // namespace helper

using namespace helper;

template <typename T, size_t S, size_t A>
class Array : public AbstractArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                                   Array<T, S, A>> {
  static_assert(std::is_trivially_constructible_v<T>);
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_move_assignable_v<T>);
  alignas(eff_align<T>(A)) T data_[S];

  T *array_data() { return &data_[0]; }
  const T *array_data() const { return &data_[0]; }

public:
  typedef AbstractArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                        Array<T, S, A>>
      Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;
  friend Super;

  Array() = default;
  Array(const Array &) = default;
  Array(Array &&) noexcept = default;

  template <class Array>
  requires is_type_compat_array<Array, T> Array(const Array &source) {
    this->assign(source);
  }

  Array(const T *source) { this->operator<<(source); }
};

template <typename T, size_t S, size_t A = 0>
class ArrayAllocatedFixedSize
    : public AbstractArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                           ArrayAllocatedFixedSize<T, S, A>> {
public:
  static_assert(std::is_trivially_constructible_v<T>);
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_move_assignable_v<T>);

  struct DataStruct {
    alignas(eff_align<T>(A)) T data_[S];
  };

  T *array_data() { return data_->data_; }
  const T *array_data() const { return data_->data_; }

public:
  typedef AbstractArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                        ArrayAllocatedFixedSize<T, S, A>>
      Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;
  friend Super;

  ArrayAllocatedFixedSize()
      : data_(new T[Super::SizeMetric::Valid::value(S)]) {}

  ArrayAllocatedFixedSize(const ArrayAllocatedFixedSize &source)
      : data_(new DataStruct) {
    for (size_t i = 0; i < Super::constSize(); i++) {
      data_[i] = source.data_[i];
    }
  }

  ArrayAllocatedFixedSize(ArrayAllocatedFixedSize &&source) noexcept
      : data_(source.data_) {
    source.data_ = nullptr;
  }

  template <class Array>
  requires is_type_compat_array<Array, T>
  ArrayAllocatedFixedSize(const Array &source) : ArrayAllocatedFixedSize() {
    this->assign(source);
  }

  ArrayAllocatedFixedSize(const T *source) : ArrayAllocatedFixedSize() {
    this->operator<<(source);
  }

  ~ArrayAllocatedFixedSize() {
    delete data_;
    data_ = nullptr;
  }

private:
  DataStruct *data_;
};

template <typename T, size_t S, size_t A = 0>
class ArrayDataRefFixedSize
    : public AbstractArray<T, eff_capacity<T, S>(), eff_align<A>(),
                           ArrayDataRefFixedSize<T, S>> {

  T *data_;

  T *array_data() { return data_; }
  const T *array_data() const { return data_; }

  T *check_valid_data(T *data) {
    T *r = core::Dereference::checked(data);
    if (A == 0 || core::alignment_matches((uintptr_t)data, eff_align<A>())) {
      return r;
    };
    throw std::invalid_argument(
        "org::simple::util::ArrayDataRefFixedSize(data): not aligned");
  }

public:
  typedef AbstractArray<T, eff_capacity<T, S>(), eff_align<A>(),
                        ArrayDataRefFixedSize<T, S>>
      Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;
  friend Super;

  ArrayDataRefFixedSize() = delete;
  ArrayDataRefFixedSize(T *data) : data_(check_valid_data(data)) {}
  ArrayDataRefFixedSize(const ArrayDataRefFixedSize &source)
      : data_(source.data_) {}
  ArrayDataRefFixedSize(ArrayDataRefFixedSize &&source) noexcept = default;
};

template <typename T, size_t A>
class ArrayDataRef
    : public AbstractArray<T, 0, eff_align<T>(A), ArrayDataRef<T>> {

  T *data_;
  size_t capacity_;

  size_t array_capacity() const { return capacity_; }
  T *array_data() { return data_; }
  const T *array_data() const { return data_; }

  T *check_valid_data(T *data) {
    T *r = core::Dereference::checked(data);
    if (A == 0 || core::alignment_matches((uintptr_t)data, eff_align<T>(A))) {
      return r;
    };
    throw std::invalid_argument(
        "org::simple::util::ArrayDataRef(data): not aligned");
  }

public:
  typedef AbstractArray<T, 0, eff_align<T>(A), ArrayDataRef<T>> Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  friend Super;

  ArrayDataRef() = delete;
  ArrayDataRef(T *data, size_t capacity)
      : data_(check_valid_data(data)),
        capacity_(Super::Size::Valid::value(capacity)) {}
  ArrayDataRef(const ArrayDataRef &source)
      : data_(source.data_), capacity_(source.capacity_) {}
  ArrayDataRef(ArrayDataRef &&source) noexcept = default;
};

template <typename T, size_t A>
class ArrayAllocated
    : public AbstractArray<T, 0, eff_align<T>(A), ArrayAllocated<T, A>> {
  static_assert(std::is_trivially_constructible_v<T>);
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_move_assignable_v<T>);

  size_t array_capacity() const { return data_.capacity(); }
  T *array_data() { return data_.data(); }
  const T *array_data() const { return data_.data(); }

  static constexpr T *allocate(size_t size) {
    const size_t sz = Size::Valid::value(size);
    if constexpr (Super::ALIGNAS > 0) {
      return new T[sz, Super::ALIGNAS];
    }
    return new T[sz];
  }

public:
  typedef AbstractArray<T, 0, eff_align<T>(A), ArrayAllocated<T, A>> Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  friend Super;

  ArrayAllocated(size_t size) : data_(size) {}

  ArrayAllocated(const T *source, size_t size) : data_(size) {
    T *to = this->begin();
    for (size_t i = 0; i < data_.capacity(); i++) {
      to[i] = source[i];
    }
  }

  template <class Array>
  requires is_type_compat_array<Array, T> ArrayAllocated(const Array &source)
      : data_(source.capacity()) {
    T *to = data_.data();
    const T *from = source.begin();
    for (size_t i = 0; i < data_.capacity(); i++) {
      to[i] = from[i];
    }
  }
  ArrayAllocated(ArrayAllocated &&source) noexcept
      : data_(std::move(source.data_)) {}

  ArrayAllocated(size_t capacity, const T *source) : ArrayAllocated(capacity) {
    this->operator<<(source);
  }

private:
  core::AlignedAlloc<T, eff_align<T>(A)> data_;
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_ARRAY_H
