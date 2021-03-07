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
#include <org-simple/core/Numbers.h>
#include <org-simple/util/BaseArray.h>
#include <type_traits>

namespace org::simple::util {

template <typename T, size_t ELEMENTS, class S> struct BaseNumArray;


namespace concepts {

using namespace org::simple::core;

template <typename L, typename R>
concept NumberIsR2LAssignable = (is_complex_v<L> && is_number<R>) ||
                                (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>);

} // namespace concepts

template <typename T, size_t ELEMENTS, class S> struct BaseNumArray : public S {
  static_assert(org::simple::core::is_number<T>);
  static_assert(BaseArrayTest<S>::value);
  static_assert(BaseArrayTest<S>::FIXED_CAPACITY != 0);

  template <typename X>
  static constexpr bool SameSizeArray =
      BaseArrayTest<X>::FIXED_CAPACITY == ELEMENTS;

  template <typename X>
  static constexpr bool NotSmallerArray =
      BaseArrayTest<X>::FIXED_CAPACITY >= ELEMENTS;

  template <typename X>
  static constexpr bool BiggerArray = BaseArrayTest<X>::FIXED_CAPACITY > ELEMENTS;

  template <typename X>
  static constexpr bool NotBiggerArray =
      BaseArrayTest<X>::FIXED_CAPACITY <= ELEMENTS;

  template <typename X>
  static constexpr bool SmallerArray =
      BaseArrayTest<X>::FIXED_CAPACITY < ELEMENTS;

  template <size_t START, size_t SRC_ELEM>
  static constexpr bool
      ValidForGraftArray = (START + SRC_ELEM <= ELEMENTS);

  template <typename X>
  static constexpr bool ValidForCrossProductArray =
      ELEMENTS == BaseArrayTest<X>::FIXED_CAPACITY && ELEMENTS == 3;

  BaseNumArray() = default;
  BaseNumArray(const BaseNumArray &) = default;
  BaseNumArray(BaseNumArray &&) = default;

  BaseNumArray(const std::initializer_list<T> &values) {
    size_t i = 0;
    auto data = this->begin();
    for (auto v : values) {
      if (i == ELEMENTS) {
        return;
      }
      data[i++]= v;
    }
    while (i < ELEMENTS) {
      data[i++]= 0;
    }
  }

  template <typename R>
  BaseNumArray(const std::initializer_list<T>
                   &values) requires concepts::NumberIsR2LAssignable<T, R> {
    size_t i = 0;
    auto data = this->begin();
    for (auto v : values) {
      data[i++]= v;
    }
    while (i < ELEMENTS) {
      data[i++]= 0;
    }
  }

  typedef T value_type;
  static constexpr size_t elements = ELEMENTS;

  void zero() noexcept {
    auto data = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      data[i]= 0;
    }
  }

  void fill(T v) noexcept {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i] = v;
    }
  }

  BaseNumArray &plus(T v) noexcept {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]+= v;
    }
    return *this;
  }

  BaseNumArray &operator<<(const T *source) {
    const T *src = core::Dereference::safe(source);
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]+= src[i];
    }
    return *this;
  }

  void operator>>(T *destination) {
    const T *dst = core::Dereference::safe(destination);
    for (size_t i = 0; i < ELEMENTS; i++) {
      dst[i] += this->data(i);
    }
  }

  template <class Array>
  requires NotSmallerArray<Array> BaseNumArray &
  operator<<(const Array &source) noexcept {
    auto data = this->begin();
    auto o = source.begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]+= o[i];
    }
    return *this;
  }

  template <typename X>
  requires SmallerArray<X> BaseNumArray &operator<<(const X &source) noexcept {
    size_t i;
    auto data = this->begin();
    auto o = source.begin();
    for (i = 0; i < X::constSize(); i++) {
      data[i]+= o[i];
    }
    for (; i < ELEMENTS; i++) {
      data[i]= 0;
    }
    return *this;
  }

  template <typename Array, size_t START, size_t SRC_ELEM>
  requires ValidForGraftArray<START, SRC_ELEM> BaseNumArray &
  graft(const Array &source) noexcept {
    auto data = this->begin();
    auto o = source.begin();

    for (size_t src = 0, dst = START; src <= SRC_ELEM; src++, dst++) {
      data[dst]= o[src];
    }
    return *this;
  }

  // Negate

  BaseNumArray operator-() const noexcept {
    BaseNumArray r;
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = -data[i];
    }
    return r;
  }

  // Add another array

  BaseNumArray &operator+=(const BaseNumArray &o) noexcept {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]+= o[i];
    }
    return *this;
  }

  template <class Array>
  BaseNumArray &operator+=(const Array &source) noexcept
      requires SameSizeArray<Array> {
    auto data = this->begin();
    auto o = source.begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]+= o[i];
    }
    return *this;
  }

  BaseNumArray operator+(const BaseNumArray &o) const noexcept {
    BaseNumArray r = *this;
    r += o;
    return r;
  }

  template <class Array>
  requires BaseArrayTest<Array>::value
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

  BaseNumArray &operator-=(const BaseNumArray &source) noexcept {
    auto data = this->begin();
    auto o = source.begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]-= o[i];
    }
    return *this;
  }

  BaseNumArray operator-(const BaseNumArray &o) const noexcept {
    BaseNumArray r = *this;
    r -= o;
    return r;
  }

  template <class Array>
  requires BaseArrayTest<Array>::value
  BaseNumArray operator-(const Array &o) const noexcept {
    BaseNumArray r = *this;
    r -= o;
    return r;
  }

  friend BaseNumArray &operator-(const BaseNumArray &o,
                                 BaseNumArray &&a) noexcept {
    a -= o;
    return a;
  }

  template <class Array>
  friend BaseNumArray &operator-(const Array &o, BaseNumArray &&a) noexcept {
    a += o;
    return a;
  }

  // Multiply by scalar

  BaseNumArray &operator*=(T v) noexcept {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]*= v;
    }
    return *this;
  }

  BaseNumArray operator*(T v) const noexcept {
    BaseNumArray r;
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = data[i]* v;
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
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i]/= v;
    }
    return *this;
  }

  BaseNumArray operator/(T v) const noexcept {
    auto data = this->begin();
    BaseNumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = data[i]/ v;
    }
    return r;
  }

  friend BaseNumArray operator/(T v, const BaseNumArray &a) noexcept {
    return a / v;
  }

  friend BaseNumArray &operator/(T v, BaseNumArray &&a) noexcept {
    a /= v;
    return a;
  }

  // Multiply with array

  // Dot product

  T dot(const BaseNumArray &other) const noexcept {
    T sum = 0;
    auto data = this->begin();
    auto o = other.begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      sum +=
          org::simple::core::Numbers::times_conj(this->data(i), o[i]);
    }
    return sum;
  }

  // Squared absolute value (norm)

  typename org::simple::core::Complex<T>::real_type
  squared_absolute() const noexcept {
    typename org::simple::core::Complex<T>::value_type sum = 0;
    for (size_t i = 0; i < ELEMENTS; i++) {
      sum += org::simple::core::Numbers::squared_absolute(this->data(i));
    }
    return sum;
  }

  template <typename Array>
  requires ValidForCrossProductArray<Array>
      BaseNumArray cross_product(const Array &source) const noexcept {
    BaseNumArray r;
    auto data = this->begin();
    auto o = source.begin();
    r[0] = data[1]* o[2] - data[2]* o[1];
    r[1] = data[2]* o[0] - data[0]* o[2];
    r[2] = data[0]* o[1] - data[1]* o[0];
    return r;
  }
};

template <typename T, size_t N>
using NumArray = BaseNumArray<T, N, ArrayInline<T, N>>;

} // namespace org::simple::util

#endif // ORG_SIMPLE_NUMARRAY_H
