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
 * Returns whether @p size_type is a valid size value type. A valid size value
 * type is  unsigned and its maximum value cannot exceed the maximum value of
 * size_t.
 * @tparam size_type The type to be checked for validity.
 * @return @c true if @p size_type is a valid size value type.
 */
template <typename size_type> static constexpr bool is_valid_size_type() {
  return std::is_unsigned_v<size_type> &&
         std::numeric_limits<size_type>::max() <=
             std::numeric_limits<size_t>::max();
}

/**
 * If \p size is a valid size type, as per @c is_valid_size_type<size>, defines
 * some additional members that can be used by other types or that can be used
 * in automatic deduction of method return types using declspec.
 *
 * @tparam size The size value type to be checkked.
 */
template <typename size> struct SizeValue {
  static_assert(is_valid_size_type<size>(),
                "org::simple::core::helper::SizeValidator::Type: size_type "
                "must be unsigned "
                "and must not be able to contain larger values than size_t.");

  /**
   * The validated size value type.
   */
  typedef size size_type;
  /**
   * The maximum value this size type can hold.
   */
  static constexpr size_type max = std::numeric_limits<size>::max();

  /**
   * Value structure, whose members can be used to deduct return types for
   * methods.
   */
  struct Values {
    size size_value = 0;
    bool bool_value = false;
  };
};

/**
 * Defines the size value type and the explicit limit to the maximum value if
 * the size value type and the limit are valied.
 * @tparam size The size value type.
 * @tparam limit The explicit limit on the value that must be positive and
 * cannot exceed the maximum type for the size value type.
 */
template <typename size, size limit>
struct SizeValueWithLimit : public SizeValue<size> {
  static_assert(limit > 0, "org::simple::core::helper::SizeValidator::Limit: "
                           "limit must be positive.");

  typedef size size_type;
  static constexpr size max = limit;
};

/**
 * Defines the size value type and the maximum value that is determined by the
 * explicitly specified maximum number of bits to be used by that value. The
 * number of bits must be positive (at least one bit used). If the number of
 * bits equals the number of bits in the size value type, max will be the
 * maximum number that can be represented by that type. For all other values,
 * the maximum is 1 shifted left by the number of specified bits.
 * @tparam size The size value type.
 * @tparam size_bits_used The number of bits used for size values that cannot
 * exceed the number of bits in the size value type.
 */
template <typename size, unsigned size_bits_used>
struct SizeValueWithBitLimit : public SizeValue<size> {
  static_assert(
      size_bits_used > 0 && size_bits_used <= sizeof(size) * 8,
      "org::simple::core::helper::SizeValidator::BitLimit: size_bits_used be "
      "positive and not larger than number of bits in size_type.");

  typedef size size_type;
  static constexpr size max = size_bits_used == sizeof(size) * 8
                                  ? std::numeric_limits<size>::max()
                                  : (size(1) << size_bits_used);
};

/**
 * Verifies whether \p SizeValue is a valid SizeValue with a correct size value
 * type and a non-zero maximum value.
 * @tparam SizeValue The type to verify.
 */
template <typename SizeValue> struct ValidSizeValue {
  static_assert(is_valid_size_type<typename SizeValue::size_type>());
  /**
   * The size value type defined by \p Metric.
   */
  typedef typename SizeValue::size_type size_type;
  typedef SizeValue Size;

  static_assert(std::is_same_v<decltype(SizeValue::max), const size_type>);
  static_assert(SizeValue::max > 0,
                "org::simple::core::ValidSizeValue::max must be positive.");
};

/**
 * Defines standard members of a Size metric that is based on a SizeValueType,
 * SizeLimit or SizeBitLimit.
 * @tparam SizeValueType The base type that must be valid according to
 * validSizeValueType.
 */
template <typename SizeValueType> struct SizeMetricBase;

/**
 * Validates a size metric type @p Metric. If @p Metric is a valid metric type
 * ValidSizeMetric defines extra types that reflect the metric and its size
 * value type.
 * @tparam Metric The metric type to validate.
 */
template <typename Metric> struct ValidSizeMetric {
  static_assert(is_valid_size_type<typename Metric::size_type>());
  /**
   * The size value type defined by \p Metric.
   */
  typedef typename Metric::size_type size_type;
  /**
   * The metric type given by \p Metric.
   */
  typedef Metric metric;
  static_assert(std::is_same_v<decltype(Metric::max), const size_type>);
  static_assert(std::is_same_v<decltype(Metric::max_index), const size_type>);
  static_assert(std::is_same_v<decltype(Metric::max_mask), const size_type>);
  static_assert(Metric::max > 0,
                "org::simple::core::helper::SizeValidator::Limit: T::max must "
                "be positive.");
  static_assert(Metric::max_index <= Metric::max,
                "org::simple::core::helper::SizeValidator::Limit:"
                " T::max_index cannot exceed T::max.");
  static_assert(
      Metric::max_mask <= Metric::max_index,
      "org::simple::core::helper::SizeValidator::Limit: T::max_mask cannot "
      "exceed T::max_index.");
  static_assert(Bits<size_type>::fill(Metric::max_mask) == Metric::max_mask,
                "org::simple::core::helper::SizeValidator::Limit: T::max_mask "
                "must have all bits set");

  typedef typename metric::check check;
  static_assert(requires { check::valid_value(size_type(1)); });
  static_assert(requires { check::valid_sum(size_type(1), size_type(0)); });
  static_assert(requires { check::valid_product(size_type(1), size_type(1)); });
  static_assert(
      std::is_same_v<size_type(size_type),
                     decltype(check::template valid_value<size_type>)>);
  static_assert(std::is_same_v<size_type(size_type, size_type),
                               decltype(check::template valid_sum<size_type>)>);
  static_assert(
      std::is_same_v<size_type(size_type, size_type),
                     decltype(check::template valid_product<size_type>)>);
};

template <class Metric, typename size, size element_size>
struct ElementSizeMetric;

/**
 * Defines a size metric that is completely defined by the size_type that must
 * be unsigned and no bigger than size_t.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 */
template <typename size> struct SizeMetric {
  typedef SizeValue<size> base;
  typedef SizeMetricBase<base> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type element_size>
  using Elements = ElementSizeMetric<SizeMetric<size>, size_type, element_size>;
};

/**
 * Defines a size metric like org::simple::core::SizeMetric<size_type> but
 * where sizes are hard limited to limit.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 * @tparam limit the maximum size limit, which
 * must be positive and cannot exceed the maximum size expressed by size_type.
 */
template <typename size, size limit> struct SizeMetricWithLimit {
  typedef SizeValueWithLimit<size, limit> base;
  typedef SizeMetricBase<base> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type element_size>
  using Elements = ElementSizeMetric<SizeMetricWithLimit<size, limit>,
                                     size_type, element_size>;
};

/**
 * Defines a size metric like org::simple::core::SizeMetric<size_type> but
 * where sizes can only be expressed in a maximum number of bits:
 * size_bit_limit.
 * @tparam size_type The underlying size type used, which must be unsigned and
 * no bigger than size_t.
 * @tparam bits maximum number of bits used to express size, which
 * must be positive and cannot exceed the number of bits in size_type.
 */
template <typename size, unsigned size_bits_used>
struct SizeMetricWithBitLimit {
  typedef SizeValueWithBitLimit<size, size_bits_used> base;
  typedef SizeMetricBase<base> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type element_size>
  using Elements =
      ElementSizeMetric<SizeMetricWithBitLimit<size, size_bits_used>, size_type,
                        element_size>;
};

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
template <class Metric, typename size, size element_size>
struct ElementSizeMetric {
  static_assert(element_size > 0 && element_size <= Metric::max,
                "org::simple::core::ArrayMetric: element_size must be positive "
                "and cannot exceed Metric::max");

  typedef typename ValidSizeMetric<Metric>::metric base;
  typedef SizeValueWithLimit<size, Metric::max / element_size> derived;
  typedef SizeMetricBase<derived> check;
  typedef size size_type;

  static constexpr size_type max = check::max;
  static constexpr size_type max_index = check::max_index;
  static constexpr size_type max_mask = check::max_mask;

  template <size_type group_size>
  using Elements =
      ElementSizeMetric<ElementSizeMetric<Metric, size, element_size>,
                        size_type, group_size>;
};

/**
 * Defines a size metric basis. Based on a valid Sizevaluetype defines related
 * maximum valies for indexes, bit masks and a number of functions that can
 * validate values, their sums and products in a safe fashion (avoiding
 * wraparounds an such stuff).
 * @tparam SizeValueType The Type that the metric is based on.
 */
template <typename SizeValueType> struct SizeMetricBase {
  typedef ValidSizeValue<SizeValueType> Value;

  typedef typename Value::size_type size_type;
  static constexpr size_type max = Value::Size::max;
  static constexpr size_type max_index = max - 1;
  static constexpr size_type max_mask =
      Bits<size_type>::bit_mask_not_exceeding(max_index);

  /**
   * Returns whether value is valid.
   * @return true if value is valid, false otherwise.
   */
  template <typename T>
  static constexpr auto value(T value) noexcept
      -> decltype(SizeValue<T>::Values::bool_value) {
    return value > 0 && value <= max;
  }

  /**
   * Returns whether value is valid: not greater than max_index.
   * @return true if value is valid, false otherwise.
   */

  template <typename T>
  static constexpr auto index(T value) noexcept
      -> decltype(SizeValue<T>::Values::bool_value) {
    return value <= max_index;
  }

  /**
   * Returns whether the sum of two values v1 and v2, represents a valid size
   * that is non-zero and not greater than max.
   * @return true if the sum represents a valid size, false otherwise.
   */
  template <typename T>
  static constexpr auto sum(T v1, T v2) noexcept
      -> decltype(SizeValue<T>::Values::bool_value) {
    return v1 > 0 ? v1 <= max && max - v1 >= v2 : v2 > 0 && v2 <= max;
  }

  /**
   * Returns whether the product of two values v1 and v2, represents a valid
   * size that is non-zero and not greater than max.
   * @return true if the product represents a valid size, false otherwise.
   */
  template <typename T>
  static constexpr auto product(T v1, T v2) noexcept
      -> decltype(SizeValue<T>::Values::bool_value) {
    return v1 > 0 && v2 > 0 && max / v1 >= v2;
  }

  /**
   * Returns whether mask is valid: not greater than mask and all bits set.
   * @return true if mask if valid, false otherwise.
   */
  template <typename T>
  static constexpr auto mask(T mask) noexcept
      -> decltype(SizeValue<T>::Values::bool_value) {
    return mask <= max_mask && Bits<size_type>::fill(mask) == mask;
  }

  /**
   * Returns v if #value(v) is true, throws std::range_error otherwise.
   */
  template <typename T>
  static constexpr auto valid_value(T v)
      -> decltype(SizeValue<T>::Values::size_value) {
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
      -> decltype(SizeValue<T>::Values::size_value) {
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
      -> decltype(SizeValue<T>::Values::size_value) {
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
      -> decltype(SizeValue<T>::Values::size_value) {
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
      -> decltype(SizeValue<T>::Values::size_value) {
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

template <class Metric> struct Size {
  typedef typename ValidSizeMetric<Metric>::metric metric;
  typedef typename metric::check check;
  typedef typename metric::size_type size_type;

  explicit Size(size_type value) : value_(check::valid_value(value)) {}
  Size(const Size &size) noexcept = default;
  Size(Size &&size) noexcept = default;

  [[nodiscard]] size_type value() const noexcept { return value_; }

  [[nodiscard]] explicit operator size_type() const noexcept { return value_; }

  Size &operator=(const Size size) noexcept {
    value_ = size.value_;
    return *this;
  }

  template <typename T> Size &operator=(Size<T> value) {
    value_ = check::valid_value(value.value());
    return *this;
  }

  template <typename T> Size &operator=(T value) {
    value_ = check::valid_value(value);
    return *this;
  }

  Size &operator+=(const Size size) {
    value_ = check::valid_sum(value_, size.value_);
    return *this;
  }

  template <typename T> Size &operator+=(Size<T> value) {
    auto v = value.value();
    value_ = check::valid_sum(value_, check::valid_value(v));
    return *this;
  }

  template <typename T> Size &operator+=(T value) {
    value_ = check::valid_sum(value_, check::valid_value(value));
    return *this;
  }

  Size &operator*=(const Size size) {
    value_ = check::valid_product(value_, size.value_);
    return *this;
  }

  template <typename T> Size &operator*=(Size<T> value) {
    auto v = value.value();
    value_ = check::valid_product(value_, check::valid_value(v));
    return *this;
  }

  template <typename T> Size &operator*=(T value) {
    value_ = check::valid_product(value_, check::valid_value(value));
    return *this;
  }

private:
  size_type value_;
};

template <class T, typename A> static Size<T> operator+(Size<T> size, A other) {
  size += other;
  return size;
}

template <class T, typename A> static Size<T> operator*(Size<T> size, A other) {
  size *= other;
  return size;
}

template <class T, typename A>
static auto operator+(A other, Size<T> size)
    -> decltype(SizeValue<A>::Values::size_value) {
  size += other;
  return size.value();
}

template <class T, typename A> static auto operator*(A other, Size<T> size)
-> decltype(SizeValue<A>::Values::size_value) {
  size *= other;
  return size;
}

typedef SizeMetric<size_t> SizeTMetric;
typedef Size<SizeTMetric> SizeType;
typedef SizeMetric<unsigned> IntSizeTMetric;
typedef Size<IntSizeTMetric> IntSizeType;
typedef SizeMetric<unsigned short> ShortSizeTMetric;
typedef Size<ShortSizeTMetric> ShortSizeType;

} // namespace org::simple::core

#endif // ORG_SIMPLE_SIZE_H
