#ifndef ORG_SIMPLE_M_CIRCULAR_H
#define ORG_SIMPLE_M_CIRCULAR_H
/*
 * org-simple/Circular.h
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
#include <numeric>
#include <type_traits>

namespace org::simple {

enum class WrappingType { BIT_MASK, MODULO };

template <WrappingType wrappingType, typename size_type>
struct CircularAlgoBase {
  static_assert(std::is_integral_v<size_type> && std::is_unsigned_v<size_type>);

  static constexpr size_type elementsForMask(size_t mask) {
    if constexpr (wrappingType == WrappingType::BIT_MASK) {
      return mask  + 1;
    }
    else {
      return mask;
    }
  }
  static constexpr size_type maskForElements(size_t elements) {
    if constexpr (wrappingType == WrappingType::BIT_MASK) {
      return allocationForElements(elements) - static_cast<size_type>(1);
    }
    else {
      return elements;
    }
  }

  static constexpr size_type allocationForElements(size_t elements) {
    if constexpr (wrappingType == WrappingType::BIT_MASK) {
      constexpr size_type maxElements = std::bit_floor(std::numeric_limits<size_type>::max());
      return std::bit_ceil(std::min(elements, maxElements));
    }
    else {
      return elements;
    }
  }

  [[nodiscard]] static constexpr size_type wrapped(size_type to_wrap,
                                                   size_t mask) {
    if constexpr (wrappingType == WrappingType::BIT_MASK) {
      return to_wrap & mask;
    } else {
      return to_wrap % mask;
    }
  }

  [[nodiscard]] static constexpr size_type unsafe_inc(size_type index,
                                                      size_t mask) {
    return wrapped(index + 1, mask);
  }

  [[nodiscard]] static constexpr size_type inc(size_type index, size_t mask) {
    return unsafe_inc(index, mask);
  }

  [[nodiscard]] static constexpr size_type unsafe_dec(size_type index,
                                                      size_t mask) {
    return wrapped(index - 1, mask);
  }

  [[nodiscard]] static constexpr size_type dec(size_type index, size_t mask) {
    return unsafe_dec(index, mask);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_add(size_type index, size_type delta, size_t mask) {
    return wrapped(index + delta, mask);
  }

  [[nodiscard]] static constexpr size_type add(size_type index, size_type delta,
                                               size_t mask) {
    return unsafe_add(index, delta, mask);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_sub(size_type index, size_type delta, size_t mask) {
    return wrapped(index - delta, mask);
  }

  [[nodiscard]] static constexpr size_type sub(size_type index, size_type delta,
                                               size_t mask) {
    return unsafe_sub(index, delta, mask);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_diff(size_type hi, size_type lo, size_t mask) {
    if constexpr (wrappingType == WrappingType::BIT_MASK) {
      return (hi > lo ? hi : hi + mask + 1) - lo;
    }
    else {
      return (hi > lo ? hi : hi + mask) - lo;
    }
  }

  [[nodiscard]] static constexpr size_type diff(size_type hi, size_type lo,
                                                size_t mask) {
    return unsafe_diff(hi, lo, mask);
  }

};


template <typename size_type = size_t>
struct CircularBase {

  template <WrappingType wrappingType> struct Metric {
    typedef CircularAlgoBase<wrappingType, size_type> circular;

    explicit Metric(size_t elements) : mask_(circular::maskForElements(elements)) {}

    size_type elements() const { return circular::elementsForMask(mask_); }

    [[nodiscard]] size_type wrapped(size_type to_wrap) const {
      return circular::wrapped(to_wrap, mask_);
    }

    [[nodiscard]] size_type unsafe_inc(size_type index) const {
      return circular::unsafe_inc(index, mask_);
    }

    [[nodiscard]] size_type inc(size_type index) const {
      return circular::inc(index, mask_);
    }

    [[nodiscard]] size_type unsafe_dec(size_type index) const {
      return circular::unsafe_dec(index, mask_);
    }

    [[nodiscard]] size_type dec(size_type index) const {
      return circular::dec(index, mask_);
    }

    [[nodiscard]] size_type unsafe_add(size_type index, size_type delta) const {
      return circular::unsafe_add(index, delta, mask_);
    }

    [[nodiscard]] size_type add(size_type index, size_type delta) const {
      return circular::add(index, delta, mask_);
    }

    [[nodiscard]] size_type unsafe_sub(size_type index, size_type delta) const {
      return circular::unsafe_sub(index, delta, mask_);
    }

    [[nodiscard]] size_type sub(size_type index, size_type delta) const {
      return circular::sub(index, delta, mask_);
    }

    [[nodiscard]] size_type unsafe_diff(size_type hi, size_type lo) const {
      return circular::unsafe_diff(hi, lo, mask_);
    }

    [[nodiscard]] size_type diff(size_type hi, size_type lo) const {
      return circular::diff(hi, lo, mask_);
    }

    size_t set_elements(size_t element_count) {
      mask_ = circular::maskForElements(element_count);
      return elements();
    }

  private:
    size_type mask_;
  };

  template <WrappingType wrappingType, size_type ELEMENTS> struct FixedMetric {
    typedef CircularAlgoBase<wrappingType, size_type> circular;
    static_assert(circular::elementsForMask(circular::maskForElements(ELEMENTS)) >= ELEMENTS);

    static constexpr size_type MASK = circular::maskForElements(ELEMENTS);

    static constexpr size_type elements() { return circular::elementsForMask(MASK); }

    [[nodiscard]] static constexpr size_type wrapped(size_type to_wrap) {
      return circular::wrapped(to_wrap, MASK);
    }

    [[nodiscard]] static constexpr size_type unsafe_inc(size_type index) {
      return circular::unsafe_inc(index, MASK);
    }

    [[nodiscard]] static constexpr size_type inc(size_type index) {
      return circular::inc(index, MASK);
    }

    [[nodiscard]] static constexpr size_type unsafe_dec(size_type index) {
      return circular::unsafe_dec(index, MASK);
    }

    [[nodiscard]] static constexpr size_type dec(size_type index) {
      return circular::dec(index, MASK);
    }

    [[nodiscard]] static constexpr size_type unsafe_add(size_type index,
                                                        size_type delta) {
      return circular::unsafe_add(index, delta, MASK);
    }

    [[nodiscard]] static constexpr size_type add(size_type index,
                                                 size_type delta) {
      return circular::add(index, delta, MASK);
    }

    [[nodiscard]] static constexpr size_type unsafe_sub(size_type index,
                                                        size_type delta) {
      return circular::unsafe_sub(index, delta, MASK);
    }

    [[nodiscard]] static constexpr size_type sub(size_type index,
                                                 size_type delta) {
      return circular::sub(index, delta, MASK);
    }

    [[nodiscard]] static constexpr size_type unsafe_diff(size_type hi,
                                                         size_type lo) {
      return circular::unsafe_diff(hi, lo, MASK);
    }

    [[nodiscard]] static constexpr size_type diff(size_type hi, size_type lo) {
      return circular::diff(hi, lo, MASK);
    }
  };

};

typedef CircularAlgoBase<WrappingType::BIT_MASK, size_t> CircularMasked;
typedef CircularAlgoBase<WrappingType::MODULO, size_t> CircularModulo;
typedef CircularBase<size_t> Circular;

} // namespace org::simple

#endif // ORG_SIMPLE_M_CIRCULAR_H
