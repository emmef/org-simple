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
#include <org-simple/core/number_traits.h>
#include <org-simple/util/Array.h>
#include <type_traits>

namespace org::simple::util {

template <typename T, class S> struct BaseNumArray;

namespace concepts {

using namespace org::simple::core;

template <typename L, typename R>
concept NumberIsR2LAssignable = (is_complex_v<L> && is_number<R>) ||
                                (std::is_arithmetic_v<L> &&
                                 std::is_arithmetic_v<R>);

} // namespace concepts

using namespace concepts;

template <typename T, class S> struct BaseNumArray : public S {
  static_assert(is_base_array<S>);
  static_assert(concept_base_array<S>::FIXED_CAPACITY != 0);
  static_assert(org::simple::traits::is_number<T>);

  typedef S Super;
  typedef typename Super::data_type data_type;
  typedef typename Super::Size Size;
  using Super::FIXED_CAPACITY;

  template <typename X>
  static constexpr bool SameSizeArray =
      is_type_compat_same_size_arrays<X, T, FIXED_CAPACITY>;

  template <typename X>
  static constexpr bool NotSmallerArray =
      is_type_compat_ge_size_arrays<X, T, FIXED_CAPACITY>;

  template <typename X>
  static constexpr bool BiggerArray =
      is_type_compat_gt_size_arrays<X, T, FIXED_CAPACITY>;

  template <typename X>
  static constexpr bool NotBiggerArray =
      is_type_compat_le_size_arrays<X, T, FIXED_CAPACITY>;

  template <typename X>
  static constexpr bool SmallerArray =
      is_type_compat_lt_size_arrays<X, T, FIXED_CAPACITY>;

  template <size_t START, size_t SRC_ELEM>
  static constexpr bool ValidForGraftArray = (START + SRC_ELEM <=
                                              FIXED_CAPACITY);

  template <typename X>
  static constexpr bool ValidForCrossProductArray =
      FIXED_CAPACITY == concept_base_array<X>::FIXED_CAPACITY &&FIXED_CAPACITY == 3;

  BaseNumArray() = default;
  BaseNumArray(const BaseNumArray &) = default;
  BaseNumArray(BaseNumArray &&) = default;

  BaseNumArray(const std::initializer_list<T> &values) {
    size_t i = 0;
    auto data = this->begin();
    for (auto v : values) {
      if (i == FIXED_CAPACITY) {
        return;
      }
      data[i++] = v;
    }
    while (i < FIXED_CAPACITY) {
      data[i++] = 0;
    }
  }

  template <typename R>
  BaseNumArray(const std::initializer_list<T>
                   &values) requires concepts::NumberIsR2LAssignable<T, R> {
    size_t i = 0;
    auto data = this->begin();
    for (auto v : values) {
      data[i++] = v;
    }
    while (i < FIXED_CAPACITY) {
      data[i++] = 0;
    }
  }

  typedef T value_type;
  static constexpr size_t elements = FIXED_CAPACITY;

  void zero() {
    auto data = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      data[i] = 0;
    }
  }

  void fill(T v) {
    auto data = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      data[i] = v;
    }
  }

  BaseNumArray &plus(T v) {
    auto data = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      data[i] += v;
    }
    return *this;
  }

  BaseNumArray &operator<<(const T *source) {
    const T *src = core::Dereference::safe(source);
    auto data = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      data[i] += src[i];
    }
    return *this;
  }

  void operator>>(T *destination) {
    const T *dst = core::Dereference::safe(destination);
    for (size_t i = 0; i < this->capacity(); i++) {
      dst[i] += this->data_(i);
    }
  }

  // Negate

  BaseNumArray operator-() const {
    BaseNumArray r;
    auto data = this->begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      r[i] = -data[i];
    }
    return r;
  }

  BaseNumArray &negate() {
    auto data = this->begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      data[i] = -data[i];
    }
    return *this;
  }

  // Add another array

  template <class Array>
  requires SameSizeArray<Array> BaseNumArray &
  operator+=(const Array &source) {
    T * __restrict data = this->begin();
    const T * __restrict o = source.begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      data[i] += o[i];
    }
    return *this;
  }

  template <class Array>
  requires SameSizeArray<Array> BaseNumArray
  operator+(const Array &o) const {
    BaseNumArray r = *this;
    r += o;
    return r;
  }

  template <class Array>
  requires SameSizeArray<Array> friend BaseNumArray &
  operator+(const Array &o, BaseNumArray &&a) {
    a += o;
    return a;
  }

  // Subtract an array

  template <class Array>
  requires SameSizeArray<Array> BaseNumArray &
  operator-=(const Array &source) {
    T * __restrict data = this->begin();
    const T * __restrict o = source.begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      data[i] -= o[i];
    }
    return *this;
  }

  template <class Array>
  requires SameSizeArray<Array> BaseNumArray
  operator-(const Array &o) const {
    BaseNumArray r = *this;
    r -= o;
    return r;
  }

  template <class Array>
  friend BaseNumArray &operator-(const Array &o, BaseNumArray &&a) {
    a += o;
    return a;
  }

  // Multiply by scalar

  BaseNumArray &operator*=(T v) {
    auto data = this->begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      data[i] *= v;
    }
    return *this;
  }

  BaseNumArray operator*(T v) const {
    BaseNumArray r;
    auto data = this->begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      r[i] = data[i] * v;
    }
    return r;
  }

  friend BaseNumArray operator*(T v, const BaseNumArray &a) { return a * v; }

  friend BaseNumArray &operator*(T v, BaseNumArray &&a) {
    a *= v;
    return a;
  }

  // Divide by scalar

  BaseNumArray &operator/=(T v) {
    auto data = this->begin();
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      data[i] /= v;
    }
    return *this;
  }

  BaseNumArray operator/(T v) const {
    auto data = this->begin();
    BaseNumArray r;
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      r[i] = data[i] / v;
    }
    return r;
  }

  friend BaseNumArray operator/(T v, const BaseNumArray &a) { return a / v; }

  friend BaseNumArray &operator/(T v, BaseNumArray &&a) {
    a /= v;
    return a;
  }

  // Multiply with array

  // Dot product

  template <class Array>
  requires SameSizeArray<Array> T dot(const Array &other) const {
    auto v1 = this->begin();
    auto v2 = other.begin();

    static constexpr bool v1Complex = org::simple::core::is_complex<T>::value;
    static constexpr bool v2Complex = org::simple::core::is_complex<
        typename concept_base_array<Array>::data_type>::value;

    if constexpr (v1Complex) {
      T sum = 0;
      for (size_t i = 0; i < FIXED_CAPACITY; i++) {
        sum += std::conj(v1[i]) * v2[i];
      }
      return sum;
    }
    if constexpr (v2Complex) {
      T sum = 0;
      for (size_t i = 0; i < FIXED_CAPACITY; i++) {
        sum += v1[i] * v2[i].real();
      }
      return sum.real();
    }
    T sum = 0;
    for (size_t i = 0; i < FIXED_CAPACITY; i++) {
      sum += v1[i] * v2[i];
    }
    return sum;
  }

  // Squared absolute value (norm)

  typename org::simple::core::is_complex<T>::real_type
  squared_absolute() const {
    if constexpr (org::simple::core::is_complex<T>::value) {
      typename org::simple::core::is_complex<T>::value_type sum = 0;
      const T *p = this->begin();
      for (size_t i = 0; i < FIXED_CAPACITY; i++) {
        sum += std::norm(p[i]);
      }
      return sum;
    } else {
      T sum = 0;
      const T *p = this->begin();
      for (size_t i = 0; i < FIXED_CAPACITY; i++) {
        sum += p[i] * p[i];
      }
      return sum;
    }
  }

  template <typename Array>
  requires ValidForCrossProductArray<Array>
      BaseNumArray cross_product(const Array &source) const {
    BaseNumArray r;
    auto data = this->begin();
    auto o = source.begin();
    r[0] = data[1] * o[2] - data[2] * o[1];
    r[1] = data[2] * o[0] - data[0] * o[2];
    r[2] = data[0] * o[1] - data[1] * o[0];
    return r;
  }
};

template <typename T, size_t N> using NumArray = BaseNumArray<T, Array<T, N>>;

} // namespace org::simple::util

#endif // ORG_SIMPLE_NUMARRAY_H
