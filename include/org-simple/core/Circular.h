#ifndef ORG_SIMPLE_CIRCULAR_H
#define ORG_SIMPLE_CIRCULAR_H
/*
 * org-simple/circular.h
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

#include <limits>
#include <org-simple/core/Size.h>
#include <type_traits>

namespace org::simple::core {

enum class WrappingType { BIT_MASK, MODULO };

namespace base {

template <WrappingType wrappingType, size_t element_size = 1,
    typename size_type = size_t, int max_size_bits = 0>
struct WrappedBase;

template <size_t element_size, typename size_type, int max_size_bits>
struct WrappedBase<WrappingType::BIT_MASK, element_size, size_type,
    max_size_bits> {

  static constexpr size_type max_element_count =
      Size<element_size, size_type, max_size_bits>::max;

  org_nodiscard static bool is_valid_element_count(size_type elements) {
    return elements > 0 && elements <= max_element_count;
  }

  explicit WrappedBase(size_type elements) : mask_(valid_mask(elements)) {}

  size_type size() const noexcept { return mask_ + 1; }

  org_nodiscard inline size_type wrapped(size_type to_wrap) const noexcept {
    return to_wrap & mask_;
  }

  org_nodiscard inline size_type unsafe_inc(size_type index) const noexcept {
    return wrapped(index + 1);
  }

  org_nodiscard inline size_type unsafe_dec(size_type index) const noexcept {
    return wrapped(index - 1);
  }

  org_nodiscard size_type unsafe_add(size_type index,
  size_type delta) const noexcept {
    return wrapped(index + delta);
  }

  org_nodiscard size_type unsafe_sub(size_type index,
  size_type delta) const noexcept {
    return wrapped(index + mask_ + 1 - delta & mask_);
  }

  void set_element_count(size_type elements) { mask_ = valid_mask(elements); }

protected:
  org_nodiscard inline size_type
  safe_parameter(size_type parameter) const noexcept {
    return parameter;
  }

  org_nodiscard static size_type
  allocation_for_valid_elements(size_type elements) noexcept {
    return Bits<size_type>::bit_mask_including(maximum(2, elements) - 1) +
           size_type(1);
  }

private:
  static size_type valid_mask(size_t elements) {
    if (is_valid_element_count(elements)) {
      return Bits<size_type>::bit_mask_including(elements - 1);
    }
    throw std::invalid_argument(
        "WrappedIndex(BIT_MASK): invalid number of elements.");
  }
  size_type mask_;
};

template <size_t element_size, typename size_type, int max_size_bits>
struct WrappedBase<WrappingType::MODULO, element_size, size_type,
    max_size_bits> {

  static constexpr size_type max_element_count = Size<element_size, size_type, max_size_bits>::max_index / 2;

  org_nodiscard static bool is_valid_element_count(size_type elements) {
    return elements > 0 && elements <= max_element_count;
  }

  explicit WrappedBase(size_type elements) : size_(valid_element_count(elements)) {}

  size_type size() const noexcept { return size_; }

  org_nodiscard inline size_type wrapped(size_type to_wrap) const noexcept {
    return to_wrap % size_;
  }

  org_nodiscard size_type unsafe_inc(size_type index) const noexcept {
    return wrapped(index + 1);
  }

  org_nodiscard size_type unsafe_dec(size_type index) const noexcept {
    return wrapped(size_ + index - 1);
  }

  org_nodiscard size_type unsafe_add(size_type index,
  size_type delta) const noexcept {
    return wrapped(index + delta);
  }

  org_nodiscard size_type unsafe_sub(size_type index,
  size_type delta) const noexcept {
    return wrapped(index + size_ - delta);
  }

  void set_element_count(size_type elements) {
    size_ = valid_element_count(elements);
  }

protected:
  org_nodiscard inline size_type
  safe_parameter(size_type parameter) const noexcept {
    return parameter % size_;
  }

  org_nodiscard static size_type
  allocation_for_valid_elements(size_type elements) noexcept {
    return elements;
  }

private:
  size_type size_;

  org_nodiscard static size_t valid_element_count(size_type elements) {
    if (is_valid_element_count(elements)) {
      return elements;
    }
    throw std::invalid_argument(
        "WrappedIndex(MODULO): number of elements must be non-zero and not "
        "greater than WrappedIndex::max_element_count.");
  }
};

} // namespace base

/**
 * Specifies a circular indexing model to address elements of size element_size
 * with indices of size_type and an optional limited number of addressing bits
 * max_size_bits.
 *
 * The model can be applied in circular buffers and the like.
 * @see core::simple::Size<element_size, size_type, max_size_bits>.
 */
template <WrappingType wrappingType, size_t element_size = 1,
    typename size_type = size_t,
    int max_size_bits = sizeof(size_type) * 8>
struct WrappedIndex : public base::WrappedBase<wrappingType, element_size,
    size_type, max_size_bits> {

  using Super =
  base::WrappedBase<wrappingType, element_size, size_type, max_size_bits>;

  using Size = Size<element_size, size_type, max_size_bits>;

  static constexpr size_type max_element_count = Super::max_element_count;

  /**
   * @return true if the minimum element count can be represented by this
   * wrapped index model type and false otherwise.
   */
  org_nodiscard static bool
  is_valid_element_count(size_type elements) noexcept {
    return Super::is_valid_element_count(elements);
  }

  org_nodiscard static size_type
  get_allocation_size_for(size_type elements) noexcept {
    return is_valid_element_count(elements)
           ? Super::allocation_for_valid_elements((elements))
           : 0;
  }

  /**
   * Creates a circular indexing model that can address at least element_count
   * elements; or throws std::invalid_argument if the element_count is less than
   * two or too big to be represented because of size constraints.
   * @param element_count the minimum number of elements that should be
   * addressable.
   * @return a proper circular mask or zero if that is not possible.
   */
  explicit WrappedIndex(size_t element_count) : Super(element_count) {}

  org_nodiscard inline size_type inc(size_type index) const noexcept {
    return Super::unsafe_inc(Super::safe_parameter(index));
  }

  org_nodiscard inline size_type dec(size_type index) const noexcept {
    return Super::unsafe_dec(Super::safe_parameter(index));
  }

  org_nodiscard size_type add(size_type index,
  size_type delta) const noexcept {
    return Super::unsafe_add(Super::safe_parameter(index),
                             Super::safe_parameter(delta));
  }

  org_nodiscard size_type sub(size_type index,
  size_type delta) const noexcept {
    return Super::unsafe_sub(Super::safe_parameter(index),
                             Super::safe_parameter(delta));
  }

// Clang problem
//  /**
//   * Sets a new element count if that is valid and throws std::invalid_argument
//   * otherwise. The actual possible number of elements can be bigger and is
//   * returned by size().
//   */
//  void set_element_count(size_type element_count) {
//    Super::set_element_count(element_count);
//  }
};

template <WrappingType wrappingType, typename Element,
    typename size_type = size_t,
    int max_size_bits = sizeof(size_type) * 8>
using WrappedIndexFor =
WrappedIndex<wrappingType, sizeof(Element), size_type, max_size_bits>;

/**
 * A bit-mask-based wrapped indexing model for the specified element type.
 */
template <typename Element, typename size_type = size_t,
    int max_size_bits = sizeof(size_type) * 8>
using MaskedIndexFor = WrappedIndex<WrappingType::BIT_MASK, sizeof(Element),
    size_type, max_size_bits>;

/**
 * A modulo-based wrapped indexing model for the specified element type.
 */
template <typename Element, typename size_type = size_t,
    int max_size_bits = sizeof(size_type) * 8>
using ModuloIndexFor = WrappedIndex<WrappingType::MODULO, sizeof(Element),
    size_type, max_size_bits>;


} // namespace org::simple::core

#endif // ORG_SIMPLE_CIRCULAR_H
