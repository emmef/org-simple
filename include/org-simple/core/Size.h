#ifndef ORG_SIMPLE_SIZE_H
#define ORG_SIMPLE_SIZE_H
/*
 * org-simple/core/size.h
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

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <org-simple/core/Bits.h>
#include <org-simple/core/bounds.h>

namespace org::simple::core {

/**
 * Defines a size metric that is completely defined by the size_type that must
 * be unsigned and no bigger than size_t.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 */
template <typename size_type> struct SizeMetric;

/**
 * Defines a size metric like org::simple::core::SizeMetric<size_type> but
 * where sizes can only be expressed in a maximum number of bits:
 * size_bit_limit.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 * @tparam bits maximum number of bits used to express size, which
 * must be positive and cannot exceed the number of bits in size_type.
 */
template <typename size_type, unsigned bits> struct SizeMetricWithBitLimit;

/**
 * Defines a size metric like org::simple::core::SizeMetric<size_type> but
 * where sizes are hard limited to limit.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 * @tparam limit the maximum size limit, which
 * must be positive and cannot exceed the maximum size expressed by size_type.
 */
template <typename size_type, size_type limit> struct SizeMetricWithLimit;

/**
 * Defines a size metric that is based on Metric where the limits are based on
 * elements with the specified size in units. Units are bytes, unless the
 * Metric is already a derived one (for example an ArrayMetric).
 * @tparam Metric The base metric type, for example
 * org::simple::core::SizeMetric, org::simple::core::BitLimitedSizeMetric or
 * org::simple::core::LimitedSizeMetric.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 * @tparam element_size The number of units
 */
template <class Metric, typename size_type, size_type element_size>
struct ElementSizeMetric;

struct SizeValidator {
  template <typename size_type> static constexpr bool is_valid_size_type() {
    return std::is_unsigned_v<size_type> &&
           std::numeric_limits<size_type>::max() <=
               std::numeric_limits<size_t>::max();
  }

  template <typename size_type>
  static constexpr bool is_valid_size_type_limit(size_type limit) {
    return is_valid_size_type<size_type> && limit > 0;
  }

  template <typename size> struct Type {
    static_assert(
        is_valid_size_type<size>(),
        "org::simple::core::SizeValidator::Type: size_type must be unsigned "
        "and must not be able to contain larger values than size_t.");

    typedef size size_type;
    static constexpr size max = std::numeric_limits<size>::max();
  };

  template <typename size> struct TypeCheck {
    static_assert(
        is_valid_size_type<size>(),
        "org::simple::core::SizeValidator::Type: size_type must be unsigned "
        "and must not be able to contain larger values than size_t.");
    struct Values {
      size size_value = 0;
      bool bool_value = false;
    };
  };

  template <typename size, size limit> struct Limit : public Type<size> {
    static_assert(
        limit > 0,
        "org::simple::core::SizeValidator::Limit: limit must be positive.");

    typedef size size_type;
    static constexpr size max = limit;
  };

  template <typename size, unsigned size_bits_used>
  struct BitLimit : public Type<size> {
    static_assert(
        size_bits_used > 0 && size_bits_used <= sizeof(size) * 8,
        "org::simple::core::SizeValidator::BitLimit: size_bits_used be "
        "positive and not larger than number of bits in size_type.");

    typedef size size_type;
    static constexpr size max = size(1) << size_bits_used;
  };

  template <typename T, typename size_type>
  struct Metric : public Type<size_type> {
    static_assert(std::is_same_v<typename T::size_type, size_type>);
    static_assert(std::is_same_v<decltype(T::max), const size_type>);
    static_assert(std::is_same_v<decltype(T::max_index), const size_type>);
    static_assert(std::is_same_v<decltype(T::max_mask), const size_type>);

    static_assert(T::max > 0,
                  "org::simple::core::SizeValidator::Limit: T::max must "
                  "be positive.");
    static_assert(T::max_index <= T::max,
                  "org::simple::core::SizeValidator::Limit:"
                  " T::max_index cannot exceed T::max.");
    static_assert(T::max_mask <= T::max_index,
                  "org::simple::core::SizeValidator::Limit: T::max_mask cannot "
                  "exceed T::max_index.");
    static_assert(org::simple::core::Bits<size_type>::fill(T::max_mask) ==
                      T::max_mask,
                  "org::simple::core::SizeValidator::Limit: T::max_mask "
                  "must have all bits set");
  };
}; // struct SizeValidator

template <typename Metric, typename size>
struct SizeMetricBase : public SizeValidator::Type<size> {
  static_assert(std::is_same_v<typename Metric::size_type, size>);
  static_assert(std::is_same_v<decltype(Metric::max), const size>);
  static_assert(
      Metric::max > 0,
      "org::simple::core::SizeMetricBase: metric::max must be positive.");

  typedef size size_type;
  static constexpr size_type max = Metric::max;
  static constexpr size_type max_index =
      max == std::numeric_limits<size_type>::max() ? max : max - 1;
  static constexpr size_type max_mask =
      Bits<size_type>::bit_mask_not_exceeding(max_index);

  template <size_type element_size>
  using Elements = ElementSizeMetric<SizeMetricBase<Metric, size_type>,
                                     size_type, element_size>;

  /**
   * Returns whether value is valid.
   * @return true if value is valid, false otherwise.
   */
  template <typename T>
  static constexpr auto value(T value) noexcept
      -> decltype(SizeValidator::TypeCheck<T>::Values::bool_value) {
    return value > 0 && value <= max;
  }

  /**
   * Returns whether value is valid: not greater than max_index.
   * @return true if value is valid, false otherwise.
   */

  template <typename T>
  static constexpr auto index(T value) noexcept
      -> decltype(SizeValidator::TypeCheck<T>::Values::bool_value) {
    return value <= max_index;
  }

  /**
   * Returns whether the sum of two values v1 and v2, represents a valid size
   * that is non-zero and not greater than max.
   * @return true if the sum represents a valid size, false otherwise.
   */
  template <typename T>
  static constexpr auto sum(T v1, T v2) noexcept
      -> decltype(SizeValidator::TypeCheck<T>::Values::bool_value) {
    return v1 > 0 ? v1 <= max && max - v1 >= v2 : v2 > 0 && v2 <= max;
  }

  /**
   * Returns whether the product of two values v1 and v2, represents a valid
   * size that is non-zero and not greater than max.
   * @return true if the product represents a valid size, false otherwise.
   */
  template <typename T>
  static constexpr auto product(T v1, T v2) noexcept
      -> decltype(SizeValidator::TypeCheck<T>::Values::bool_value) {
    return v1 > 0 && v2 > 0 && max / v1 >= v2;
  }

  /**
   * Returns whether mask is valid: not greater than mask and all bits set.
   * @return true if mask if valid, false otherwise.
   */
  template <typename T>
  static constexpr auto mask(T mask) noexcept
      -> decltype(SizeValidator::TypeCheck<T>::Values::bool_value) {
    return mask <= max_mask && Bits<size_type>::fill(mask) == mask;
  }

  /**
   * Returns v if #value(v) is true, throws std::range_error otherwise.
   */
  template <typename T>
  static constexpr auto valid_value(T v)
  -> decltype(SizeValidator::TypeCheck<T>::Values::size_value) {
    if (value(v)) {
      return static_cast<size_type>(v);
    }
    throw std::range_error("org::simple::core::SizeMetricBase: Size value "
                           "must be positive and cannot exceed max.");
  }

  /**
   * Returns v if #index(v) is true, throws std::range_error otherwise.
   */
  template <typename T>
  static constexpr auto valid_index(T v)
  -> decltype(SizeValidator::TypeCheck<T>::Values::size_value) {
    if (index<T>(v)) {
      return v;
    }
    throw std::range_error("org::simple::core::SizeMetricBase: Index "
                           "value cannot exceed max_index.");
  }

  /**
   * Returns the sum (v1 + v2) if #sum(v1, v2) is true, throws
   * std::invalid_argument otherwise.
   */
  template <typename T>
  static constexpr auto valid_sum(T v1, T v2)
  -> decltype(SizeValidator::TypeCheck<T>::Values::size_value) {
    if (sum(v1, v2)) {
      return v1 + v2;
    }
    throw std::invalid_argument(
        "org::simple::core::SizeMetricBase: Sum of values must be positive and "
        "cannot exceed max.");
  }

  /**
   * Returns the product (v1 * v2) if #product(v1, v2) is true, throws
   * std::invalid_argument otherwise.
   */
  template <typename T>
  static constexpr auto valid_product(T v1, T v2)
  -> decltype(SizeValidator::TypeCheck<T>::Values::size_value) {
    if (product(v1, v2)) {
      return v1 * v2;
    }
    throw std::invalid_argument(
        "org::simple::core::SizeMetricBase: Product of values must be positive "
        "and cannot exceed max.");
  }

  /**
   * Returns v if #mask(v) is true, throws std::invalid_argument otherwise.
   */
  template <typename T>
  static constexpr auto valid_mask(T v)
  -> decltype(SizeValidator::TypeCheck<T>::Values::size_value) {
    if (mask(v)) {
      return v;
    }
    throw std::invalid_argument(
        "org::simple::core::SizeMetricBase: Mask must have all bits set and "
        "cannot exceed max_mask.");
  }

  ////////////////////////////////////
  /**
   * Returns the maximum number of elements in an array with elements of the
   * specified size in units. Units are bytes, unless the Metric that declared
   * this SizeValueChecks is already a derived one (for example an
   * ArrayMetric).
   * @param element_size The size of each element in units.
   * @return the maximum number of elements, that can be zero.
   */
  template <typename T>
  static constexpr size_type max_element_count(T element_size) {
    return max / valid_value(element_size);
  }
};

template <typename size> struct SizeMetric {
  typedef SizeValidator::Type<size> base;
  typedef SizeMetricBase<base, size> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type element_size>
  using Elements = typename check::template Elements<element_size>;
};

template <typename size, size limit> struct SizeMetricWithLimit {
  typedef SizeValidator::Limit<size, limit> base;
  typedef SizeMetricBase<base, size> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type element_size>
  using Elements = typename check::template Elements<element_size>;
};

template <typename size, unsigned size_bits_used>
struct SizeMetricWithBitLimit {
  typedef SizeValidator::BitLimit<size, size_bits_used> base;
  typedef SizeMetricBase<base, size> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type element_size>
  using Elements = typename check::template Elements<element_size>;
};

template <class Metric, typename size, size element_size>
struct ElementSizeMetric : private SizeValidator::Metric<Metric, size> {
  static_assert(element_size > 0 && element_size <= Metric::max,
                "org::simple::core::ArrayMetric: element_size must be positive "
                "and cannot exceed Metric::max");

  typedef Metric base;
  typedef SizeValidator::Limit<size, Metric::max / element_size> derived;
  typedef SizeMetricBase<derived, size> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type group_size>
  using Elements = typename check::template Elements<group_size>;
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_SIZE_H
