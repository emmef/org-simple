#ifndef ORG_SIMPLE_NUMARRAY_H
#define ORG_SIMPLE_NUMARRAY_H
/*
 * org-simple/util/NumArray.h
 *
 * Added by michel on 2020-10-11
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

#include <complex>
#include <org-simple/core/Index.h>
#include <org-simple/util/BaseArray.h>
#include <type_traits>

namespace org::simple::util {

template <typename T, size_t ELEMENTS, class S> struct BaseNumArray;


namespace concepts {

template <typename T> struct BaseIsComplexType : public std::false_type {};

template <typename T>
struct BaseIsComplexType<std::complex<T>> : public std::true_type {};

template <typename T> concept ComplexNumber = BaseIsComplexType<T>::value;

template <typename T> concept RealNumber = std::is_arithmetic_v<T>;

template <typename T> concept Number = (RealNumber<T> || ComplexNumber<T>);

template <typename L, typename R>
concept NumberIsR2LAssignable = (ComplexNumber<L> && Number<R>) ||
                                (RealNumber<L> && RealNumber<R>);

} // namespace concepts

template <typename T, size_t ELEMENTS, class S> struct BaseNumArray : public S {
  static_assert(concepts::Number<T>);
  static_assert(IsBaseArrayConstSize<S>);

  template <typename X>
  static constexpr bool SameSizeArray =
      IsBaseArrayConstSize<T> && X::constSize() == ELEMENTS;

  template <typename X>
  static constexpr bool NotSmallerArray =
      IsBaseArrayConstSize<T> && X::constSize() >= ELEMENTS;

  template <typename X>
  static constexpr bool BiggerArray = IsBaseArrayConstSize<T> &&
                                      X::constSize() > ELEMENTS;

  template <typename X>
  static constexpr bool NotBiggerArray =
      IsBaseArrayConstSize<T> && X::constSize() <= ELEMENTS;

  template <typename X>
  static constexpr bool SmallerArray = IsBaseArrayConstSize<T> &&
                                       X::constSize() < ELEMENTS;

  template <size_t START, size_t SRC_ELEM>
  static constexpr bool
      ValidForGraftArray = IsBaseArrayConstSize<T> &&
                           (START + SRC_ELEM <= ELEMENTS);

  template <typename X>
  static constexpr bool ValidForCrossProductArray =
      IsBaseArrayConstSize<T> &&ELEMENTS == X::constSize() == 3;

  BaseNumArray() = default;
  BaseNumArray(const BaseNumArray &) = default;
  BaseNumArray(BaseNumArray &&) = default;

  BaseNumArray(const std::initializer_list<T> &values) {
    size_t i = 0;
    for (auto v : values) {
      if (i == ELEMENTS) {
        return;
      }
      this->data(i++) = v;
    }
    while (i < ELEMENTS) {
      this->data(i++) = 0;
    }
  }

  template <typename R>
  BaseNumArray(const std::initializer_list<T>
                   &values) requires concepts::NumberIsR2LAssignable<T, R> {
    size_t i = 0;
    for (auto v : values) {
      this->data(i++) = v;
    }
    while (i < ELEMENTS) {
      this->data(i++) = 0;
    }
  }

  typedef T value_type;
  static constexpr size_t elements = ELEMENTS;

  void zero() noexcept {
    for (size_t i = 0; i < this->size(); i++) {
      this->data(i) = 0;
    }
  }

  void fill(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) = v;
    }
  }

  BaseNumArray &plus(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) += v;
    }
    return *this;
  }

  BaseNumArray &operator<<(const T *source) {
    const T *src = core::Dereference::safe(source);
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) += src[i];
    }
    return *this;
  }

  void operator>>(const T *destination) {
    const T *dst = core::Dereference::safe(destination);
    for (size_t i = 0; i < ELEMENTS; i++) {
      dst[i] += this->data(i);
    }
  }

  template <class Array>
  requires NotSmallerArray<Array> BaseNumArray &
  operator<<(const Array &source) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) += source[i];
    }
    return *this;
  }

  template <typename X>
  requires SmallerArray<X> BaseNumArray &operator<<(const X &source) noexcept {
    size_t i;
    for (i = 0; i < X::constSize(); i++) {
      this->data(i) += source[i];
    }
    for (; i < ELEMENTS; i++) {
      this->data(i) = 0;
    }
    return *this;
  }

  template <typename Array, size_t START, size_t SRC_ELEM>
  requires ValidForGraftArray<Array, START, SRC_ELEM> BaseNumArray &
  graft(const Array &source) noexcept {
    for (size_t src = 0, dst = START; src <= SRC_ELEM; src++, dst++) {
      this->data(dst) = source[src];
    }
    return *this;
  }

  // Negate

  BaseNumArray operator-() const noexcept {
    BaseNumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = -this->data(i);
    }
    return r;
  }

  // Add another array

  BaseNumArray &operator+=(const BaseNumArray &o) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) += o[i];
    }
    return *this;
  }

  template <class Array>
  BaseNumArray &operator+=(const Array &o) noexcept
      requires SameSizeArray<Array> {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) += o[i];
    }
    return *this;
  }

  BaseNumArray operator+(const BaseNumArray &o) const noexcept {
    BaseNumArray r = *this;
    r += o;
    return r;
  }

  template <class Array>
  requires IsBaseArray<Array>
  BaseNumArray operator+(const Array &o) const noexcept {
    BaseNumArray r = *this;
    r += o;
    return r;
  }

  friend BaseNumArray &operator+(const BaseNumArray &o,
                                 BaseNumArray &&a) noexcept {
    a += o;
    return a;
  }

  template <class Array>
  friend BaseNumArray &operator+(const Array &o, BaseNumArray &&a) noexcept {
    a += o;
    return a;
  }

  // Subtract an array

  BaseNumArray &operator-=(const BaseNumArray &o) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) -= o[i];
    }
    return *this;
  }

  BaseNumArray operator-(const BaseNumArray &o) const noexcept {
    BaseNumArray r = *this;
    r -= o;
    return r;
  }

  // Multiply by scalar

  BaseNumArray &operator*=(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) *= v;
    }
    return *this;
  }

  BaseNumArray operator*(T v) const noexcept {
    BaseNumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = this->data(i) * v;
    }
    return r;
  }

  friend BaseNumArray operator*(T v, const BaseNumArray &a) noexcept {
    return a * v;
  }

  friend BaseNumArray &operator*(T v, BaseNumArray &&a) noexcept {
    a *= v;
    return a;
  }

  // Divide by scalar

  BaseNumArray &operator/=(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      this->data(i) /= v;
    }
    return *this;
  }

  BaseNumArray operator/(T v) const noexcept {
    BaseNumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = this->data(i) / v;
    }
    return r;
  }

  // Multiply with array

  T in_product(const BaseNumArray &o) const noexcept {
    T sum = this->data(0) * o[0];
    for (size_t i = 0; i < ELEMENTS; i++) {
      sum += this->data(i) * o[i];
    }
    return sum;
  }

  T self() const noexcept { return in(*this); }

  template <typename Array>
  requires ValidForCrossProductArray<Array>
      BaseNumArray cross_product(const Array &o) const noexcept {
    BaseNumArray r;
    r[0] = this->data(1) * o[2] - this->data(2) * o[1];
    r[1] = this->data(2) * o[0] - this->data(0) * o[2];
    r[2] = this->data(0) * o[1] - this->data(1) * o[0];
    return r;
  }
};

template <typename T, size_t N>
using NumArray = BaseNumArray<T, N, ArrayInline<T, N>>;

} // namespace org::simple::util

#endif // ORG_SIMPLE_NUMARRAY_H
