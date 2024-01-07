#ifndef ORG_SIMPLE_ALIGN_H
#define ORG_SIMPLE_ALIGN_H
/*
 * org-simple/Alignment.h
 *
 * Added by michel on 2024-01-04
 * Copyright (C) 2015-2024 Michel Fleur.
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
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace org::simple {
struct Align {
  static constexpr size_t max =
      std::bit_floor(std::numeric_limits<unsigned short>::max()) >> 1;

  static constexpr size_t maxNatural = alignof(std::max_align_t);

  template <typename T> static constexpr size_t maxElements() {
    return max / sizeof(T);
  }

  template <typename T> static constexpr size_t fixWithMaxNatural(const size_t alignment) {
    const size_t inherent = std::max(maxNatural, alignof(T));
    return alignment <= inherent ? inherent
           : alignment < max      ? std::bit_ceil(alignment)
                                  : std::bit_floor(alignment);
  }

  /**
   * Returns whether the alignment equal to the natural alignment or a greater
   * power of two.
   */
  template <class type>
  static constexpr bool isValid(const size_t alignment) noexcept {
    return alignment >= alignof(type) && alignment <= max &&
           std::has_single_bit(alignment);
  }

  template <class type>
  static constexpr size_t validOrDefault(const size_t alignment) noexcept {
    return alignment && isValid<type>(alignment) ? alignment : alignof(type);
  }

  template <class type> static constexpr size_t fixed(const size_t alignment) {
    return alignment <= alignof(type) ? alignof(type)
           : alignment <= max         ? std::bit_ceil(alignment)
                                      : std::bit_floor(alignment);
  }

  template <class type = char>
  static constexpr bool isAlignedValue(const size_t value,
                                       size_t alignment) noexcept {
    return (value % validOrDefault<type>(alignment)) == 0;
  }

  template <class type>
  static constexpr bool isAlignedPointer(const type *const p,
                                         size_t alignment = 0) noexcept {
    return isAlignedValue<type>(reinterpret_cast<uintptr_t>(p), alignment);
  }
};

template <class T, size_t ALIGNMENT = alignof(T)> struct AlignedType {
  static_assert(Align::isValid<T>(ALIGNMENT));

  typedef T type;
  static constexpr size_t alignment = ALIGNMENT;
  [[maybe_unused]] static constexpr size_t maxElements =
      static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max()) / sizeof(T);

  static constexpr size_t alignedElements() noexcept {
    return std::max(static_cast<size_t>(1), alignment / sizeof(type));
  }

  static constexpr size_t isConsecutive() noexcept {
    return alignment % sizeof(type) == 0;
  }

  static constexpr size_t isConsecutive(size_t count) noexcept {
    return isConsecutive() && count % alignedElements() == 0;
  }

  static constexpr size_t alignedElementsCeiling(size_t count) noexcept {
    const auto elements = alignedElements();
    if (elements == 1) {
      return count;
    }
    const auto test = count / alignment * alignment;
    return test == count ? count : test + alignment;
  }

  static constexpr bool isAlignedValue(const size_t value) noexcept {
    return Align::isAlignedValue(value, alignment);
  }

  static constexpr bool isAlignedPointer(const T *const p) noexcept {
    return Align::isAlignedPointer(p);
  }
};

} // namespace org::simple

#endif // ORG_SIMPLE_ALIGN_H
