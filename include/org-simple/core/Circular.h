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

template <WrappingType wrappingType, typename size_type = size_t,
          size_type size_limit = std::numeric_limits<size_type>::max()>
struct CircularAlgoBase;

template <typename size_type, size_type size_limit>
struct CircularAlgoBase<WrappingType::BIT_MASK, size_type, size_limit> {
  typedef SizeValueBase<size_type, size_limit> size_metric;

  static constexpr size_type elements(size_t mask) noexcept { return mask + 1; }

  [[nodiscard]] static constexpr size_t value_for_elements(size_type elements) {
    return size_metric::Valid::mask_for_elements(elements);
  }

  static constexpr size_type elements_for_allocation(size_t elements) noexcept {
    return CircularAlgoBase<WrappingType::BIT_MASK, size_type,
                            size_limit>::elements(value_for_elements(elements));
  }

  [[nodiscard]] static constexpr bool is_valid_elements(size_type elements) {
    return size_metric::IsValid::value(elements);
  }

  [[nodiscard]] static constexpr size_type wrapped(size_type to_wrap,
                                                   size_t mask) noexcept {
    return to_wrap & mask;
  }

  [[nodiscard]] static constexpr size_type unsafe_inc(size_type index,
                                                      size_t mask) noexcept {
    return wrapped(index + 1, mask);
  }

  [[nodiscard]] static constexpr size_type inc(size_type index,
                                               size_t mask) noexcept {
    return unsafe_inc(index, mask);
  }

  [[nodiscard]] static constexpr size_type unsafe_dec(size_type index,
                                                      size_t mask) noexcept {
    return wrapped(index - 1, mask);
  }

  [[nodiscard]] static constexpr size_type dec(size_type index,
                                               size_t mask) noexcept {
    return unsafe_dec(index, mask);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_add(size_type index, size_type delta, size_t mask) noexcept {
    return wrapped(index + delta, mask);
  }

  [[nodiscard]] static constexpr size_type add(size_type index, size_type delta,
                                               size_t mask) noexcept {
    return unsafe_add(index, delta, mask);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_sub(size_type index, size_type delta, size_t mask) noexcept {
    return wrapped(index - delta, mask);
  }

  [[nodiscard]] static constexpr size_type sub(size_type index, size_type delta,
                                               size_t mask) noexcept {
    return unsafe_sub(index, delta, mask);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_diff(size_type hi, size_type lo, size_t mask) noexcept {
    return (hi > lo ? hi : hi + mask + 1) - lo;
  }

  [[nodiscard]] static constexpr size_type diff(size_type hi, size_type lo,
                                                size_t mask) noexcept {
    return unsafe_diff(hi, lo, mask);
  }
};

template <typename size_type, size_type size_limit>
struct CircularAlgoBase<WrappingType::MODULO, size_type, size_limit> {

  typedef SizeValueBase<size_type, size_limit> size_metric;

  static constexpr size_type elements(size_t size) noexcept { return size; }

  [[nodiscard]] static constexpr size_t value_for_elements(size_type elements) {
    return size_metric::Valid::value(elements);
  }

  static constexpr size_type elements_for_allocation(size_t elements) noexcept {
    return CircularAlgoBase<WrappingType::MODULO, size_type,
                            size_limit>::elements(value_for_elements(elements));
  }

  [[nodiscard]] static constexpr bool is_valid_elements(size_type elements) {
    return size_metric::IsValid::value(elements);
  }

  [[nodiscard]] static constexpr size_type wrapped(size_type to_wrap,
                                                   size_t size) noexcept {
    return to_wrap % size;
  }

  [[nodiscard]] static constexpr size_type unsafe_inc(size_type index,
                                                      size_t size) noexcept {
    return wrapped(index + 1, size);
  }

  [[nodiscard]] static constexpr size_type inc(size_type index,
                                               size_t size) noexcept {
    return unsafe_inc(wrapped(index, size), size);
  }

  [[nodiscard]] static constexpr size_type unsafe_dec(size_type index,
                                                      size_t size) noexcept {
    return wrapped(size + index - 1, size);
  }

  [[nodiscard]] static constexpr size_type dec(size_type index,
                                               size_t size) noexcept {
    return unsafe_dec(wrapped(index, size), size);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_add(size_type index, size_type delta, size_t size) noexcept {
    return wrapped(index + delta, size);
  }

  [[nodiscard]] static constexpr size_type add(size_type index, size_type delta,
                                               size_t size) noexcept {
    return unsafe_add(wrapped(index, size), wrapped(delta, size), size);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_sub(size_type index, size_type delta, size_t size) noexcept {
    return wrapped(index + size - delta, size);
  }

  [[nodiscard]] static constexpr size_type sub(size_type index, size_type delta,
                                               size_t size) noexcept {
    return unsafe_sub(wrapped(index, size), wrapped(delta, size), size);
  }

  [[nodiscard]] static constexpr size_type
  unsafe_diff(size_type hi, size_type lo, size_t size) noexcept {
    return (hi > lo ? hi : hi + size) - lo;
  }

  [[nodiscard]] static constexpr size_type diff(size_type hi, size_type lo,
                                                size_t size) noexcept {
    return unsafe_diff(wrapped(hi, size), wrapped(lo, size), size);
  }
};

template <typename size_type = size_t,
          size_type size_limit = std::numeric_limits<size_type>::max()>
struct CircularBase {

  template <WrappingType wrappingType> struct Metric {
    typedef CircularAlgoBase<wrappingType, size_type, size_limit> circular;

    [[nodiscard]] static constexpr size_t
    value_for_elements(size_type elements) {
      return circular::value_for_elements(elements);
    }

    explicit Metric(size_t elements) : value_(value_for_elements(elements)) {}

    size_type elements() const noexcept { return circular::elements(value_); }

    [[nodiscard]] size_type wrapped(size_type to_wrap) const noexcept {
      return circular::wrapped(to_wrap, value_);
    }

    [[nodiscard]] size_type unsafe_inc(size_type index) const noexcept {
      return circular::unsafe_inc(index, value_);
    }

    [[nodiscard]] size_type inc(size_type index) const noexcept {
      return circular::inc(index, value_);
    }

    [[nodiscard]] size_type unsafe_dec(size_type index) const noexcept {
      return circular::unsafe_dec(index, value_);
    }

    [[nodiscard]] size_type dec(size_type index) const noexcept {
      return circular::dec(index, value_);
    }

    [[nodiscard]] size_type unsafe_add(size_type index,
                                       size_type delta) const noexcept {
      return circular::unsafe_add(index, delta, value_);
    }

    [[nodiscard]] size_type add(size_type index,
                                size_type delta) const noexcept {
      return circular::add(index, delta, value_);
    }

    [[nodiscard]] size_type unsafe_sub(size_type index,
                                       size_type delta) const noexcept {
      return circular::unsafe_sub(index, delta, value_);
    }

    [[nodiscard]] size_type sub(size_type index,
                                size_type delta) const noexcept {
      return circular::sub(index, delta, value_);
    }

    [[nodiscard]] size_type unsafe_diff(size_type hi,
                                        size_type lo) const noexcept {
      return circular::unsafe_diff(hi, lo, value_);
    }

    [[nodiscard]] size_type diff(size_type hi, size_type lo) const noexcept {
      return circular::diff(hi, lo, value_);
    }

    size_t set_elements(size_t element_count) {
      value_ = value_for_elements(element_count);
      return elements();
    }

  private:
    size_type value_;
  };

  template <WrappingType wrappingType, size_type ELEMENTS> struct FixedMetric {
    typedef CircularAlgoBase<wrappingType, size_type,
                             std::numeric_limits<size_type>::max()>
        circular;

    static_assert(circular::is_valid_elements(ELEMENTS));
    static constexpr size_type VALUE = circular::value_for_elements(ELEMENTS);

    static constexpr size_type elements() noexcept {
      return circular::elements(VALUE);
    }

    [[nodiscard]] static constexpr size_type
    wrapped(size_type to_wrap) noexcept {
      return circular::wrapped(to_wrap, VALUE);
    }

    [[nodiscard]] static constexpr size_type
    unsafe_inc(size_type index) noexcept {
      return circular::unsafe_inc(index, VALUE);
    }

    [[nodiscard]] static constexpr size_type inc(size_type index) noexcept {
      return circular::inc(index, VALUE);
    }

    [[nodiscard]] static constexpr size_type
    unsafe_dec(size_type index) noexcept {
      return circular::unsafe_dec(index, VALUE);
    }

    [[nodiscard]] static constexpr size_type dec(size_type index) noexcept {
      return circular::dec(index, VALUE);
    }

    [[nodiscard]] static constexpr size_type
    unsafe_add(size_type index, size_type delta) noexcept {
      return circular::unsafe_add(index, delta, VALUE);
    }

    [[nodiscard]] static constexpr size_type add(size_type index,
                                                 size_type delta) noexcept {
      return circular::add(index, delta, VALUE);
    }

    [[nodiscard]] static constexpr size_type
    unsafe_sub(size_type index, size_type delta) noexcept {
      return circular::unsafe_sub(index, delta, VALUE);
    }

    [[nodiscard]] static constexpr size_type sub(size_type index,
                                                 size_type delta) noexcept {
      return circular::sub(index, delta, VALUE);
    }

    [[nodiscard]] static constexpr size_type
    unsafe_diff(size_type hi, size_type lo) noexcept {
      return circular::unsafe_diff(hi, lo, VALUE);
    }

    [[nodiscard]] static constexpr size_type diff(size_type hi,
                                                  size_type lo) noexcept {
      return circular::diff(hi, lo, VALUE);
    }
  };

  template <WrappingType wrappingType, size_type ELEMENTS>
  struct FixedMetricInstance {
    typedef CircularAlgoBase<wrappingType, size_type,
                             std::numeric_limits<size_type>::max()>
        circular;

    static_assert(circular::is_valid_elements(ELEMENTS));
    static constexpr size_type value_ = circular::value_for_elements(ELEMENTS);

    size_type elements() const noexcept { return circular::elements(value_); }

    [[nodiscard]] size_type wrapped(size_type to_wrap) const noexcept {
      return circular::wrapped(to_wrap, value_);
    }

    [[nodiscard]] size_type unsafe_inc(size_type index) const noexcept {
      return circular::unsafe_inc(index, value_);
    }

    [[nodiscard]] size_type inc(size_type index) const noexcept {
      return circular::inc(index, value_);
    }

    [[nodiscard]] size_type unsafe_dec(size_type index) const noexcept {
      return circular::unsafe_dec(index, value_);
    }

    [[nodiscard]] size_type dec(size_type index) const noexcept {
      return circular::dec(index, value_);
    }

    [[nodiscard]] size_type unsafe_add(size_type index,
                                       size_type delta) const noexcept {
      return circular::unsafe_add(index, delta, value_);
    }

    [[nodiscard]] size_type add(size_type index,
                                size_type delta) const noexcept {
      return circular::add(index, delta, value_);
    }

    [[nodiscard]] size_type unsafe_sub(size_type index,
                                       size_type delta) const noexcept {
      return circular::unsafe_sub(index, delta, value_);
    }

    [[nodiscard]] size_type sub(size_type index,
                                size_type delta) const noexcept {
      return circular::sub(index, delta, value_);
    }

    [[nodiscard]] size_type unsafe_diff(size_type hi,
                                        size_type lo) const noexcept {
      return circular::unsafe_diff(hi, lo, value_);
    }

    [[nodiscard]] size_type diff(size_type hi, size_type lo) const noexcept {
      return circular::diff(hi, lo, value_);
    }
  };
};

template <WrappingType type>
using CircularAlgo = CircularAlgoBase<type, size_t>;
typedef CircularAlgoBase<WrappingType::BIT_MASK, size_t> CircularMasked;
typedef CircularAlgoBase<WrappingType::MODULO, size_t> CircularModulo;
typedef CircularBase<size_t> Circular;

} // namespace org::simple::core

#endif // ORG_SIMPLE_CIRCULAR_H
