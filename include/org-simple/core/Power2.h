#ifndef ORG_SIMPLE_CORE_M_POWER2_H
#define ORG_SIMPLE_CORE_M_POWER2_H
/*
 * org-simple/core/Power2.h
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

#include <org-simple/core/Bits.h>

namespace org::simple::core {
template <typename unsigned_type = size_t> struct Power2For {

  /**
   * @return true if value is a power of two, false otherwise.
   */
  static constexpr bool is(const unsigned_type value) {
    return Bits<unsigned_type>::is_power_of_two(value);
  }

  /**
   * @return true if value is a power of two minus one, false otherwise.
   */
  static constexpr bool is_minus_one(const unsigned_type value) {
    return Bits<unsigned_type>::all_lesser_bits_set(value);
  }

  /**
   * @return value if it is a power of two, the next greater power of
   * two if that fits inside unsigned_type and zero otherwise.
   */
  static constexpr unsigned_type same_or_bigger(const unsigned_type value) {
    return value <= 2 ? 2 : Bits<unsigned_type>::fill(value - 1) + 1;
  }

  /**
   * Returns the value if it is already a multiple of power_of_two or the
   * next multiple of power_of_two otherwise. If power_of_two is not a power
   * of two, the alignment is done with the next higher power of two.
   *
   * @param value Value to be aligned
   * @param power_of_two The power of two to align to, or the next bigger
   * power of two.
   * @return the aligned value
   */
  static constexpr unsigned_type get_aligned_with(unsigned_type value,
                                                  unsigned_type power_of_two) {
    unsigned_type filled = Bits<unsigned_type>::fill(power_of_two >> 1);
    return (value + filled) & ~filled;
  }

  /**
   * Returns whether the value is a multiple of the power_of_two.
   *
   * @param value Value to check for alignment
   * @param power_of_two The power of two to align to, or the next bigger
   * power of two.
   * @return True of the value is aligned: a multiple of the provided power of
   * two.
   */
  static constexpr bool is_aligned_with(const unsigned_type value,
                                        const unsigned_type power_of_two) {
    return value == get_aligned_with(value, power_of_two);
  }
};

struct Power2 {

  /**
   * @return true if value is a power of two, false otherwise.
   */
  template <typename size_type>
  static constexpr bool is(const size_type value) {
    return Power2For<size_type>::is(value);
  }

  /**
   * @return true if value is a power of two minus one, false otherwise.
   */
  template <typename size_type>
  static constexpr bool is_minus_one(const size_type value) {
    return Power2For<size_type>::is_minus_one(value);
  }

  /**
   * @return value if it is a power of two, the next greater power of
   * two if that fits inside unsigned_type and zero otherwise.
   */
  template <typename size_type>
  static constexpr size_type same_or_bigger(const size_type value) {
    return Power2For<size_type>::same_or_bigger(value);
  }

  /**
   * Returns the value if it is already a multiple of power_of_two or the
   * next multiple of power_of_two otherwise. If power_of_two is not a power
   * of two, the alignment is done with the next higher power of two.
   *
   * @param value Value to be aligned
   * @param power_of_two The power of two to align to, or the next bigger
   * power of two.
   * @return the aligned value
   */
  template <typename size_type>
  static constexpr size_type get_aligned_with(size_type value,
                                              size_type power_of_two) {
    return Power2For<size_type>::get_aligned_with(value, power_of_two);
  }

  /**
   * Returns whether the value is a multiple of the power_of_two.
   *
   * @param value Value to check for alignment
   * @param power_of_two The power of two to align to, or the next bigger
   * power of two.
   * @return True of the value is aligned: a multiple of the provided power of
   * two.
   */
  template <typename size_type>
  static constexpr bool is_aligned_with(const size_type value,
                                        const size_type power_of_two) {
    return Power2For<size_type>::is_aligned_with(value, power_of_two);
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_CORE_M_POWER2_H
