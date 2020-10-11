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

#include <org-simple/core/Index.h>
#include <type_traits>

namespace org::simple::util {

template <typename T, size_t ELEMENTS> struct NumArray;

} // namespace org::simple::util

namespace org::simple::concepts {

template <size_t ELEM1, size_t ELEM2>
concept NumArrayWithCrossProduct = ELEM1 == ELEM2 == 3;

template <size_t ME, size_t OTHER> concept NumArrayBiggerThanMe = ME < OTHER;

template <size_t ME, size_t OTHER> concept NumArraySmallerThanMe = ME > OTHER;

template <size_t START, size_t END, size_t ELEMENTS>
concept NumArrayValidSlice = END < ELEMENTS &&START <= END;

template <size_t START, size_t SRC_ELEM, size_t DST_ELEM>
concept NumArrayValidGraft = START + SRC_ELEM <= DST_ELEM;

} // namespace org::simple::concept

namespace org::simple::util {


template <typename T, size_t ELEMENTS> struct NumArray {
  static_assert(std::is_arithmetic_v<T>);

  typedef T value_type;
  static constexpr size_t elements = ELEMENTS;

  typedef T ray[ELEMENTS];

  T x[ELEMENTS];

  void zero() noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] = 0;
    }
  }

  void fill(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] = v;
    }
  }

  NumArray &plus(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] += v;
    }
    return *this;
  }

  // Access

  T &operator[](size_t i) ORG_SIMPLE_INDEX_SAFE_NOEXCEPT {
    return x[core::Index::safe(i, ELEMENTS)];
  }

  const T &operator[](size_t i) const ORG_SIMPLE_INDEX_SAFE_NOEXCEPT {
    return x[core::Index::safe(i, ELEMENTS)];
  }

  NumArray &operator<<(const T *source) ORG_SIMPLE_DEREFERENCE_SAFE_NOEXCEPT {
    const T *src = core::Dereference::safe(source);
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] += src[i];
    }
    return *this;
  }

  void operator>>(const T *destination) ORG_SIMPLE_DEREFERENCE_SAFE_NOEXCEPT {
    const T *dst = core::Dereference::safe(destination);
    for (size_t i = 0; i < ELEMENTS; i++) {
      dst[i] += x[i];
    }
  }

  template <size_t ELEM>
  requires concepts::NumArrayBiggerThanMe<ELEMENTS, ELEM> NumArray &
  operator<<(const NumArray<T, ELEM> &source) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] += source.x[i];
    }
    return *this;
  }

  template <size_t ELEM>
  requires concepts::NumArraySmallerThanMe<ELEMENTS, ELEM> NumArray &
  operator<<(const NumArray<T, ELEM> &source) noexcept {
    size_t i;
    for (i = 0; i < ELEM; i++) {
      x[i] += source.x[i];
    }
    for (; i < ELEMENTS; i++) {
      x[i] = 0;
    }
    return *this;
  }

  template <size_t START, size_t END>
  requires concepts::NumArrayValidSlice<START, END, ELEMENTS> NumArray<T, END + 1 - START>
  slice() const noexcept {
    NumArray<T, END + 1 - START> r;
    for (size_t src = START, dst = 0; src <= END; src++, dst++) {
      r.x[dst] = x[src];
    }
    return r;
  }

  template <size_t START, size_t SRC_ELEM>
  requires concepts::NumArrayValidGraft<START, SRC_ELEM, ELEMENTS>
  NumArray & graft(const NumArray<T, SRC_ELEM> &source) noexcept {
    for (size_t src = 0, dst = START; src <= SRC_ELEM; src++, dst++) {
      x[dst] = source.x[src];
    }
    return *this;
  }

  // Negate

  NumArray operator-() const noexcept {
    NumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r.x[i] = -x[i];
    }
    return r;
  }

  // Add another array

  NumArray &operator+=(const NumArray &o) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] += o.x[i];
    }
    return *this;
  }

  NumArray operator+(const NumArray &o) const noexcept {
    NumArray r = *this;
    r += o;
    return r;
  }

  friend NumArray &operator+(const NumArray &o, NumArray &&a) noexcept {
    a += o;
    return a;
  }

  // Subtract an array

  NumArray &operator-=(const NumArray &o) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] -= o.x[i];
    }
    return *this;
  }

  NumArray operator-(const NumArray &o) const noexcept {
    NumArray r = *this;
    r -= o;
    return r;
  }

  // Multiply by scalar

  NumArray &operator*=(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] *= v;
    }
    return *this;
  }

  NumArray operator*(T v) const noexcept {
    NumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r.x[i] = x[i] * v;
    }
    return r;
  }

  friend NumArray operator*(T v, const NumArray &a) noexcept { return a * v; }

  friend NumArray &operator*(T v, NumArray &&a) noexcept {
    a *= v;
    return a;
  }

  // Divide by scalar

  NumArray &operator/=(T v) noexcept {
    for (size_t i = 0; i < ELEMENTS; i++) {
      x[i] /= v;
    }
    return *this;
  }

  NumArray operator/(T v) const noexcept {
    NumArray r;
    for (size_t i = 0; i < ELEMENTS; i++) {
      r.x[i] = x[i] / v;
    }
    return r;
  }

  // Multiply with array

  T in_product(const NumArray &o) const noexcept {
    T sum = x[0] * o.x[0];
    for (size_t i = 0; i < ELEMENTS; i++) {
      sum += x[i] * o.x[i];
    }
    return sum;
  }

  T self() const noexcept { return in(*this); }

  template <size_t ELEM>
  requires concepts::NumArrayWithCrossProduct<ELEM, ELEMENTS>
      NumArray cross_product(const NumArray<T, ELEM> &o) const noexcept {
    NumArray r;
    r.x[0] = x[1] * o.x[2] - x[2] * o.x[1];
    r.x[1] = x[2] * o.x[0] - x[0] * o.x[2];
    r.x[2] = x[0] * o.x[1] - x[1] * o.x[0];
    return r;
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_NUMARRAY_H
