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
struct MemoryModel {
  static constexpr unsigned size_t_bits = 8 * sizeof(size_t);
  static constexpr unsigned address_bits =
#if defined(SIMPLE_CORE_MEMORY_MODEL_ADDRESS_BITS)
      clamp(int(SIMPLE_CORE_MEMORY_MODEL_ADDRESS_BITS), 1, size_t_bits);
#else
      size_t_bits;
  /*
   * The actual (virtual) addressing space is limited by the hardware and the
   * operating system memory model(s). These limits are ignored here. First,
   * because they are generally so high that they are rather theoretical.
   * Second, because it is difficult to find out the correct limits for all
   * combinations of operating systems and architectures. Third, limiting the
   * size does not protect the programmer from the combination of a too-high
   * offset and a size. Unless there is a water-tight way of solving this, we
   * keep the theoretical limit and assume people in the know provide a proper
   * value/expression for SIMPLE_CORE_MEMORY_MODEL_ADDRESS_BITS.
   */
#endif

  static constexpr size_t address_max =
      Bits<size_t>::max_value_for_bits(address_bits);
};

/**
 * Describes properties of sizes that are stored in the underlying SIZE data
 * type, that must be integral, unsigned and cannot be larger than std::size_t.
 *
 * Sizes can be artificially limited to a reduced number of bits by setting
 * size_bit_limit to a non-zero value. Positive value set the number of bits,
 * negative values subtract from the number of bits in size_type.
 *
 * @tparam SIZE The integral, unsigned type that contains sizes.
 * @tparam size_bit_limit The artificially reduced number of bits to represent
 * sizes.
 */
template <typename SIZE = size_t, int size_bit_limit = 0> struct SizeType {
  static_assert(std::is_integral_v<SIZE>,
                "org::simple::core::size::system::SizeType: size_type must be an "
                "integral type.");

  static_assert(!std::is_signed_v<SIZE>, "org::simple::core::size::system::SizeType: "
                                         "size_type must be an unsigned type.");

  static_assert(
      sizeof(SIZE) <= sizeof(size_t),
      "org::simple::core::size::system::SizeType: size_type cannot be larger "
      "than std::size_t.");

  static_assert(int(MemoryModel::size_t_bits) + size_bit_limit > 0,
                "SizeType::number of unused bits for size cannot equal or "
                "exceed number of bits in size_type");
  static_assert(size_bit_limit <= MemoryModel::size_t_bits,
                "SizeType: explicit size_bit_limit cannot exceed number of "
                "bits in size_type.");

  using size_type = SIZE;

  /**
   * The number of bits in the underlying size_type.
   */
  static constexpr unsigned type_bits = 8 * sizeof(size_type);

  /**
   * The maximum value that can be represented by the underlying size_type.
   */
  static constexpr size_type type_max = std::numeric_limits<size_type>::max();

  /**
   * The number of bits used to represent size values.
   */
  static constexpr unsigned size_bits = size_bit_limit > 0 ? size_bit_limit
                                 : MemoryModel::size_t_bits + size_bit_limit;

  /**
   * The maximum value of a size.
   */
  static constexpr size_type max =
      Bits<size_type>::max_value_for_bits(size_bits);

  /**
   * The maximum index to address structures with a size. This is equal to max
   * if max is equal to std::numeric_limits<size_type>::max() and max minus one
   * otherwise. This value is always a power of two minus one.
   */
  static constexpr size_type max_index =
      type_bits == size_bits ? std::numeric_limits<size_type>::max()
                             : (size_type(1) << size_bits) - 1;

  /**
   * The maximum bit mask that can be used for "wrapped" addressing structures
   * like circular buffers.
   */
  static constexpr size_type max_bit_mask =
      Bits<size_type>::bit_mask_not_exceeding(max_index);

  /**
   * Returns whether size is valid: non-zero and not greater than max.
   * @return true if size if valid, false otherwise.
   */
  static constexpr bool is_valid(size_type size) noexcept {
    return Unsigned::is_nonzero_not_greater(size, max);
  }

  /**
   * Returns whether index is valid: not greater than max_index.
   * @return true if index if valid, false otherwise.
   */
  static constexpr bool is_valid_index(size_type index) noexcept {
    return Unsigned::is_not_greater(index, max_index);
  }

  /**
   * Returns whether size is valid, where size is of another size_type.
   * @return true if size if valid, false otherwise.
   */
  template <typename src_size_type>
  static constexpr bool is_valid(src_size_type size) noexcept {
    return Unsigned::is_nonzero_not_greater(size, max);
  }

  /**
   * Returns whether the sum of two values v1 and v2, represents a valid size
   * that is non-zero and not greater than max.
   * @return true if the sum represents a valid size, false otherwise.
   */
  static constexpr bool is_valid_sum(size_type v1, size_type v2) noexcept {
    return Unsigned::is_sum_nonzero_not_greater(v1, v2, max);
  }

  /**
   * Returns whether the product of two values v1 and v2, represents a valid
   * size that is non-zero and not greater than max.
   * @return true if the product represents a valid size, false otherwise.
   */
  static constexpr bool is_valid_product(size_type v1, size_type v2) noexcept {
    return Unsigned::is_product_nonzero_not_greater(v1, v2, max);
  }

  /**
   * Returns the maximum number of elements in an array with elements of the
   * specified size.
   * @param element_size The size of each element.
   * @return the maximum number of elements, that can be zero.
   */
  static constexpr size_type max_count_for_element_size(size_t element_size) {
    return max / org::simple::core::maximum(size_t(1), element_size);
  }

  /**
   * Returns the maximum index to address an array with elements of the
   * specified size.
   * @param element_size The size of each element.
   * @return the maximum index, that can be zero.
   */
  static constexpr size_type max_index_for_element_size(size_t element_size) {
    return element_size > 1 ? max_count_for_element_size(element_size) - 1
                            : max_index;
  }

  /**
   * Returns the maximum bit mask that can be used for "wrapped" addressing
   * structures, when backed by an array with elements of the specified size.
   */
  static constexpr size_type
  max_bit_mask_for_element_size(size_t element_size) {
    return Bits<size_type>::bit_mask_not_exceeding(
        max_index_for_element_size(element_size));
  }
};

static constexpr size_t system_max_size_in_bytes = SizeType<size_t>::max;

static constexpr size_t system_max_byte_index = SizeType<size_t>::max_index;

/**
 * Represents the size_type-represented, non-zero size of an array with elements
 * of element_size. In addition, a number of static properties and utilities are
 * provided.
 *
 * The size range can be artificially reduced by setting max_size_bits to a
 * non-zero value: this mechanism is explained in SizeType<size_type,
 * max_size_bits>.
 *
 * @tparam element_size The size of each element.
 * @tparam SIZE The integral, unsigned type that contains sizes.
 *  @tparam size_bit_limit The artificially reduced number of bits to represent
 * sizes.
 */
template <size_t element_size, typename SIZE = std::size_t,
          int max_size_bits = 0>
struct Size {
  static_assert(element_size > 0, "Size: element_size cannot be zero.");

  using size_type = SIZE;
  using Type = SizeType<size_type, max_size_bits>;

  /**
   * The maximum number of elements.
   */
  static constexpr size_type max =
      Type::max_count_for_element_size(element_size);

  /**
   * The maximum element index.
   */
  static constexpr size_type max_index =
      Type::max_index_for_element_size(element_size);

  /**
   * The maximum bit mask for "wrapped" index models. This is normally the same
   * as max_index, except in cases where max_index is equal to the numeric limit
   * of size_t. In that case, the size of a wrapped index model (max_index + 1)
   * cannot be represented by size_type and the max_bit_mask is the next smaller
   * power of two minus one.
   * r
   */
  static constexpr size_type
      max_bit_mask = max_index == Type::type_max
                         ? Type::max >> 1
                         : Bits<size_type>::bit_mask_not_exceeding(max_index);

  /**
   * Returns whether size is valid: non-zero and no greater than max.
   * @return true if size is valid, false otherwise.
   */
  template <typename T>
  org_nodiscard static constexpr bool is_valid(T size) noexcept {
    return Unsigned::is_nonzero_not_greater(size, max);
  }

  /**
   * Returns whether size is valid: non-zero and no greater than max.
   * @return true if size is valid, false otherwise.
   */
  template <typename source_size_type, source_size_type size_bits>
  org_nodiscard static constexpr bool is_valid(
      const Size<element_size, source_size_type, size_bits> &size) noexcept {
    return is_valid(size.get());
  }

  /**
   * Returns whether element_index is valid: non-zero and no greater than max.
   * @return true if element_index is valid, false otherwise.
   */
  template <typename T>
  org_nodiscard static constexpr bool is_valid_index(T element_index) noexcept {
    return Unsigned::is_not_greater(element_index, max_index);
  }

  /**
   * Returns whether element_index is valid: non-zero and no greater than max.
   * @return true if element_index is valid, false otherwise.
   */
  template <typename source_size_type, source_size_type size_bits>
  org_nodiscard static constexpr bool
  is_valid_index(const Size<element_size, source_size_type, size_bits>
                     &element_index) noexcept {
    return is_valid_index(element_index.get());
  }

  /**
   * Returns whether the sum of v1 and v2 represents a valid size:
   * non-zero and no greater than max.
   * @return true if the sum represents a valid size.
   */
  org_nodiscard static constexpr bool is_valid_sum(size_type v1,
                                                   size_type v2) noexcept {
    return Unsigned::is_sum_nonzero_not_greater(v1, v2, max);
  }

  /**
   * Returns whether the product of v1 and v2 represents a valid size:
   * non-zero and no greater than max.
   * @return true if the product represents a valid size.
   */
  org_nodiscard static constexpr bool is_valid_product(size_type v1,
                                                       size_type v2) noexcept {
    return Unsigned::is_product_nonzero_not_greater(v1, v2, max);
  }

  /**
   * Returns size if it is valid and throws std::invalid_argument
   * otherwise.
   * @return element_size
   * @see is_valid(size_type)
   */
  template <typename T> static constexpr size_type get_valid(T size) {
    if (is_valid(size)) {
      return size;
    }
    throw std::invalid_argument(
        "Size: size must be positive and not greater than Size::max.");
  }

  /**
   * Returns the size if it represents a valid size and throws
   * std::invalid_argument otherwise.
   * @return the value of size cast to size_type.
   */
  template <typename ST, ST size_bits>
  static constexpr size_type
  get_valid(const Size<element_size, ST, size_bits> &elementCount) {
    return get_valid(elementCount.get());
  }

  /**
   * Returns element_index if it is valid and throws std::invalid_argument
   * otherwise.
   * @return element_index
   * @see is_valid(size_type)
   */
  template <typename T>
  static constexpr size_type get_valid_index(T element_index) {
    if (is_valid_index(element_index)) {
      return element_index;
    }
    throw std::invalid_argument(
        "Size: index must not be greater than Size::max_index");
  }

  /**
   * Returns element_index if it is valid and throws std::invalid_argument
   * otherwise.
   * @return element_index
   * @see is_valid(size_type)
   */
  template <typename ST, ST size_bits>
  static constexpr size_type
  get_valid_index(const Size<element_size, ST, size_bits> &element_index) {

    return get_valid_index(element_index.get());
  }

  /**
   * Returns the sum of v1 and v2 is it is valid and throws
   * std::invalid_argument otherwise.
   * @return the sum of v1 and v2.
   * @see is_valid_sum(size_type, size_type)
   */
  static constexpr size_type get_valid_sum(size_type v1, size_type v2) {
    if (is_valid_sum(v1, v2)) {
      return v1 + v2;
    }
    throw std::invalid_argument(
        "Size: sum must be positive and not greater than Size::max.");
  }

  /**
   * Returns the product of v1 and v2 is it is valid and throws
   * std::invalid_argument otherwise.
   * @return the product of v1 and v2.
   * @see is_valid_product(size_type, size_type)
   */
  static constexpr size_type get_valid_product(size_type v1, size_type v2) {
    if (is_valid_product(v1, v2)) {
      return v1 * v2;
    }
    throw std::invalid_argument(
        "Size: product must be positive and not greater than Size::max.");
  }

  /**
   * Construct a Size based on a valid size or throws
   * std::invalid_argument otherwise.
   */
  template <typename T> explicit Size(const T size) : value(get_valid(size)) {}

  /**
   * Construct a Size based on a valid size or throws
   * std::invalid_argument otherwise.
   */
  template <typename ST, ST size_bits>
  explicit Size(const Size<element_size, ST, size_bits> &size)
      : value(get_valid(size)) {}

  /**
   * @return value
   */
  org_nodiscard explicit operator size_type() const noexcept { return value; }

  /**
   * Explicitly returns value.
   * @return value
   */
  org_nodiscard size_t get() const noexcept { return value; }

  /**
   * Returns the maximum size from an instance.
   * @return the maximum size, max.
   */
  org_nodiscard size_t maximum() const noexcept { return max; }

  /**
   * Returns the maximum element index from an instance.
   * @return the maximum element index, max_index.
   */
  org_nodiscard size_t maximum_index() const noexcept { return max_index; }

  /**
   * Returns the maximum bit mask from an instance.
   * @return the maximum bit mask, max_bit_mask.
   */
  org_nodiscard size_t maximum_bit_mask() const noexcept {
    return max_bit_mask;
  }

  /**
   * Adds other_size if that results in a valid total size and
   * throws std::invalid_argument otherwise.
   */
  Size &operator+=(size_type other_size) {
    value = get_valid_sum(value, other_size);
    return *this;
  }

  /**
   * Adds other_size if that results in a valid total size and
   * throws std::invalid_argument otherwise.
   */
  Size &operator+=(const Size &other_size) {
    value = get_valid_sum(value, other_size.value);
    return *this;
  }

  /**
   * Multiplies with other_size if that results in a valid total element
   * count and throws std::invalid_argument otherwise.
   */
  Size &operator*=(size_type other_size) {
    value = get_valid_product(value, other_size);
    return *this;
  }

  /**
   * Multiplies with other_size if that results in a valid total element
   * count and throws std::invalid_argument otherwise.
   */
  Size &operator*=(const Size &other_size) {
    value = get_valid_product(value, other_size.value);
    return *this;
  }

  /**
   * Returns a size with the sum of size and other_size if that results in a
   * valid total size and throws std::invalid_argument otherwise.
   */
  friend Size operator+(Size size, size_type other_size) {
    size += other_size;
    return size;
  }

  /**
   * Returns a size with the sum of size and other_size if that results in a
   * valid total size and throws std::invalid_argument otherwise.
   */
  friend Size operator+(Size size, const Size &other_size) {
    size += other_size;
    return size;
  }

  /**
   * Returns a size with the product of size and other_size if that results
   * in a valid total size and throws std::invalid_argument otherwise.
   */
  friend Size operator*(Size size, size_type other_size) {
    size *= other_size;
    return size;
  }

  /**
   * Returns a size with the product of size and other_size if that results
   * in a valid total size and throws std::invalid_argument otherwise.
   */
  friend Size operator*(Size size, const Size &other_size) {
    size *= other_size;
    return size;
  }

  /**
   * Assigns a new size is that is valid and throws
   * std::invalid_argument otherwise.
   */
  template <typename T> Size &operator=(T other_size) {
    value = get_valid(other_size);
    return *this;
  }

  /**
   * Explicitly assigns an size with different typing if that results
   * in a valid size and throws std::invalid_argument otherwise.
   */
  template <typename ST, ST size_bits>
  void assign(const Size<element_size, ST, size_bits> other_size) {
    operator=(other_size.get());
  }

private:
  size_type value;
};

template <typename T, typename size_type = std::size_t,
          size_type max_size_bits = 0>
using SizeFor = Size<sizeof(T), size_type, max_size_bits>;

} // namespace org::simple::core

#endif // ORG_SIMPLE_SIZE_H
