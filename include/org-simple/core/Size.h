#ifndef ORG_SIMPLE_SIZE_H
#define ORG_SIMPLE_SIZE_H
/*
 * org-simple/core/Size.h
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

namespace org::simple::traits {

/**
 * Checks whether @p unsigned_type is a valid size value type. A valid size
 * value type is  unsigned and its maximum value cannot exceed the maximum value
 * of size_t.
 * @tparam T The type to be checked to be valid ofr containing size.
 */
template <typename T>
static constexpr bool is_valid_size_type =
    std::is_unsigned_v<T> &&
    std::numeric_limits<T>::max() <= std::numeric_limits<size_t>::max();
} // namespace org::simple::traits

namespace org::simple::core {

using namespace org::simple::traits;

template <typename size_type> struct SizeType {
  static_assert(
      org::simple::traits::is_valid_size_type<size_type>,
      "org::simple::core::helper::TypeCheckSize<T>: T must be unsigned and "
      "must not be able to contain larger values than size_t.");

  static constexpr size_type max = std::numeric_limits<size_type>::max();
};

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    size_type_max() {
  return SizeType<size_type>::max;
}

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    is_valid_size_value(size_type value) {
  return is_within(value, size_type(1), size_type_max<size_type>());
}

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    size_type_max_index() {
  return size_type_max<size_type>() - 1;
}

template <typename size_type>
requires(is_valid_size_type<
         size_type>()) static constexpr bool is_valid_index_value(size_type
                                                                      index) {
  return is_within(index, size_type(0), size_type_max_index<size_type>());
}

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    get_max_index_value(size_type size_limit) {
  return size_limit > 0 ? size_limit - 1 : 0;
}

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    get_max_mask_from_index(size_type max_index) {
  return Bits<size_type>::bit_mask_not_exceeding(max_index);
}

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    get_max_mask_from_size(size_type max_size) {
  return max_size > 2 ? Bits<size_type>::bit_mask_not_exceeding(max_size - 1)
                      : 0;
}

template <typename size_type>
requires(
    is_valid_size_type<
        size_type>) static constexpr bool is_valid_number_of_size_bits(unsigned
                                                                           bits) {
  return bits <= sizeof(size_type) * 8;
}

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    get_size_for_size_bits(unsigned bits) {
  return bits == sizeof(size_type) * 8 ? std::numeric_limits<size_type>::max()
                                       : size_type(1) << bits;
}

template <typename size_type, unsigned bits>
requires(is_valid_size_type<size_type>) static constexpr size_type
    get_size_for_size_bits() {
  static_assert(is_valid_number_of_size_bits<size_type>(bits));
  return bits == sizeof(size_type) * 8 ? std::numeric_limits<size_type>::max()
                                       : size_type(1) << bits;
}

template <typename size_type = size_t,
          size_type SIZE_LIMIT = std::numeric_limits<size_type>::max()>
struct SizeMetricBase {
  typedef SizeType<size_type> Base;
  static_assert(
      SIZE_LIMIT > 0,
      "org::simple::core::SizeValues<T,LIMIT>::LIMIT must be positive.");

  typedef size_type type;
  static constexpr size_type limit = SIZE_LIMIT ? SIZE_LIMIT : Base::max;

  /**
   * Returns the maximum number of elements in an array with elements of the
   * specified size in units. Units are bytes, unless the Metric that declared
   * this SizeValueChecks is already a derived one (for example an
   * ArrayMetric).
   * @param element_size The size of each element in units.
   * @return the maximum number of elements, that can be zero.
   */
  static constexpr size_type max_element_count(size_type element_size,
                                               size_type size_limit) {
    return element_size > 0 ? size_limit / element_size : 0;
  }

  template <typename T>
  static constexpr size_type max_element_count(T element_size) {
    return max_element_count(element_size, limit);
  }

  template <size_type element_size>
  using Elements = SizeMetricBase<size_type, max_element_count(element_size)>;

  template <size_type v1, size_type v2> static constexpr size_type times() {
    static_assert(v1 * v2 >= v1);
    static_assert(v1 * v2 >= v2);
    static_assert(v1 * v2 <= limit);
    return v1 * v2;
  }

  template <size_type v1, size_type v2> static constexpr size_type plus() {
    static_assert(v1 + v2 >= v1);
    static_assert(v1 + v2 >= v2);
    static_assert(v1 + v2 <= limit);
    return v1 + v2;
  }

  static size_type product(size_type v1, size_type v2) {
    return Valid::product(v1, v2);
  }

  static size_type sum(size_type v1, size_type v2) {
    return Valid::sum(v1, v2);
  }

  struct IsValid {
    /**
     * Returns whether value is valid.
     * @return true if value is valid, false otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<T>) static constexpr bool value_with_limit(
        T value, size_type size_limit) {
      return is_within(value, size_type(1), size_limit);
    }
    template <typename T>
    requires(is_valid_size_type<T>) static constexpr bool value(T value) {
      return value_with_limit(value, limit);
    }

    /**
     * Returns whether value is valid: not greater than max_index.
     * @return true if value is valid, false otherwise.
     */

    template <typename T>
    requires(
        is_valid_size_type<
            size_type>) static constexpr bool index_with_limit(T value,
                                                               size_type
                                                                   size_limit) {
      return is_within(value, size_type(0), size_limit - 1);
    }
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr bool index(
        T value) {
      return index_with_limit(value, limit);
    }

    /**
     * Returns whether the sum of two values v1 and v2, represents a valid size
     * that is non-zero and not greater than max.
     * @return true if the sum represents a valid size, false otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<
             size_type>) static constexpr bool sum_with_limit(T v1, T v2,
                                                              size_type
                                                                  size_limit) {
      return v1 > 0 ? v1 <= size_limit && size_limit - v1 >= v2
                    : v2 > 0 && v2 <= size_limit;
    }
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr bool sum(T v1,
                                                                      T v2) {
      return sum_with_limit(v1, v2, limit);
    }

    /**
     * Returns whether the product of two values v1 and v2, represents a valid
     * size that is non-zero and not greater than max.
     * @return true if the product represents a valid size, false otherwise.
     */
    template <typename T>
    requires(
        is_valid_size_type<
            size_type>) static constexpr bool product_with_limit(T v1, T v2,
                                                                 size_type
                                                                     size_limit) {
      return v1 > 0 && v2 > 0 && size_limit / v1 >= v2;
    }
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr bool product(
        T v1, T v2) {
      return product_with_limit(v1, v2, limit);
    }

    /**
     * Returns whether mask is valid: smaller than size_limit and all bits set.
     * @return true if mask if valid, false otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<
             size_type>) static constexpr bool mask_with_limit(T mask,
                                                               size_type
                                                                   size_limit) {
      return size_limit > 1
                 ? mask <= size_limit - 1 && Bits<size_type>::fill(mask) == mask
                 : false;
    }
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr bool mask(T mask) {
      return mask_with_limit(mask, limit);
    }
  };

  struct Valid {
    /**
     * Returns v if #value(v) is true, throws std::range_error otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        value_with_limit(T v, size_type size_limit) {
      if (IsValid::value_with_limit(v, size_limit)) {
        return static_cast<size_type>(v);
      }
      throw std::range_error("org::simple::core::SizeValue::Valid: Size value "
                             "must be positive and cannot exceed max.");
    }
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T value(T v) {
      return value_with_limit(v, limit);
    }

    /**
     * Returns v if #index(v) is true, throws std::range_error otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        index_with_limit(T v, size_type size_limit) {
      if (IsValid::index_with_limit(v, size_limit)) {
        return v;
      }
      throw std::range_error("org::simple::core::SizeValue::Valid: Index "
                             "value cannot exceed max_index.");
    }
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T index(T v) {
      return index_with_limit(v, limit);
    }

    /**
     * Returns the sum (v1 + v2) if #sum(v1, v2) is true, throws
     * std::invalid_argument otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        sum_with_limit(T v1, T v2, size_type size_limit) {
      if (IsValid::sum_with_limit(v1, v2, size_limit)) {
        return v1 + v2;
      }
      throw std::invalid_argument("org::simple::core::SizeValue::Valid: Sum of "
                                  "values must be positive and "
                                  "cannot exceed max.");
    }

    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T sum(T v1, T v2) {
      return sum_with_limit(v1, v2, limit);
    }

    /**
     * Returns the product (v1 * v2) if #product(v1, v2) is true, throws
     * std::invalid_argument otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        product_with_limit(T v1, T v2, size_type size_limit) {
      if (IsValid::product_with_limit(v1, v2, size_limit)) {
        return v1 * v2;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: Product "
          "of values must be positive "
          "and cannot exceed max.");
    }

    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        product(T v1, T v2) {
      return product_with_limit(v1, v2, limit);
    }

    /**
     * Returns v if #mask(v) is true, throws std::invalid_argument otherwise.
     */
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        mask_with_limit(T v, size_type size_limit) {
      if (IsValid::mask_with_limit(v, size_limit)) {
        return v;
      }
      throw std::invalid_argument("org::simple::core::SizeValue::Valid: Mask "
                                  "must have all bits set and "
                                  "cannot exceed max_mask.");
    }

    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T mask(T v) {
      return mask_with_limit(v, limit);
    }

    /**
     * Return a mask that can be used for indexes that can at least address the
     * number of provided elements.
     */
    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        mask_for_elements_with_limit(T elements, size_type size_limit) {
      size_type value =
          Bits<size_type>::fill(value_with_limit(elements, limit) - 1);
      if (value < size_limit) {
        return value;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: cannot create a valid indexing "
          "mask for number of elements without exceeding size limit.");
    }

    template <typename T>
    requires(is_valid_size_type<size_type>) static constexpr T
        mask_for_elements(T elements) {
      return mask_for_elements_with_limit(elements, limit);
    }
  };
};

typedef SizeMetricBase<size_t> SizeMetric;

} // namespace org::simple::core

#endif // ORG_SIMPLE_SIZE_H
