#ifndef ORG_SIMPLE_BASEARRAY_H
#define ORG_SIMPLE_BASEARRAY_H
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
#include <org-simple/core/Align.h>
#include <org-simple/core/Index.h>
#include <org-simple/core/Size.h>
#include <stdexcept>

namespace org::simple::util {

template <typename T, size_t S, size_t A, typename C> class BaseArray;
template <typename T> class ArraySlice;
template <typename T, size_t A = 0> class ArrayHeap;

template <class T> class BaseArrayTestHelper {

  template <typename T1, size_t S1, size_t A1, typename C1>
  static constexpr bool try_subst(const BaseArray<T1, S1, A1, C1> *) {
    return true;
  }

  template <typename... A> static constexpr bool try_subst(...) {
    return false;
  }

public:
  static constexpr bool value = try_subst(static_cast<const T *>(nullptr));
};

template <class T, bool = BaseArrayTestHelper<T>::value> class BaseArrayTest {
public:
  static constexpr size_t FIXED_CAPACITY = 0;
  static constexpr size_t ALIGNAS = 0;
  static constexpr bool value = false;
  using data_type = void;

  template <typename Target> static constexpr bool compatible() {
    return false;
  }
};

template <class T> class BaseArrayTest<T, true> {

public:
  static constexpr size_t FIXED_CAPACITY = T::FIXED_CAPACITY;
  static constexpr size_t ALIGNAS = T::ALIGNAS;
  static constexpr bool value = true;
  using data_type = typename T::data_type;

  template <typename Target> static constexpr bool compatible() {
    return std::is_nothrow_convertible_v<data_type, Target>;
  }
};

template <class Array, typename T>
concept ArrayCompatible = BaseArrayTest<Array>::template compatible<T>();

template <class Array, typename T, size_t FIXED_CAPACITY>
concept ArrayFixedCompatible =
    BaseArrayTest<Array>::template compatible<T>() &&
    BaseArrayTest<Array>::FIXED_CAPACITY != 0 && FIXED_CAPACITY != 0;

/**
 * ArrayBase implements basic array behavior that a subclass can inherit. To do
 * that, the subclass must pass a delegate class as the last parameter \c C and
 * implement the following three methods: <table>
 *   <tr><td><code>array_capacity()</code></td><td>Returns the current capacity
 * of the array and is ignored if #FIXED_CAPACITY is non-zero. The method must
 * be const and nothrow.</td></tr>
 *   <tr><td><code>array_data()</code></td><td>Returns the location of the data
 * with a const qualifier. The method must be declared const noexcept.</td></tr>
 *   <tr><td><code>array_data()</code></td><td>Returns the location of the data
 * without a const qualifier. The method must be declared noexcept.</td></tr>
 * </table>
 * @tparam T The element type
 * @tparam S The fixed capacity or zero if the capacity is not fixed.
 * @tparam A The alignment value or zero if no specific alignment is required.
 * coincide with the data of another array, \c false otherwise.
 * @tparam C The delegate class that needs to implement the
 */
template <typename T, size_t S, size_t A, typename C> class BaseArray {
  static_assert(A == 0 || org::simple::core::alignment_is_valid<T>(A));
  static_assert(
      S == 0 ||
      org::simple::core::SizeValue::Elements<sizeof(T)>::IsValid::value(S));

public:
  static constexpr size_t ALIGNAS =
      org::simple::core::alignment_get_correct<T>(A);
  static constexpr size_t FIXED_CAPACITY = S;

  using data_type = T;
  using delegate_type = C;
  typedef org::simple::core::SizeValue::Elements<sizeof(T)> Size;

  // raw data access

  inline const T *begin() const noexcept {
    const T *ptr = static_cast<const delegate_type *>(this)->array_data();
    if constexpr (ALIGNAS != 0) {
      return std::assume_aligned<ALIGNAS, const T>(ptr);
    } else {
      return ptr;
    }
  }

  inline T *begin() noexcept {
    T *ptr = static_cast<delegate_type *>(this)->array_data();
    if constexpr (ALIGNAS != 0) {
      return std::assume_aligned<ALIGNAS, T>(ptr);
    }
    return ptr;
  }

  inline const T *end() const noexcept {
    if constexpr (ALIGNAS != 0 && FIXED_CAPACITY != 0 &&
                  (FIXED_CAPACITY % ALIGNAS == 0)) {
      return std::assume_aligned<ALIGNAS, const T>(begin() + FIXED_CAPACITY);
    }
    return begin() + capacity();
  }

  inline T *end() noexcept {
    if constexpr (ALIGNAS != 0 && FIXED_CAPACITY != 0 &&
                  (FIXED_CAPACITY % ALIGNAS == 0)) {
      return std::assume_aligned<ALIGNAS, T>(begin() + FIXED_CAPACITY);
    }
    return begin() + capacity();
  }

  // capacity

  size_t capacity() const noexcept {
    if constexpr (FIXED_CAPACITY != 0) {
      return FIXED_CAPACITY;
    } else {
      return static_cast<const delegate_type *>(this)->array_capacity();
    }
  }

  bool is_valid_calacity(size_t capacity) const {
    return Size::IsValid::value(capacity);
  }

  // Raw element access

  const T &data(size_t offset) const noexcept { return begin()[offset]; }
  const T &operator[](size_t offset) const noexcept { return data(offset); }

  T &data(size_t offset) noexcept { return begin()[offset]; }
  T &operator[](size_t offset) noexcept { return data(offset); }

  // Offset-checked element access

  const T &at(size_t offset) const {
    if constexpr (FIXED_CAPACITY != 0) {
      return begin()[org::simple::core::Index::safe(offset, FIXED_CAPACITY)];
    } else {
      return begin()[org::simple::core::Index::safe(offset, capacity())];
    }
  }
  T &at(size_t offset) {
    if constexpr (FIXED_CAPACITY != 0) {
      return begin()[org::simple::core::Index::safe(offset, FIXED_CAPACITY)];
    } else {
      return begin()[org::simple::core::Index::safe(offset, capacity())];
    }
  }

  /**
   * Copies all the elements from \c source to this array if they both have the
   * same capacity and returns \c true if that is the case. If a difference in
   * capacity can be established compile-time, compilation will fail.
   * @tparam Array The source array type
   * @param source The source array
   * @return \c true if copy was successful, \c false otherwise.
   */
  template <class Array>
  requires ArrayCompatible<Array, T> bool assign(const Array &source) noexcept {
    if constexpr (FIXED_CAPACITY != 0) {
      if constexpr (Array::FIXED_CAPACITY != 0) {
        static_assert(FIXED_CAPACITY == Array::FIXED_CAPACITY);
      } else if (FIXED_CAPACITY != source.capacity()) {
        return false;
      }
    } else if constexpr (Array::FIXED_CAPACITY != 0) {
      if (capacity() != Array::FIXED_CAPACITY) {
        return false;
      }
    } else {
      if (capacity() != source.capacity()) {
        return false;
      }
      T *__restrict destination = begin();
      const T *__restrict src = source.begin();
      for (size_t i = 0; i < capacity(); i++) {
        destination[i] = src[i];
      }
      return true;
    }
    const size_t MOVES =
        FIXED_CAPACITY != 0 ? FIXED_CAPACITY : Array::FIXED_CAPACITY;
    T *__restrict to = begin();
    const T *__restrict from = source.begin();
    for (size_t i = 0; i < MOVES; i++) {
      to[i] = from[i];
    }
    return true;
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
  requires ArrayCompatible<Array, T> bool copy(size_t dest,
                                               const Array &source) {
    if (dest >= capacity() || this->capacity() - dest < source.capacity()) {
      return false;
    }
    T *__restrict to = begin();
    const size_t end = dest + source.capacity();
    const T *__restrict wrapper = source.begin();
    for (size_t src = 0, dst = dest; dst < end; src++, dst++) {
      to[dst] = wrapper[src];
    }
    return true;
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
  requires ArrayFixedCompatible<Array, T, FIXED_CAPACITY> void
  copy(const Array &source) {
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
  requires ArrayCompatible<Array, T> bool
  copy_range(size_t dest, const Array &source, size_t start, size_t end) {
    if (end >= source.capacity() || end < start || dest >= capacity()) {
      return false;
    }
    const size_t last = dest + (end - start);
    if (last >= capacity()) {
      return false;
    }
    T *__restrict to = begin();
    const T *__restrict from = source.begin();

    for (size_t src = start, dst = dest; dst <= last; src++, dst++) {
      to[dst] = from[src];
    }
    return true;
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
  requires ArrayFixedCompatible<Array, T, FIXED_CAPACITY> void
  copy_range(const Array &source) {
    static_assert(!(END >= Array::FIXED_CAPACITY || END < START ||
                    DEST >= FIXED_CAPACITY));
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
  ArraySlice<T> range_ref(size_t start, size_t end) {
    if (end < capacity() && start <= end) {
      return ArraySlice<T>(begin() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }

  const ArraySlice<T> range_ref(size_t start, size_t end) const {
    if (end < capacity() && start <= end) {
      return ArraySlice<T>(begin() + start, end + 1 - start);
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
  requires(FIXED_CAPACITY != 0) ArraySlice<T> range_ref() {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return ArraySlice<T>(begin() + START, END + 1 - START);
  }

  template <size_t START, size_t END>
  requires(FIXED_CAPACITY != 0) const ArraySlice<T> range_ref() const {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return ArraySlice<T>(begin() + START, END + 1 - START);
  }

  /**
   * Returns an array whose elements are a copy of the elements in the range
   * that starts at element \c start and ends at element \c end. Throws
   * std::out_of_range if the range exceeds the array boundaries.
   * @param start The first element to copy.
   * @param end The last element to copy.
   * @return an Array representing the range.
   */
  const ArrayHeap<T> range_copy(size_t start, size_t end) const {
    if (end < capacity() && start <= end) {
      return ArrayHeap<T>(begin() + start, end + 1 - start);
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
  requires(FIXED_CAPACITY != 0) const ArrayHeap<T> range_copy() const {
    static_assert(END < FIXED_CAPACITY && START <= END);
    return ArrayHeap<T>(begin() + START, END + 1 - START);
  }
};

namespace helper {

template <typename T> static constexpr size_t eff_align(size_t A) {
  return org::simple::core::alignment_get_correct<T>(A);
}

template <typename T, size_t S> static constexpr size_t eff_capacity() {
  static_assert(
      org::simple::core::SizeValue::Elements<sizeof(T)>::IsValid::value(S));
  return S;
}

} // namespace helper

using namespace helper;

template <typename T, size_t S, size_t A = 0>
class ArrayInline : public BaseArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                                     ArrayInline<T, S, A>> {
  static_assert(std::is_trivially_constructible_v<T>);
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_move_assignable_v<T>);
  alignas(eff_align<T>(A)) T data_[S];

  T *array_data() noexcept { return &data_[0]; }
  const T *array_data() const noexcept { return &data_[0]; }

public:
  typedef BaseArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                    ArrayInline<T, S, A>>
      Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;
  friend Super;

  ArrayInline() = default;
  ArrayInline(const ArrayInline &) = default;
  ArrayInline(ArrayInline &&) = default;
};

template <typename T, size_t S, size_t A = 0>
class ArrayConstAlloc
    : public BaseArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                       ArrayConstAlloc<T, S, A>> {
public:
  static_assert(std::is_trivially_constructible_v<T>);
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_move_assignable_v<T>);

  struct DataStruct {
    alignas(eff_align<T>(A)) T data_[S];
  };

  T *array_data() noexcept { return data_->data_; }
  const T *array_data() const noexcept { return data_->data_; }

public:
  typedef BaseArray<T, eff_capacity<T, S>(), eff_align<T>(A),
                    ArrayConstAlloc<T, S, A>>
      Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;
  friend Super;

  ArrayConstAlloc() : data_(new T[Super::SizeMetric::Valid::value(S)]) {}

  ArrayConstAlloc(const ArrayConstAlloc &source) : data_(new DataStruct) {
    for (size_t i = 0; i < Super::constSize(); i++) {
      data_[i] = source.data_[i];
    }
  }

  ArrayConstAlloc(ArrayConstAlloc &&source) : data_(source.data_) {
    source.data_ = nullptr;
  }
  ~ArrayConstAlloc() {
    delete data_;
    data_ = nullptr;
  }

private:
  DataStruct *data_;
};

template <typename T, size_t S>
class ArrayConstRef
    : public BaseArray<T, eff_capacity<T, S>(), 0, ArrayConstRef<T, S>> {

  T *data_;

  T *array_data() noexcept { return data_; }
  const T *array_data() const noexcept { return data_; }

public:
  typedef BaseArray<T, eff_capacity<T, S>(), 0, ArrayConstRef<T, S>> Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;
  friend Super;

  ArrayConstRef() = delete;
  ArrayConstRef(T *data) : data_(core::Dereference::checked(data)) {}
  ArrayConstRef(const ArrayConstRef &source) : data_(source.data_) {}
};

template <typename T>
class ArraySlice : public BaseArray<T, 0, 0, ArraySlice<T>> {

  T *data_;
  size_t capacity_;

  size_t array_capacity() const noexcept { return capacity_; }
  T *array_data() noexcept { return data_; }
  const T *array_data() const noexcept { return data_; }

public:
  typedef BaseArray<T, 0, 0, ArraySlice<T>> Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  friend Super;

  ArraySlice() = delete;
  ArraySlice(T *data, size_t size)
      : data_(core::Dereference::checked(data)),
        capacity_(Super::Size::Valid::value(size)) {}
  ArraySlice(const ArraySlice &source)
      : data_(source.data_), capacity_(source.capacity_) {}
};

template <typename T, size_t A>
class ArrayHeap : public BaseArray<T, 0, eff_align<T>(A), ArrayHeap<T, A>> {
  static_assert(std::is_trivially_constructible_v<T>);
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_move_assignable_v<T>);

  size_t array_capacity() const noexcept { return data_.capacity(); }
  T *array_data() noexcept { return data_.data(); }
  const T *array_data() const noexcept { return data_.data(); }

  static constexpr T *allocate(size_t size) {
    const size_t sz = Size::Valid::value(size);
    if constexpr (Super::ALIGNAS > 0) {
      return new T[sz, Super::ALIGNAS];
    }
    return new T[sz];
  }

public:
  typedef BaseArray<T, 0, eff_align<T>(A), ArrayHeap<T, A>> Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  friend Super;

  ArrayHeap(size_t size) : data_(size) {}
  ArrayHeap(const T *source, size_t size) : data_(size) {
    T *to = this->begin();
    for (size_t i = 0; i < data_.capacity(); i++) {
      to[i] = source[i];
    }
  }
  template <class Array>
  requires ArrayCompatible<Array, T> ArrayHeap(const Array &source)
      : data_(source.capacity()) {
    T *to = data_.data();
    const T *from = source.begin();
    for (size_t i = 0; i < data_.capacity(); i++) {
      to[i] = from[i];
    }
  }
  ArrayHeap(ArrayHeap &&source) : data_(std::move(source.data_)) {}

private:
  core::AlignedAlloc<T, eff_align<T>(A)> data_;
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_BASEARRAY_H
