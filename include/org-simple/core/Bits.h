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
#include <cstdint>
#include <limits>
#include <type_traits>

namespace org::simple::core {

template <typename U>
concept unsignedIntegral = std::is_integral_v<U> && std::is_unsigned_v<U> &&
                           sizeof(U) <= 8;
template <typename I>
concept signedIntegral = std::is_integral_v<I> && std::is_signed_v<I> &&
                         sizeof(I) <= 8;

/**
 * Defines various bit-related operations for a size-like type.
 * @tparam unsigned_type An integral, unsigned type.
 */
template <typename unsigned_type> class Bits {
  static_assert(unsignedIntegral<unsigned_type>);

  /**
   * Basic workhorse for the fill_bits functions.
   * @tparam B The number of bits in the type, divided by 2. This is substituted
   * by default and supplying a different value leads to behaviour out of the
   * design scope of this function.
   * @param n The number to fill.
   * @return The number with all bits that are less significant than its most
   * significant bit set.
   * @see {@code fill_bits<U>(n)}
   */
  template <int B = sizeof(unsigned_type) * 4>
  static constexpr unsigned_type fill_bits(unsigned_type n) {
    if constexpr (B == 1) {
      return n | (n >> 1);
    } else {
      const unsigned_type n1 = fill_bits<B / 2>(n);
      return n1 | (n1 >> B);
    }
  }

  typedef typename std::make_signed<unsigned_type>::type signed_type;

public:
  /**
   * Returns the number of bits for the chosen unsigned_type.
   */
  static constexpr unsigned type_bits = 8 * sizeof(unsigned_type);

  /**
   * Sets all bits that are less significant than the most significant bit set
   * in the number, a.k. "right-fill".
   * @param n The number to fill.
   * @return The number with all bits that are less significant than its most
   * significant bit set.
   */
  static constexpr unsigned_type fill(unsigned_type value) {
    return fill_bits(value);
  };

  /**
   * Sets all bits that are less significant than the most significant bit set
   * in the number, a.k. "right-fill". Special handling for signed types i
   * necessary, as the right-shift operator can carry the most significant bit
   * set, which leads to the wrong result.
   * @param n The number to fill.
   * @return The number with all bits that are less significant than its most
   * significant bit set.
   */
  static constexpr signed_type fill_signed(signed_type n) {
    return reinterpret_cast<signed_type>(
        fill(reinterpret_cast<unsigned_type>(n)));
  }

  /**
   * Returns the number of leading zero bits in {@code x}. If {@code x} is zero,
   * the result is the number of bits in the type.
   * @param x The number to test.
   * @return the number of leading zero bits.
   */
  static constexpr int number_of_leading_zeroes(unsigned_type x) {
    if (unsigned_type(1) << (type_bits - 1) & x) {
      return 0;
    }
    signed_type xp = x;
    signed_type n = type_bits;
    signed_type c = type_bits >> 1;
    do {
      unsigned_type y = xp >> c;
      if (y != 0) {
        n = n - c;
        xp = y;
      }
      c >>= 1;
    } while (c != 0);
    return n - xp;
  }

  /**
   * Returns the number of the most significant bit in value or -1 when value is
   * zero. The number of the least significant bit is zero.
   * @param x The number to test.
   * @return the number of the most significant bit set, or -1 if value is zero.
   */
  static constexpr int most_significant(unsigned_type x) {
    return x ? type_bits - 1 - number_of_leading_zeroes(x) : -1;
  }

  /**
   * Returns the number of the most significant bit in value when it is a power
   * of two. The number of the least significant bit is zero. If value is not a
   * power of two, or zero, this function returns minus one minus the number of
   * the second most significant bit.
   * @return the number of the most significant bit set, or -1 if value is zero.
   */
  static constexpr int most_significant_single(unsigned_type value) {
    int r = most_significant(value);
    if (r < 1) {
      return r;
    }
    unsigned_type masked = value & fill(1 << (r - 1));
    return masked ? -1 - most_significant(masked) : r;
  }

  /**
   * Returns whether {@code x} is a power of two.
   * @param x The number ot check.
   * @return whether {@code x} is a power of two.
   */
  static constexpr bool is_power_of_two(unsigned_type x) {
    return x > 1 && (fill(x - 1) & x) == 0;
  }

  /**
   * Returns whether {@code x} has all bits set that are less significant than
   * its most significant bit. In other words, whether {@code x} is one, or a
   * power of two.
   * @param x The number ot check.
   * @return whether {@code x} has all bits set that are less significant than
   * its most significant bit
   */
  static constexpr bool all_lesser_bits_set(unsigned_type x) {
    return x && (fill(x) ^ x) == 0;
  }

  /**
   * Returns a bit mask that can be used to wrap addresses that include the
   * specified index.
   * @return the bit mask that includes index.
   */
  static constexpr unsigned_type bit_mask_including(unsigned_type index) {
    return index == 0 ? 0 : fill(index);
  }

  /**
   * Returns a bit mask that can be used to wrap addresses that must not exceed
   * the specified index. The minimum returned mask is 0.
   * @return the bit mask that includes index.
   */
  static constexpr unsigned_type bit_mask_not_exceeding(unsigned_type index) {
    const unsigned_type filled = fill(index);
    return index == 0 ? 0 : filled == index ? index : filled >> 1;
  }
};

namespace bits {
/**
 * Sets all bits that are less significant than the most significant bit set
 * in the number, a.k. "right-fill".
 * @param n The number to fill.
 * @return The number with all bits that are less significant than its most
 * significant bit set.
 */
template <typename unsigned_type>
static constexpr unsigned_type fill(unsigned_type value) {
  return Bits<unsigned_type>::fill(value);
};

/**
 * Sets all bits that are less significant than the most significant bit set
 * in the number, a.k. "right-fill". Special handling for signed types i
 * necessary, as the right-shift operator can carry the most significant bit
 * set, which leads to the wrong result.
 * @param n The number to fill.
 * @return The number with all bits that are less significant than its most
 * significant bit set.
 */
template <typename signed_type>
static constexpr signed_type fill_signed(signed_type n) {
  return Bits<typename std::make_unsigned<signed_type>::type>::fill(n);
}

/**
 * Returns the number of leading zero bits in {@code x}. If {@code x} is zero,
 * the result is the number of bits in the type.
 * @param x The number to test.
 * @return the number of leading zero bits.
 */
template <typename unsigned_type>
static constexpr int number_of_leading_zeroes(unsigned_type x) {
  return Bits<unsigned_type>::number_of_leading_zeroes(x);
}

/**
 * Returns the number of the most significant bit in value or -1 when value is
 * zero. The number of the least significant bit is zero.
 * @param x The number to test.
 * @return the number of the most significant bit set, or -1 if value is zero.
 */
template <typename unsigned_type>
static constexpr int most_significant(unsigned_type x) {
  return Bits<unsigned_type>::most_significant(x);
}

/**
 * Returns the number of the most significant bit in value when it is a power
 * of two. The number of the least significant bit is zero. If value is not a
 * power of two, or zero, this function returns minus one minus the number of
 * the second most significant bit.
 * @return the number of the most significant bit set, or -1 if value is zero.
 */
template <typename unsigned_type>
static constexpr int most_significant_single(unsigned_type value) {
  return Bits<unsigned_type>::most_significant_single(value);
}

/**
 * Returns whether {@code x} is a power of two.
 * @param x The number ot check.
 * @return whether {@code x} is a power of two.
 */
template <typename unsigned_type>
static constexpr bool is_power_of_two(unsigned_type x) {
  return Bits<unsigned_type>::is_power_of_two(x);
}

/**
 * Returns whether {@code x} has all bits set that are less significant than
 * its most significant bit. In other words, whether {@code x} is one, or a
 * power of two.
 * @param x The number ot check.
 * @return whether {@code x} has all bits set that are less significant than
 * its most significant bit
 */
template <typename unsigned_type>
static constexpr bool all_lesser_bits_set(unsigned_type x) {
  return Bits<unsigned_type>::all_lesser_bits_set(x);
}

/**
 * Returns a bit mask that can be used to wrap addresses that include the
 * specified index.
 * @return the bit mask that includes index.
 */
template <typename unsigned_type>
static constexpr unsigned_type bit_mask_including(unsigned_type index) {
  return Bits<unsigned_type>::bit_mask_including(index);
}

/**
 * Returns a bit mask that can be used to wrap addresses that must not exceed
 * the specified index. The minimum returned mask is 0.
 * @return the bit mask that includes index.
 */
template <typename unsigned_type>
static constexpr unsigned_type bit_mask_not_exceeding(unsigned_type index) {
  return Bits<unsigned_type>::bit_mask_not_exceeding(index);
}

} // namespace bits
} // namespace org::simple::core

#endif // ORG_SIMPLE_BITS_H
