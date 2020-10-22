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
#include <org-simple/core/Index.h>
#include <org-simple/core/Size.h>
#include <stdexcept>

namespace org::simple::util {

template <typename T, class S> class BaseArray;
template <typename T, class S> class BaseArrayConstSize;
template <typename T, size_t S> class ArrayInline;
template <typename T> class ArraySlice;

namespace concepts {

template <typename T, class Sub>
concept BaseArrayVarSizeImpl = std::is_base_of_v<BaseArray<T, Sub>, Sub>;

template <typename T, class Sub>
concept BaseArrayConstSizeImpl =
    std::is_base_of_v<BaseArrayConstSize<T, Sub>, Sub>;

template <typename T, class Sub>
concept BaseArrayImpl =
    BaseArrayConstSizeImpl<T, Sub> || BaseArrayVarSizeImpl<T, Sub>;

} // namespace concepts

template <typename T, class S> class BaseArray {
protected:
  T *data() noexcept { return static_cast<S *>(this)->virtualData(); }

  const T *data() const noexcept {
    return static_cast<const S *>(this)->virtualConstData();
  }

  size_t checked_index(size_t index) const {
    if (index < size()) {
      return index;
    }
    throw std::out_of_range("BaseArray.checked_index(index)");
  }

public:
  typedef ::org::simple::core::SizeValue::Elements<sizeof(T)> SizeMetric;

  size_t size() const noexcept {
    return static_cast<const S *>(this)->virtualSize();
  }
  // Checked non-const access to element
  T &operator()(size_t index) noexcept { return data()[checked_index(index)]; }
  // Checked const access to element
  const T &operator()(size_t index) const noexcept {
    return data()[checked_index(index)];
  }
  // Unchecked non-const access to element
  T &operator[](size_t index) noexcept { return data()[index]; }
  // Unchecked const access to element
  const T &operator[](size_t index) const noexcept { return data()[index]; }

  template <typename Array>
  requires concepts::BaseArrayImpl<T, Array> bool copy(size_t dest,
                                                       const Array &source) {
    if (dest >= this->size() || this->size() - dest < source.size()) {
      return false;
    }
    const size_t end = dest + source.size();
    for (size_t src = 0, dst = dest; dst < end; src++, dst++) {
      this->operator[](dst) = source[src];
    }
    return true;
  }

  template <typename Array>
  requires concepts::BaseArrayImpl<T, Array> bool
  copy_range(size_t dest, const Array &source, size_t start, size_t end) {
    if (end >= source.size() || end < start || dest >= this->size()) {
      return false;
    }
    const size_t last = dest + (end - start);
    if (last >= this->size()) {
      return false;
    }
    for (size_t src = start, dst = dest; dst <= last; src++, dst++) {
      this->operator[](dst) = source[src];
    }
    return true;
  }

  ArraySlice<T> splice(size_t start, size_t end) {
    if (end < size() && start <= end) {
      return ArraySlice<T>(data() + start, end + 1 - start);
    }
    throw std::out_of_range("org::simple::util::BaseArray(start,end)");
  }

  const ArraySlice<T> splice(size_t start, size_t end) const {
    if (end < size() && start <= end) {
      return ArraySlice<T>(data(), end + 1 - start);
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
    if (index < constSize()) {
      return index;
    }
    throw std::out_of_range("BaseArrayCOnstSize::checked_index(index)");
  }

  size_t virtualSize() const noexcept { return S::virtualConstSize(); }

public:
  typedef BaseArray<T, BaseArrayConstSize<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;

  static constexpr size_t constSize() noexcept { return S::virtualConstSize(); }

  template <size_t START, size_t END>
  static constexpr bool ValidSlice = (END < constSize()) && (START <= END);

  template <size_t START, size_t END>
  requires(ValidSlice<START, END>)
      ArrayInline<T, END + 1 - START> splice() const noexcept {
    ArrayInline<T, END + 1 - START> r;
    for (size_t src = START, dst = 0; src <= END; src++, dst++) {
      r[dst] = this->operator[](src);
    }
    return r;
  }

  ArraySlice<T> splice(size_t start, size_t end) {
    return Super::splice(start, end);
  }

  const ArraySlice<T> splice(size_t start, size_t end) const {
    return Super::splice(start, end);
  }

  template <typename Array, size_t DEST>
  static constexpr bool
      ValidCopy = concepts::BaseArrayConstSizeImpl<T, Array> &&
                  (DEST < constSize()) &&
                  (constSize() - DEST >= Array::constSize());

  template <size_t DEST, typename Array>
  requires ValidCopy<Array, DEST> void copy(Array &source) {
    const size_t end = DEST + Array::constSize();
    for (size_t src = 0, dst = DEST; dst < end; src++, dst++) {
      this->operator[](dst) = source[src];
    }
  }

  template <typename Array, size_t DEST, size_t START, size_t END>
  static constexpr bool
      ValidFixedRangeCopy = concepts::BaseArrayConstSizeImpl<T, Array> &&
                            (END < Array::constSize()) && (START <= END) &&
                            (DEST < constSize()) &&
                            (constSize() - DEST >= (END + 1 - START));

  template <typename Array, size_t DEST, size_t START, size_t END>
  requires ValidFixedRangeCopy<Array, DEST, START, END> void
  copy_range(Array &source) {
    const size_t last = DEST + (END - START);
    for (size_t src = START, dst = DEST; dst <= last; src++, dst++) {
      this->operator[](dst) = source[src];
    }
  }
};

template <typename T, size_t S>
class ArrayInline : public BaseArrayConstSize<T, ArrayInline<T, S>> {
  T data_[S];

  T *virtualData() noexcept { return &data_[0]; }
  const T *virtualConstData() const noexcept { return &data_[0]; }
  static constexpr size_t virtualConstSize() noexcept { return S; }

public:
  typedef BaseArrayConstSize<T, ArrayInline<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  static_assert(SizeMetric::IsValid::value(S));

  ArrayInline() = default;
  ArrayInline(const ArrayInline &) = default;
  ArrayInline(ArrayInline &&) = default;
};

template <typename T, size_t S>
class ArrayConstAlloc : public BaseArrayConstSize<T, ArrayConstAlloc<T, S>> {

  T *data_;

  static constexpr size_t virtualConstSize() noexcept { return S; }
  T *virtualData() noexcept { return data_; }
  const T *virtualConstData() const noexcept { return data_; }

protected:
  T *data() noexcept { return Super::data(); }
  const T *data() const noexcept { return Super::data(); }

public:
  typedef BaseArrayConstSize<T, ArrayConstAlloc<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  ArrayConstAlloc() : data_(new T[Super::SizeMetric::Valid::value(S)]) {}

  ArrayConstAlloc(const ArrayConstAlloc &source)
      : data_(new T[Super::SizeMetric::Valid::value(S)]) {
    for (size_t i = 0; i < Super::constSize(); i++) {
      data_[i] = source.data_[i];
    }
  }

  ArrayConstAlloc(ArrayConstAlloc &&source) : data_(source.data_) {
    source.data_ = nullptr;
  }
  ~ArrayConstAlloc() {
    delete[] data_;
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
  typedef BaseArrayConstSize<T, ArrayConstRef<T, S>> Super;
  typedef typename Super::SizeMetric SizeMetric;
  friend Super;
  ArrayConstRef() = delete;
  ArrayConstRef(T *data) : data_(core::Dereference::checked(data)) {}
  ArrayConstRef(const ArrayConstRef &source) : data_(source.data_) {}
  static_assert(SizeMetric::IsValid::value(S));
};

template <typename T>
class ArraySlice : public BaseArray<T, ArraySlice<T>> {

  T *data_;
  size_t size_;

  T *virtualData() noexcept { return data_; }
  const T *virtualConstData() const noexcept { return data_; }
  size_t virtualSize() const noexcept { return size_; }

protected:
  T *data() noexcept { return Super::data(); }
  const T *data() const noexcept { return Super::data(); }

public:
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

} // namespace org::simple::util

#endif // ORG_SIMPLE_BASEARRAY_H
