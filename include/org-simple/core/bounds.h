#ifndef ORG_SIMPLE_CORE_M_BOUNDS_H
#define ORG_SIMPLE_CORE_M_BOUNDS_H
/*
 * org-simple/core/bounds.h
 *
 * Added by michel on 2020-09-21
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

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace org::simple::core {

template <typename T>
[[nodiscard]] static constexpr bool is_within(const T v, const T min,
                                              const T max) {
  static_assert(std::is_arithmetic_v<T>);
  return v >= min && v <= max;
}

template <typename T, typename V>
[[nodiscard]] static constexpr bool is_within(const V v, const T min,
                                              const T max) {
  if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<V>) {
    return v >= min && v <= max;
  } else if constexpr (std::is_unsigned_v<T>) {
    if constexpr (std::is_unsigned_v<V>) {
      return v >= min && v <= max;
    } else {
      return v >= 0 && static_cast<T>(v) >= min && static_cast<T>(v) <= max;
    }
  } else { // T is signed
    if constexpr (std::is_unsigned_v<V>) {
      return v <= static_cast<V>(std::numeric_limits<T>::max()) &&
             static_cast<T>(v) >= min && static_cast<T>(v) <= max;
    } else {
      return v >= min && v <= max;
    }
  }
}

template <typename T>
[[nodiscard]] static constexpr bool is_within_excl(const T v, const T min,
                                                   const T max) {
  static_assert(std::is_arithmetic_v<T>);
  return v > min && v < max;
}

template <typename T, typename V>
[[nodiscard]] static constexpr bool is_within_excl(const V v, const T min,
                                                   const T max) {
  if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<V>) {
    return v > min && v < max;
  } else if constexpr (std::is_unsigned_v<T>) {
    if constexpr (std::is_unsigned_v<V>) {
      return v > min && v < max;
    } else {
      return v >= 0 && static_cast<T>(v) > min && static_cast<T>(v) < max;
    }
  } else { // T is signed
    if constexpr (std::is_unsigned_v<V>) {
      return v <= static_cast<V>(std::numeric_limits<T>::max()) &&
             static_cast<T>(v) > min && static_cast<T>(v) < max;
    } else {
      return v > min && v < max;
    }
  }
}

} // namespace org::simple::core

#endif // ORG_SIMPLE_CORE_M_BOUNDS_H
