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
#include <org-simple/core/ClassTraits.h>
#include <org-simple/core/Index.h>
#include <org-simple/core/Size.h>
#include <stdexcept>

namespace org::simple::util {

template <typename Array> class AboutBaseArray;
template <typename T, class S> class BaseArray;
template <typename T, class S> class BaseArrayConstSize;
template <typename T, size_t S, size_t A = 0> class ArrayInline;
template <typename T> class ArraySlice;
template <typename T> class ArrayHeap;

namespace helper {

ORG_SIMPLE_ABOUT_CLASS_FUNCTION(array, size);


template <typename Array, typename Value> class AboutBaseArrayBase;

template <typename T> class BaseArrayHasValueType {

  template <typename A>
  static constexpr bool
  hasValueTypeSubst(std::add_pointer_t<typename A::value_type>()) {
    return true;
  }

  template <typename A> static constexpr bool hasValueTypeSubst(...) {
    return false;
  }

  static constexpr bool hasValueTypeTestWithValue(size_t v) {
    return hasValueTypeSubst<T>(v);
  }

public:
  static constexpr bool value = hasValueTypeSubst<T>(nullptr);
};

template <typename Array,
          bool hasValueType = BaseArrayHasValueType<Array>::value>
class BaseArrayDeductAboutBase {
  class Dummy {};

public:
  typedef AboutBaseArrayBase<Dummy, void> About;
};

template <typename Array> class BaseArrayDeductAboutBase<Array, true> {
public:
  typedef AboutBaseArrayBase<Array, typename Array::value_type> About;
};

template <typename Array> struct BaseArrayStaticWrapper {
  typedef AboutBaseArray<Array> About;

  static const typename About::value_type &at(const Array &array,
                                              size_t index) {
    if constexpr (About::hasUnsafeData) {
      return array.hasUnsafeData()[index];
    } else {
      return array[index];
    }
  }
};

template <typename Array, typename Value> class AboutBaseArrayBase {

  // Checking for unsafeData() member, returning T*.

  template <typename X>
  static constexpr bool
  hasUnsafeDataSubst(decltype(std::declval<X>().hasUnsafeData())) {
    return true;
  }
  template <typename X> static constexpr bool hasUnsafeDataSubst(...) {
    return false;
  }
  static constexpr bool hasUnsafeDataSubstWithValue(Value *v) {
    return hasUnsafeDataSubst<Array>(v);
  }

  // Checking for operator[](size_t) returning a Value&

  template <typename A>
  static constexpr bool hasArrayOperatorSubst(
      std::add_pointer_t<decltype(std::declval<A>().operator[](0))>) {
    return true;
  }

  template <typename A> static constexpr bool hasArrayOperatorSubst(void *) {
    return false;
  }
  static constexpr bool hasArrayOperatorTestWithValue(Value *v) {
    return hasArrayOperatorSubst<Array>(v);
  }

public:
  typedef Value value_type;
  typedef about_array_function_size<Array, size_t> AboutSize;
  static constexpr bool hasUnsafeData =
      hasUnsafeDataSubstWithValue(static_cast<Value *>(0));
  static constexpr bool hasArrayOperator =
      hasArrayOperatorTestWithValue(static_cast<Value *>(0));

  static constexpr bool size = AboutSize ::exists;
  static constexpr bool constSize = AboutSize::is_constexpr;
};

} // namespace helper

template <typename Array>
class AboutBaseArray : public helper::BaseArrayDeductAboutBase<Array> {
  using Super = typename helper::BaseArrayDeductAboutBase<Array>;

public:
  typedef typename Super::About::value_type value_type;
  static constexpr bool hasUnsafeData = Super::About::hasUnsafeData;
  static constexpr bool hasArrayOperator = Super::About::hasArrayOperator;
  static constexpr bool size = Super::About::size;
  static constexpr bool constSize = Super::About::constSize;

  static constexpr bool isBaseArray =
      (hasUnsafeData || hasArrayOperator) && size;
  static constexpr bool isBaseArrayConstSize =
      (hasUnsafeData || hasArrayOperator) && constSize;

  template <typename TargetType>
  static constexpr bool isAssignable = std::is_convertible_v<TargetType, value_type>;

  template <typename TargetType>
  static constexpr bool isSourceArray =
      isBaseArray && isAssignable<TargetType>;
  template <typename TargetType>
  static constexpr bool isSourceArrayConstSize =
      isBaseArrayConstSize &&isAssignable<TargetType>;
};

template <typename Array>
concept IsBaseArray = AboutBaseArray<Array>::isBaseArray;
template <typename Array>
concept IsBaseArrayConstSize = AboutBaseArray<Array>::isBaseArrayConstSize;

template <typename Array, typename T>
concept IsSourceArray = AboutBaseArray<Array>::template isSourceArray<T>;
template <typename Array, typename T>
concept IsSourceArrayConstSize =
    AboutBaseArray<Array>::template isBaseArrayConstSize<T>;

template <typename Array,
          typename Value = typename AboutBaseArray<Array>::value_type,
          bool hasConstSize = AboutBaseArray<Array>::isBaseArrayConstSize>
class BaseArrayWrapper;

template <typename Array, typename Value>
class BaseArrayWrapper<Array, Value, true>
    : public helper::BaseArrayStaticWrapper<Array> {
  const Array &array_;

public:
  BaseArrayWrapper(const Array &array) : array_(array){};
  typedef typename helper::BaseArrayStaticWrapper<Array>::About About;
  using helper::BaseArrayStaticWrapper<Array>::at;

  const Value &operator[](size_t index) { return at(array_, index); }
  static constexpr size_t size() { return Array::size(); }
};

template <typename Array, typename Value>
class BaseArrayWrapper<Array, Value, false>
    : public helper::BaseArrayStaticWrapper<Array> {
  const Array &array_;

public:
  BaseArrayWrapper(const Array &array) : array_(array){};
  using helper::BaseArrayStaticWrapper<Array>::at;

  const Value &operator[](size_t index) { return at(array_, index); }
  size_t size() const { return array_.size(); }
};

template <typename T, class S> class BaseArray {
protected:
  T *data() noexcept { return static_cast<S *>(this)->virtualData(); }
  T &data(size_t index) noexcept { return data()[index]; }
  const T &data(size_t index) const noexcept { return unsafeData()[index]; }
  size_t checked_index(size_t index) const {
    if (index < size()) {
      return index;
    }
    throw std::out_of_range("BaseArray.checked_index(index)");
  }

public:
  typedef T value_type;
  const T *unsafeData() const noexcept {
    return static_cast<const S *>(this)->virtualConstData();
  }
  typedef ::org::simple::core::SizeValue::Elements<sizeof(T)> SizeMetric;
  size_t size() const noexcept {
    return static_cast<const S *>(this)->virtualSize();
  }
  // Unchecked non-const access to element
  T &operator[](size_t index) noexcept { return data(index); }
  // Unchecked const access to element
  const T &operator[](size_t index) const noexcept {
    return data(index);
  }
  // Checked non-const access to element
  T &at(size_t index) noexcept { return data(checked_index(index)); }
  // Checked const access to element
  const T &at(size_t index) const noexcept {
    return data(checked_index(index));
  }

  /**
   * Copies the array \c source to position \c dest and returns \c true if that
   * was successful. Fails if \c source too large or the combination of \c dest
   * and the size of \c would exceed this array.
   * @tparam Array The array type.
   * @param dest The destination (offset) for the first element of the source.
   * @param source The source.array
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <typename Array>
  requires IsSourceArray<Array, T> bool copy(size_t dest, const Array &source) {
    BaseArrayWrapper<Array, T> wrapper(source);
    if (dest >= this->size() || this->size() - dest < wrapper.size()) {
      return false;
    }
    const size_t end = dest + source.size();
    for (size_t src = 0, dst = dest; dst < end; src++, dst++) {
      data(dst) = wrapper[src];
    }
    return true;
  }

  /**
   * Moves the array \c source to position \c dest and returns \c true if that
   * was successful. Fails if \c source too large or the combination of \c dest
   * and the size of \c would exceed this array.
   * @tparam Array The array type.
   * @param dest The destination (offset) for the first element of the source.
   * @param source The source.array
   * @return \c true if moving was successful, \c false otherwise.
   */
  template <typename Array>
  requires IsBaseArray<Array> bool move(size_t dest, Array &source) {
    BaseArrayWrapper<Array, T> wrapper(source);
    if (dest >= this->size() || this->size() - dest < wrapper.size()) {
      return false;
    }
    const size_t end = dest + source.size();
    for (size_t src = 0, dst = dest; dst < end; src++, dst++) {
      data(dst) = std::move(wrapper[src]);
    }
    return true;
  }

  /**
   * Copies the the inclusive range \c start to \end from array \c source to
   * position \c dest and returns \c true if that was successful. Fails if \c
   * end is not smaller than the size of \c source, if \c start is larger than
   * end or if the combination of \c dest, \c start and the size of the range
   * would exceed this array.
   * @tparam Array The array type.
   * @param dest The destination (offset) for the first element of the source.
   * @param source The source array.
   * @param start The starting element to copy from the source array.
   * @param end The last element to copy from the source array.
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <typename Array>
  requires IsBaseArray<Array> bool copy_range(size_t dest, const Array &source,
                                              size_t start, size_t end) {
    BaseArrayWrapper<Array, T> wrapper(source);
    if (end >= wrapper.size() || end < start || dest >= this->size()) {
      return false;
    }
    const size_t last = dest + (end - start);
    if (last >= this->size()) {
      return false;
    }
    for (size_t src = start, dst = dest; dst <= last; src++, dst++) {
      data(dst) = wrapper[src];
    }
    return true;
  }

  /**
   * Moves the the inclusive range \c start to \end from array \c source to
   * position \c dest and returns \c true if that was successful. Fails if \c
   * end is not smaller than the size of \c source, if \c start is larger than
   * end or if the combination of \c dest, \c start and the size of the range
   * would exceed this array.
   * @tparam Array The array type.
   * @param dest The destination (offset) for the first element of the source.
   * @param source The source array.
   * @param start The starting element to copy from the source array.
   * @param end The last element to copy from the source array.
   * @return \c true if moving was successful, \c false otherwise.
   */
  template <typename Array>
  requires IsBaseArray<Array> bool move_range(size_t dest, Array &source,
                                              size_t start, size_t end) {
    BaseArrayWrapper<Array, T> wrapper(source);
    if (end >= wrapper.size() || end < start || dest >= this->size()) {
      return false;
    }
    const size_t last = dest + (end - start);
    if (last >= this->size()) {
      return false;
    }
    for (size_t src = start, dst = dest; dst <= last; src++, dst++) {
      data(dst) = std::move(wrapper[src]);
    }
    return true;
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
    if (end < size() && start <= end) {
      return ArraySlice<T>(unsafeData() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }

  const ArraySlice<T> range_ref(size_t start, size_t end) const {
    if (end < size() && start <= end) {
      return ArraySlice<T>(unsafeData() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
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
    if (end < size() && start <= end) {
      return ArrayHeap<T>(unsafeData() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }
};

template <typename T, class S>
class BaseArrayConstSize : public BaseArray<T, BaseArrayConstSize<T, S>> {

protected:
  T *virtualData() noexcept { return static_cast<S *>(this)->virtualData(); }
  const T *virtualConstData() const noexcept {
    return static_cast<const S *>(this)->virtualConstData();
  }
  static constexpr size_t checked_index(size_t index) {
    if (index < size()) {
      return index;
    }
    throw std::out_of_range("BaseArrayCOnstSize::checked_index(index)");
  }

  size_t virtualSize() const noexcept { return S::virtualConstSize(); }

public:
  typedef T value_type;
  typedef BaseArray<T, BaseArrayConstSize<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;

  static constexpr size_t size() noexcept { return S::virtualConstSize(); }

  template <size_t START, size_t END>
  static constexpr bool ValidRange = (END < size()) && (START <= END);

  /**
   * Copies the array \c source to position \c DEST and returns \c true if that
   * was successful. Fails if \c source too large or the combination of \c DEST
   * and the size of \c would exceed this array.
   * @tparam DEST The destination (offset) for the first element of the source.
   * @tparam Array The array type.
   * @param source The source.array
   * @return \c true if copying was successful, \c false otherwise.
   */
  template <size_t DEST, typename Array>
  requires IsBaseArrayConstSize<Array> void copy(const Array &source) {
    static_assert((DEST < size()) &&
                  (size() - DEST >= Array::size()));
    const size_t end = DEST + Array::size();
    for (size_t src = 0, dst = DEST; dst < end; src++, dst++) {
      this->data(dst) = BaseArrayWrapper<Array, T>::at(source, src);
    }
  }
  /**
   * @see BaseArray.copy(dest, source)
   */
  template <typename Array>
  requires IsBaseArray<Array> bool copy(size_t dest, const Array &source) {
    return Super::copy(dest, source);
  }

  /**
   * Moves the array \c source to position \c DEST and returns \c true if that
   * was successful. Fails if \c source too large or the combination of \c DEST
   * and the size of \c would exceed this array.
   * @tparam DEST The destination (offset) for the first element of the source.
   * @tparam Array The array type.
   * @param source The source array.
   * @return \c true if moving was successful, \c false otherwise.
   */
  template <size_t DEST, typename Array>
  requires IsBaseArrayConstSize<Array> void move(Array &source) {
    static_assert((DEST < size()) &&
                  (size() - DEST >= Array::size()));
    const size_t end = DEST + Array::size();
    for (size_t src = 0, dst = DEST; dst < end; src++, dst++) {
      this->data(dst) = std::move(BaseArrayWrapper<Array, T>::at(source, src));
    }
  }
  /**
   * @see BaseArray.move(dest, source)
   */
  template <typename Array>
  requires IsBaseArray<Array> bool move(size_t dest, const Array &source) {
    return Super::move(dest, source);
  }
  /**
   * Copies the the inclusive range \c start to \end from array \c source to
   * position \c dest and returns \c true if that was successful. Fails if \c
   * end is not smaller than the size of \c source, if \c start is larger than
   * end or if the combination of \c dest, \c start and the size of the range
   * would exceed this array.
   * @tparam DEST The destination (offset) for the first element of the source.
   * @tparam START The starting element to copy from the source array.
   * @tparam END The last element to copy from the source array.
   * @tparam Array The array type.
   * @param source The source array.
   */
  template <size_t DEST, size_t START, size_t END, typename Array>
  requires IsBaseArrayConstSize<Array> void copy_range(Array &source) {
    static_assert((END < Array::size()) && (START <= END) &&
                  (DEST < size()) &&
                  (size() - DEST >= (END + 1 - START)));
    const size_t last = DEST + (END - START);
    for (size_t src = START, dst = DEST; dst <= last; src++, dst++) {
      this->data(dst) = BaseArrayWrapper<Array, T>::at(source, src);
    }
  }
  /**
   * @see BaseArray.copy_range(dest, source, start, end)
   */
  template <typename Array>
  requires IsBaseArray<Array> bool
      copy_range(size_t dest, const Array &source, size_t start, size_t end) {
    return Super::copy_range(dest, source, start, end);
  }

  /**
   * Moves the the inclusive range \c start to \end from array \c source to
   * position \c dest and returns \c true if that was successful. Fails if \c
   * end is not smaller than the size of \c source, if \c start is larger than
   * end or if the combination of \c dest, \c start and the size of the range
   * would exceed this array.
   * @tparam DEST The destination (offset) for the first element of the source.
   * @tparam START The starting element to copy from the source array.
   * @tparam END The last element to copy from the source array.
   * @tparam Array The array type.
   * @param source The source array.
   */
  template <size_t DEST, size_t START, size_t END, typename Array>
  requires IsBaseArrayConstSize<Array> void move_range(Array &source) {
    static_assert((END < Array::size()) && (START <= END) &&
                  (DEST < size()) &&
                  (size() - DEST >= (END + 1 - START)));
    const size_t last = DEST + (END - START);
    for (size_t src = START, dst = DEST; dst <= last; src++, dst++) {
      this->data(dst) = std::move(BaseArrayWrapper<Array, T>::at(source, src));
    }
  }
  /**
   * @see BaseArray.move_range(dest, source, start, end)
   */
  template <typename Array>
  requires IsBaseArray<Array> bool move_range(size_t dest, const Array &source,
                                              size_t start, size_t end) {
    return Super::move_range(dest, source, start, end);
  }

  /**
   * Returns an Array whose elements reference the elements in the range that
   * starts at element \c START and ends at element \c END. Throws
   * std::out_of_range when the range exceeds the array boundaries.
   * @tparam START The first element to reference.
   * @tparam END The last element to reference.
   * @return an Array slice representing the range.
   */
  template <size_t START, size_t END>
  requires(ValidRange<START, END>) ArraySlice<T> range_ref() const noexcept {
    return ArraySlice<T>(this->data() + START, END + 1 - START);
  }
  template <size_t START, size_t END>
  requires(ValidRange<START, END>) const
      ArraySlice<T> range_ref() const noexcept {
    return ArraySlice<T>(this->data() + START, END + 1 - START);
  }
  /**
   * @see BaseArray.range_ref(start, end);
   */
  ArraySlice<T> range_ref(size_t start, size_t end) {
    return Super::range_ref(start, end);
  }
  /**
   * @see BaseArray.range_ref(start, end);
   */
  const ArraySlice<T> range_ref(size_t start, size_t end) const {
    return Super::range_ref(start, end);
  }

  /**
   * Returns an array whose elements are a copy of the elements in the range
   * that starts at element \c START and ends at element \c END..
   * @tparam START The first element to copy.
   * @tparam END The last element to copy.
   * @return an Array representing the range.
   */
  template <size_t START, size_t END>
  requires(ValidRange<START, END>)
      ArrayInline<T, END + 1 - START> range_copy() const noexcept {
    ArrayInline<T, END + 1 - START> r;
    for (size_t src = START, dst = 0; src <= END; src++, dst++) {
      r[dst] = this->data(src);
    }
    return r;
  }
  /**
   * @see BaseArray.range_copy(start, end)
   */
  const ArrayHeap<T> range_copy(size_t start, size_t end) const {
    return Super::range_copy(start, end);
  }
};

template <typename T, size_t S, size_t A>
class ArrayInline : public BaseArrayConstSize<T, ArrayInline<T, S, A>> {
  alignas(org::simple::core::Alignment<T>::get_valid(A)) T data_[S];

  T *virtualData() noexcept { return &data_[0]; }
  const T *virtualConstData() const noexcept { return &data_[0]; }
  static constexpr size_t virtualConstSize() noexcept { return S; }

public:
  typedef T value_type;
  typedef BaseArrayConstSize<T, ArrayInline<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  static_assert(SizeMetric::IsValid::value(S));

  ArrayInline() = default;
  ArrayInline(const ArrayInline &) = default;
  ArrayInline(ArrayInline &&) = default;
};

template <typename T, size_t S, size_t A = 0>
class ArrayConstAlloc : public BaseArrayConstSize<T, ArrayConstAlloc<T, S, A>> {
  struct DataStruct {
    alignas(org::simple::core::Alignment<T>::get_valid(A)) T data_[S];
  };

  DataStruct *data_;

  static constexpr size_t virtualConstSize() noexcept { return S; }
  T *virtualData() noexcept { return data_->data_; }
  const T *virtualConstData() const noexcept { return data_->data_; }

public:
  typedef T value_type;
  typedef BaseArrayConstSize<T, ArrayConstAlloc<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  ArrayConstAlloc() : data_(new T[Super::SizeMetric::Valid::value(S)]) {}

  ArrayConstAlloc(const ArrayConstAlloc &source)
      : data_(new DataStruct) {
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
  static_assert(SizeMetric::IsValid::value(S));
};

template <typename T, size_t S>
class ArrayConstRef : public BaseArrayConstSize<T, ArrayConstRef<T, S>> {

  T *data_;

  static constexpr size_t virtualConstSize() noexcept { return S; }
  T *virtualData() noexcept { return data_; }
  const T *virtualConstData() const noexcept { return data_; }

protected:
  T *data() noexcept { return Super::data(); }
  const T *data() const noexcept { return Super::data(); }

public:
  typedef T value_type;
  typedef BaseArrayConstSize<T, ArrayConstRef<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  ArrayConstRef() = delete;
  ArrayConstRef(T *data) : data_(core::Dereference::checked(data)) {}
  ArrayConstRef(const ArrayConstRef &source) : data_(source.data_) {}
  static_assert(SizeMetric::IsValid::value(S));
};

template <typename T> class ArraySlice : public BaseArray<T, ArraySlice<T>> {

  T *data_;
  size_t size_;

  T *virtualData() noexcept { return data_; }
  const T *virtualConstData() const noexcept { return data_; }
  size_t virtualSize() const noexcept { return size_; }

protected:
  T *data() noexcept { return Super::data(); }
  const T *data() const noexcept { return Super::data(); }

public:
  typedef T value_type;
  typedef BaseArray<T, ArraySlice<T>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  ArraySlice() = delete;
  ArraySlice(T *data, size_t size)
      : data_(core::Dereference::checked(data)),
        size_(SizeMetric::Valid::value(size)) {}
  ArraySlice(const ArraySlice &source)
      : data_(source.data_), size_(source.size_) {}
};

template <typename T> class ArrayHeap : public BaseArray<T, ArraySlice<T>> {

  T *data_;
  size_t size_;

  T *virtualData() noexcept { return data_; }
  const T *virtualConstData() const noexcept { return data_; }
  size_t virtualSize() const noexcept { return size_; }

protected:
  T *data() noexcept { return Super::data(); }
  const T *data() const noexcept { return Super::data(); }

public:
  typedef T value_type;
  typedef BaseArray<T, ArraySlice<T>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  ArrayHeap(size_t size)
      : data_(new T[SizeMetric::Valid::value(size)]), size_(size) {}
  ArrayHeap(const T *source, size_t size)
      : data_(new T[SizeMetric::Valid::value(size)]), size_(size) {
    for (size_t i = 0; i < size_; i++) {
      data_[i] = source[i];
    }
  }
  ArrayHeap(const ArrayHeap &source)
      : data_(new T[source.size_]), size_(source.size) {
    for (size_t i = 0; i < size_; i++) {
      data_[i] = source.data_[i];
    }
  }
  ArrayHeap(ArrayHeap &&source) : data_(source.data_), size_(source.size_) {
    source.data_ = 0;
    source.size_ = 0;
  }
  ~ArrayHeap() {
    delete data_;
    data_ = 0;
    size_ = 0;
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_BASEARRAY_H
