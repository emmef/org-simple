#ifndef ORG_SIMPLE_BITS_H
#define ORG_SIMPLE_BITS_H
/*
 * org-simple/core/bits.h
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

#include <cstddef>
#include <limits>
#include <type_traits>

namespace org::simple::core {

/**
 * Defines various bit-related operations for a size-like type.
 * @tparam unsigned_type An integral, unsigned type.
 */
template <typename unsigned_type = size_t> class Bits {
  static_assert(std::is_integral_v<unsigned_type> &&
                    !std::is_signed_v<unsigned_type>,
                "Power2:: Type must be an integral, unsigned type");

  template <int N> static constexpr unsigned_type fillN(unsigned_type n) {
    return N < 2 ? n : fillN<N / 2>(n) | (fillN<N / 2>(n) >> (N / 2));
  }

public:
  /**
   * Returns the number of bits for the chosen unsigned_type.
   */
  static constexpr unsigned type_bits = 8 * sizeof(unsigned_type);

  /**
   * Fill all bits that are less significant than the most significant bit.
   * @param value The value to fill bits
   * @return value with all bits set that are less significant than the most
   * significant bit.
   */
  static constexpr unsigned_type fill(unsigned_type value) {
    return fillN<8 * sizeof(unsigned_type)>(value);
  };

  /**
   * Returns the number of the most significant bit in value or -1 when value is
   * zero. The number of the least significant bit is zero.
   * @return the number of the most significant bit set, or -1 if value is zero.
   */
  static constexpr int most_significant(unsigned_type value) {
    int bit = sizeof(unsigned_type) * 8 - 1;
    while (bit >= 0 && (value & (unsigned_type(1) << bit)) == 0) {
      bit--;
    }
    return bit;
  }

  /**
   * Returns the number of the most significant bit in value when it is a power
   * of two. The number of the least significant bit is zero. If value is not a
   * power of two, or zero, this function returns minus one minus the number of
   * the second most significant bit.
   * @return the number of the most significant bit set, or -1 if value is zero.
   */
  static constexpr int most_significant_single(unsigned_type value) {
    int bit = sizeof(unsigned_type) * 8 - 1;
    while (bit >= 0 && (value & (unsigned_type(1) << bit)) == 0) {
      bit--;
    }
    if (bit < 1) {
      return bit;
    }
    int lower_bit = bit - 1;
    while (lower_bit >= 0) {
      if (value & (unsigned_type(1) << lower_bit)) {
        return -lower_bit - 1;
      }
      lower_bit--;
    }
    return bit;
  }

  /**
   * Returns a bit mask that can be used to wrap addresses that include the
   * specified index. The minimum returned mask is 1.
   * @return the bit mask that includes index.
   */
  static constexpr unsigned_type bit_mask_including(unsigned_type index) {
    return index < 2 ? 1 : fill(index);
  }

  /**
   * Returns a bit mask that can be used to wrap addresses that must not exceed
   * the specified index. The minimum returned mask is 1.
   * @return the bit mask that includes index.
   */
  static constexpr unsigned_type bit_mask_not_exceeding(unsigned_type index) {
    return index < 2 ? 0 : fill(index) == index ? index : fill(index) >> 1;
  }

  /**
   * Returns the maximum size that corresponds with the number of bits or the
   * maximum value of unsigned_type if that is smaller.
   * @return the maximum size
   */
  static constexpr unsigned_type max_value_for_bits(unsigned size_bits) {
    return size_bits >= type_bits ? std::numeric_limits<unsigned_type>::max()
                                  : unsigned_type(1) << size_bits;
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_BITS_H
