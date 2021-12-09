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
template <typename S>
concept is_valid_size_type = core::unsignedIntegral<S> &&
    std::numeric_limits<S>::max()
<= std::numeric_limits<size_t>::max();

/**
 * Returns the maximum value for a valid size type.
 * @tparam S The size type.
 */
template <typename S>
requires(is_valid_size_type<S>) static constexpr S size_type_max =
    std::numeric_limits<S>::max();

/**
 * Returns whether it is ensured that \c T will not be word-wrapped implicitly
 * when used as an operand to a value of type \c S.
 * @tparam S The type to perform an operation on.
 * @tparam T The operand to the operation.
 */
template <typename S, typename T>
static constexpr bool is_unwrapped_operand =
    size_type_max<T> <= size_type_max<S>;

} // namespace org::simple::traits

namespace org::simple::core {

using namespace org::simple::traits;

template <typename size_type = size_t,
          size_type SIZE_LIMIT = std::numeric_limits<size_type>::max()>
struct SizeMetricBase;

template <typename size_type = size_t,
          size_type SIZE_LIMIT = std::numeric_limits<size_type>::max()>
class SizeBase;

template <typename size_type>
requires(is_valid_size_type<size_type>) static constexpr size_type
    get_max_index_value(size_type size_limit) {
  return size_limit > 0 ? size_limit - 1 : 0;
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

template <typename size_type, size_type SIZE_LIMIT> struct SizeMetricBase {
  static_assert(is_valid_size_type<size_type>);
  static_assert(
      SIZE_LIMIT > 0,
      "org::simple::core::SizeValues<T,LIMIT>::LIMIT must be positive.");

  typedef size_type type;
  template <typename S>
  requires(is_valid_size_type<S>) static constexpr bool unwrapped_operand =
      is_unwrapped_operand<size_type, S>;

  static constexpr size_type limit =
      SIZE_LIMIT ? SIZE_LIMIT : size_type_max<size_type>;
  static constexpr size_type max_element_count(size_type element_size,
                                               size_type size_limit) {
    return size_limit / std::max((size_type)1, element_size);
  }
  template <typename S>
  requires(unwrapped_operand<S>) static constexpr size_type
      max_element_count(S element_size, size_type size_limit) {
    return size_limit / std::max((size_type)1, (size_type)element_size);
  }

  /**
   * Returns the maximum number of elements in an array with elements of the
   * specified size in units. Units are bytes, unless the Metric that declared
   * this SizeValueChecks is already a derived one (for example an
   * ArrayMetric).
   * @tparam S The type of the element size, which must be a valid size type.
   * @param element_size The size of each element in units.
   * @return the maximum number of elements, that can be zero.
   */
  template <typename S>
  requires(unwrapped_operand<S>) static constexpr size_type
      max_element_count(S element_size) {
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
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool value_with_limit(
        S value, size_type size_limit) {
      return value > 0 && value <= size_limit;
    }
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool value(S value) {
      return value_with_limit(value, limit);
    }

    /**
     * Returns whether value is valid: not greater than max_index.
     * @return true if value is valid, false otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool index_with_limit(
        S value, size_type size_limit) {
      return value < size_limit;
    }
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool index(S value) {
      return index_with_limit(value, limit);
    }

    /**
     * Returns whether the sum of two values v1 and v2, represents a valid size
     * that is non-zero and not greater than max.
     * @return true if the sum represents a valid size, false otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool sum_with_limit(
        S v1, S v2, size_type size_limit) {
      return v1 <= size_limit && size_limit - v1 >= v2;
    }
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool sum(S v1, S v2) {
      return sum_with_limit(v1, v2, limit);
    }

    /**
     * Returns whether the difference of two values v1 and v2, represents a
     * valid size that is non-zero and not greater than max.
     * @return true if the difference represents a valid size, false otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool difference_with_limit(
        S v1, S v2, size_type size_limit) {
      return v1 > v2 && v1 - v2 <= size_limit;
    }
    template <typename S>
    requires(is_valid_size_type<S>) static constexpr bool difference(S v1,
                                                                     S v2) {
      return difference_with_limit(v1, v2, limit);
    }

    /**
     * Returns whether the product of two values v1 and v2, represents a valid
     * size that is non-zero and not greater than max.
     * @return true if the product represents a valid size, false otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool product_with_limit(
        S v1, S v2, size_type size_limit) {
      return v1 > 0 && v2 > 0 && size_limit / v1 >= v2;
    }
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool product(S v1, S v2) {
      return product_with_limit(v1, v2, limit);
    }

    /**
     * Returns whether the quotient of two values v1 and v2, represents a valid
     * size that is non-zero and not greater than max.
     * @return true if the quotient represents a valid size, false otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool quotient_with_limit(
        S v1, S v2, size_type size_limit) {
      return v1 > 0 && v2 > 0 && v1 / v2 <= size_limit;
    }
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool quotient(S v1, S v2) {
      return quotient_with_limit(v1, v2, limit);
    }

    /**
     * Returns whether mask is valid: smaller than size_limit and all bits set.
     * @return true if mask if valid, false otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool mask_with_limit(
        S mask, size_type size_limit) {
      return size_limit > 1 && mask < size_limit &&
             Bits<size_type>::fill(mask) == mask;
    }
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr bool mask(S mask) {
      return mask_with_limit(mask, limit);
    }
  };

  struct Valid {
    /**
     * Returns v if IsValid::value_with_limit(v,size_limit) is true, throws
     * std::range_error otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        value_with_limit(S v, size_type size_limit) {
      if (IsValid::value_with_limit(v, size_limit)) {
        return static_cast<size_type>(v);
      }
      throw std::range_error("org::simple::core::SizeValue::Valid: Size value "
                             "must be positive and cannot exceed size limit.");
    }
    /**
     * Returns v if IsValid::value(v) is true, throws std::range_error
     * otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S value(S v) {
      return value_with_limit(v, limit);
    }

    /**
     * Returns v if IsValid::index_with_limit(v,size_limit) is true, throws
     * std::range_error otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        index_with_limit(S v, size_type size_limit) {
      if (IsValid::index_with_limit(v, size_limit)) {
        return v;
      }
      throw std::range_error("org::simple::core::SizeValue::Valid: Index "
                             "value cannot exceed max_index for size limit.");
    }
    /**
     * Returns v if IsValid::index(v) is true, throws std::range_error
     * otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S index(S v) {
      return index_with_limit(v, limit);
    }

    /**
     * Returns the sum (v1 + v2) if IsValid::sum_with_limit(v1, v2, size_limit)
     * is true, throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        sum_with_limit(S v1, S v2, size_type size_limit) {
      if (IsValid::sum_with_limit(v1, v2, size_limit)) {
        return v1 + v2;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: Sum of values must be positive "
          "and cannot exceed size limit.");
    }

    /**
     * Returns the sum (v1 + v2) if IsValid::sum(v1, v2) is true, throws
     * std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S sum(S v1, S v2) {
      return sum_with_limit(v1, v2, limit);
    }

    /**
     * Returns the difference (v1 - v2) if IsValid::difference_with_limit(v1,
     * v2, size_limit) is true, throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        difference_with_limit(S v1, S v2, size_type size_limit) {
      if (IsValid::difference_with_limit(v1, v2, size_limit)) {
        return v1 - v2;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: Difference of values must be "
          "positive and cannot exceed size limit.");
    }

    /**
     * Returns the difference (v1 - v2) if IsValid::difference(v1, v2) is true,
     * throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S difference(S v1, S v2) {
      return sum_with_limit(v1, v2, limit);
    }

    /**
     * Returns the product (v1 * v2) if IsValid::product_with_limit(v1, v2,
     * size_limit) is true, throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        product_with_limit(S v1, S v2, size_type size_limit) {
      if (IsValid::product_with_limit(v1, v2, size_limit)) {
        return v1 * v2;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: Product of values must be "
          "positive and cannot exceed size limit.");
    }

    /**
     * Returns the product (v1 * v2) if IsValid::product(v1, v2,) is true,
     * throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S product(S v1, S v2) {
      return product_with_limit(v1, v2, limit);
    }

    /**
     * Returns the quotient (v1 / v2) if IsValid::quotient_with_limit(v1, v2,
     * size_limit) is true, throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        quotient_with_limit(S v1, S v2, size_type size_limit) {
      if (IsValid::quotient_with_limit(v1, v2, size_limit)) {
        return v1 / v2;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: Quotient of values must be "
          "positive and cannot exceed size limit.");
    }

    /**
     * Returns the quotient (v1 / v2) if IsValid::quotient(v1, v2,) is true,
     * throws std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S quotient(S v1, S v2) {
      return quotient_with_limit(v1, v2, limit);
    }

    /**
     * Returns v if IsValid::mask_with_limit(v, size_limit) is true, throws
     * std::invalid_argument otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        mask_with_limit(S v, size_type size_limit) {
      if (IsValid::mask_with_limit(v, size_limit)) {
        return v;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: Mask must have all bits set "
          "and cannot exceed maximum mask for size limit.");
    }
    /**
     * Returns v if IsValid::mask(v) is true, throws std::invalid_argument
     * otherwise.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S mask(S v) {
      return mask_with_limit(v, limit);
    }

    /**
     * Return a mask that can be used for indexes that can at least address the
     * number of provided elements.
     */
    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        mask_for_elements_with_limit(S elements, size_type size_limit) {
      size_type value =
          Bits<size_type>::fill(value_with_limit(elements, limit) - 1);
      if (value < size_limit) {
        return value;
      }
      throw std::invalid_argument(
          "org::simple::core::SizeValue::Valid: cannot create a valid indexing "
          "mask for number of elements without exceeding size limit.");
    }

    template <typename S>
    requires(unwrapped_operand<S>) static constexpr S
        mask_for_elements(S elements) {
      return mask_for_elements_with_limit(elements, limit);
    }
  };
};

template <typename size_type, size_type SIZE_LIMIT> class SizeBase {
  size_type size_;

public:
  typedef SizeMetricBase<size_type, SIZE_LIMIT> metric;

  explicit SizeBase(const SizeBase &source) = default;
  template <typename S, S LIMIT>
  explicit SizeBase(const SizeBase<S, LIMIT> &source)
      : size_(metric::Valid::value((S)source)) {}
  SizeBase(SizeBase &&source) noexcept = default;
  explicit SizeBase(size_type initial_size)
      : size_(metric::Valid::value(initial_size)) {}
  SizeBase &operator=(size_type new_size) {
    size_ = metric::Valid::value(new_size);
  }
  SizeBase &operator=(const SizeBase new_size) = default;
  template <typename S>
  requires(is_valid_size_type<S>) SizeBase &operator=(S new_size) {
    size_ = metric::Valid::value(new_size);
  }
  template <typename S, S LIMIT>
  requires(is_valid_size_type<S>) SizeBase &
  operator=(const SizeBase<S, LIMIT> &new_size) {
    size_ = metric::Valid::value((S)new_size);
  }
  operator size_type() const { return size_; }
  operator bool() const { return size_ != 0; }

  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) SizeBase &operator+=(S v) {
    size_ = metric::Valid::sum(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) SizeBase &operator-=(S v) {
    size_ = size_ > v ? size_ - v : metric::Valid::difference(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) SizeBase &operator*=(S v) {
    size_ = metric::Valid::product(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) SizeBase &operator/=(S v) {
    size_ = metric::Valid::quotient(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) size_type operator+(S v) const {
    return metric::Valid::sum(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) size_type operator-=(S v) const {
    return metric::Valid::difference(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) SizeBase &operator*(S v) const {
    return metric::Valid::product(size_, v);
  }
  template <typename S>
  requires(is_unwrapped_operand<size_type, S>) SizeBase &operator/(S v) const {
    return metric::Valid::quotient(size_, v);
  }
};

typedef SizeMetricBase<size_t> SizeMetric;
typedef SizeBase<size_t> Size;

} // namespace org::simple::core

#endif // ORG_SIMPLE_SIZE_H
