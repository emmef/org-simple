#ifndef ORG_SIMPLE_NUMBERS_H
#define ORG_SIMPLE_NUMBERS_H
/*
 * org-simple/core/Complex.h
 *
 * Added by michel on 2021-03-05
 * Copyright (C) 2015-2021 Michel Fleur.
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

namespace org::simple::core {


template <class T> struct Complex {
  static constexpr bool value = false;
  using real_type = T;
};

template <class T> struct Complex<std::complex<T>>  {
  static constexpr bool value = !Complex<T>::value;
  using real_type = T;
};

template <typename T> static constexpr bool is_complex_v = Complex<T>::value;

template <typename T>
static constexpr bool is_number = std::is_arithmetic_v<T> || is_complex_v<T>;

struct Numbers {
  template <typename T> static constexpr T squared_absolute(const T &v) {
    return std::norm(v);
  }

  template <typename T> static constexpr T squared_absolute(const std::complex<T> &v) {
    return std::norm(v);
  }

  template <typename T> static constexpr T times_conj(const T &v1, T &v2) {
    return v1 * std::conj(v2);
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_NUMBERS_H
