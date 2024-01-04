#ifndef ORG_SIMPLE_M_NUM_ARRAY_H
#define ORG_SIMPLE_M_NUM_ARRAY_H
/*
 * org-simple/NumArray.h
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
#include <org-simple/AlignedData.h>
#include <org-simple/Index.h>
#include <type_traits>

namespace org::simple {

template <typename T, size_t ELEMENTS, class S> struct BaseNumArray;

template <typename T, size_t S, size_t A = alignof(T)>
using NumArray = BaseNumArray<T, S, AlignedLocalStorage<T, S, A>>;

namespace concepts {
template <class T> struct is_complex {
  static constexpr bool value = false;
  using real_type = T;
};

template <class T> struct is_complex<std::complex<T>> {
  static constexpr bool value = !is_complex<T>::value;
  using real_type = T;
};

template <typename T> static constexpr bool is_complex_v = is_complex<T>::value;

template <typename L, typename R>
concept NumberIsR2LAssignable =
    (is_complex_v<L> && (is_complex_v<R> || std::is_arithmetic_v<R>)) ||
    (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>);

} // namespace concepts

template <typename T, size_t ELEMENTS, class S> struct BaseNumArray : public S {
  static_assert(AlignedDataInfo::get<S>().isAlignedData);
  static_assert(concepts::is_complex_v<T> || std::is_arithmetic_v<T>);

  typedef S Super;
  typedef typename Super::Type data_type;

  using ResultArray = NumArray<T, ELEMENTS, Super::alignment>;

  template <typename X> static constexpr bool is_type_compat_arrays() {
    const auto info = AlignedDataInfo::get<S>();
    if constexpr (info.isAlignedData) {
      return std::is_same_v<typename X::type, T>;
    } else {
      return false;
    }
  }

  template <typename X> static constexpr bool SameSizeArray() {
    return is_type_compat_arrays<X>() &&
           AlignedDataInfo::get<S>().elements == ELEMENTS;
  }

  template <typename X>
  static constexpr bool NotSmallerArray =
      is_type_compat_arrays<X>() &&
      AlignedDataInfo::get<S>().elements >= ELEMENTS;

  template <typename X>
  static constexpr bool BiggerArray =
      is_type_compat_arrays<X>() &&
      AlignedDataInfo::get<S>().elements > ELEMENTS;

  template <typename X>
  static constexpr bool NotBiggerArray =
      is_type_compat_arrays<X>() &&
      AlignedDataInfo::get<S>().elements <= ELEMENTS;

  template <typename X>
  static constexpr bool SmallerArray =
      is_type_compat_arrays<X>() &&
      AlignedDataInfo::get<S>().elements < ELEMENTS;

  template <size_t START, size_t SRC_ELEM>
  static constexpr bool ValidForGraftArray = (START + SRC_ELEM <= ELEMENTS);

  template <typename X>
  static constexpr bool ValidForCrossProductArray =
      AlignedDataInfo::get<X>().elements == 3 && ELEMENTS == 3;

  BaseNumArray() = default;
  BaseNumArray(const BaseNumArray &source) { assign(source); }
  BaseNumArray(BaseNumArray &&) = default;
  template <class Array>
    requires(SameSizeArray<Array>())
  BaseNumArray(const Array &array) : S(array) {}
  BaseNumArray &operator=(const BaseNumArray &source) {
    assign(source);
    return *this;
  }

  BaseNumArray(const std::initializer_list<T> &values) {
    size_t i = 0;
    auto data = this->begin();
    for (auto v : values) {
      if (i == ELEMENTS) {
        return;
      }
      data[i++] = v;
    }
    while (i < ELEMENTS) {
      data[i++] = 0;
    }
  }

  template <typename R>
  BaseNumArray(const std::initializer_list<R> &values)
    requires concepts::NumberIsR2LAssignable<T, R>
  {
    size_t i = 0;
    auto data = this->begin();
    for (auto v : values) {
      data[i++] = v;
    }
    while (i < ELEMENTS) {
      data[i++] = 0;
    }
  }

  typedef T value_type;
  static constexpr size_t elements = ELEMENTS;

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
    const T *src = source;
    auto data = this->begin();
    for (size_t i = 0; i < this->capacity(); i++) {
      data[i] += src[i];
    }
    return *this;
  }

  void operator>>(T *destination) {
    const T *dst = destination;
    for (size_t i = 0; i < this->capacity(); i++) {
      dst[i] += this->data_(i);
    }
  }

  // Negate

  ResultArray operator-() const {
    ResultArray r;
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = -data[i];
    }
    return r;
  }

  BaseNumArray &negate() {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i] = -data[i];
    }
    return *this;
  }

  // Add another array

  template <class Array>
    requires (SameSizeArray<Array>())
  BaseNumArray &operator+=(const Array &source) {
    T *__restrict data = this->begin();
    const T *__restrict o = source.begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i] += o[i];
    }
    return *this;
  }

  template <class Array>
    requires SameSizeArray<Array>
  ResultArray operator+(const Array &o) const {
    ResultArray r = *this;
    r += o;
    return r;
  }

  template <class Array>
    requires SameSizeArray<Array>
  friend BaseNumArray &operator+(const Array &o, BaseNumArray &&a) {
    a += o;
    return a;
  }

  // Subtract an array

  template <class Array>
    requires SameSizeArray<Array>
  BaseNumArray &operator-=(const Array &source) {
    T *__restrict data = this->begin();
    const T *__restrict o = source.begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i] -= o[i];
    }
    return *this;
  }

  template <class Array>
    requires SameSizeArray<Array>
  ResultArray operator-(const Array &o) const {
    ResultArray r = *this;
    r -= o;
    return r;
  }

  template <class Array>
  friend BaseNumArray &operator-(const Array &o, BaseNumArray &&a) {
    T *__restrict dst = a.begin();
    const T *const src = o.begin();
    for (size_t i = -0; i < ELEMENTS; i++) {
      dst[i] = src[i] - dst[i];
    }
    return a;
  }

  // Multiply by scalar

  BaseNumArray &operator*=(T v) {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i] *= v;
    }
    return *this;
  }

  ResultArray operator*(T v) const {
    ResultArray r;
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = data[i] * v;
    }
    return r;
  }

  friend ResultArray operator*(T v, const BaseNumArray &a) { return a * v; }

  friend BaseNumArray &operator*(T v, BaseNumArray &&a) {
    a *= v;
    return a;
  }

  // Divide by scalar

  BaseNumArray &operator/=(T v) {
    auto data = this->begin();
    for (size_t i = 0; i < ELEMENTS; i++) {
      data[i] /= v;
    }
    return *this;
  }

  ResultArray operator/(T v) const {
    auto data = this->begin();
    ResultArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r[i] = data[i] / v;
    }
    return r;
  }

  friend ResultArray operator/(T v, const BaseNumArray &a) { return a / v; }

  friend BaseNumArray &operator/(T v, BaseNumArray &&a) {
    a /= v;
    return a;
  }

  // Multiply with array

  // Dot product

  template <class Array>
    requires SameSizeArray<Array>
  T dot(const Array &other) const {
    auto v1 = this->begin();
    auto v2 = other.begin();

    static constexpr bool v1Complex = concepts::is_complex<T>::value;
    static constexpr bool v2Complex =
        concepts::is_complex<typename Array::type>::value;

    if constexpr (v1Complex) {
      T sum = 0;
      for (size_t i = 0; i < ELEMENTS; i++) {
        sum += std::conj(v1[i]) * v2[i];
      }
      return sum;
    }
    if constexpr (v2Complex) {
      T sum = 0;
      for (size_t i = 0; i < ELEMENTS; i++) {
        sum += v1[i] * v2[i].real();
      }
      return sum.real();
    }
    T sum = 0;
    for (size_t i = 0; i < ELEMENTS; i++) {
      sum += v1[i] * v2[i];
    }
    return sum;
  }

  // Squared absolute value (norm)

  typename concepts::is_complex<T>::real_type squared_absolute() const {
    if constexpr (concepts::is_complex<T>::value) {
      typename concepts::is_complex<T>::value_type sum = 0;
      const T *p = this->begin();
      for (size_t i = 0; i < ELEMENTS; i++) {
        sum += std::norm(p[i]);
      }
      return sum;
    } else {
      T sum = 0;
      const T *p = this->begin();
      for (size_t i = 0; i < ELEMENTS; i++) {
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

} // namespace org::simple

#endif // ORG_SIMPLE_M_NUM_ARRAY_H
