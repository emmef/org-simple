#ifndef ORG_SIMPLE_RATE_H
#define ORG_SIMPLE_RATE_H
/*
 * org-simple/dsp/rate.h
 *
 * Added by michel on 2021-04-10
 * Copyright (C) 2015-2021 Michel Fleur.
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
#include <cmath>
#include <limits>
#include <org-simple/core/number_traits.h>
#include <type_traits>

namespace org::simple::dsp {

class Rate {
public:
  static constexpr bool too_low(double v) { return v < 1e-100; }

  static constexpr bool too_high(double v) { return v > 1e+100; }

  template <typename R> static constexpr bool is_valid(R value) {
    return !(too_low(value) || too_high(value));
  }

  template <typename R> static R validated(R value) {
    if (too_low(value)) {
      throw std::invalid_argument("SampleRate: too close to zero.");
    }
    if (too_high(value)) {
      throw std::invalid_argument("SampleRate: value too high");
    }
    return value;
  }

  Rate(double rate) : rate_(validated(rate)) {}

  Rate(const Rate &source) = default;

  Rate(Rate &&source) noexcept = default;

  Rate &operator=(double rate) {
    rate_ = validated(rate);
    return *this;
  }

  operator const double() const { return rate_; }

  template <typename R> bool operator>(R other) {
    return rate_ > (double)other;
  }

  template <typename R> bool operator>=(R other) const {
    return rate_ >= (double)other;
  }

  template <typename R> bool operator==(R other) const {
    return rate_ == (double)other;
  }

  template <typename R> bool operator<(R other) const {
    return rate_ < (double)other;
  }

  template <typename R> bool operator<=(R other) const {
    return rate_ <= (double)other;
  }

  template <typename R> Rate &operator<<(R newRate) {
    rate_ = validated(newRate);
    return *this;
  }

  template <typename R> const Rate &operator>>(R &target) const {
    if (rate_ > (double)std::numeric_limits<R>::max) {
      throw std::runtime_error("SampleRate::value to big for target type.");
    }
    if constexpr (std::is_integral_v<R>) {
      if (rate_ >= 0.5) {
        target = std::round(rate_);
        return *this;
      }
    } else if (rate_ >= (double)std::numeric_limits<R>::min()) {
      target = rate_;
      return *this;
    }
    throw std::runtime_error(
        "SampleRate::value to small to represent in target as non-zero");
  }

  template <typename F>
  requires(org::simple::traits::is_complex_v<F>) F relative(F frequency)
  const { return frequency / rate_; }

  template <typename F>
  requires(!org::simple::traits::is_complex_v<F>) double relative(
      F frequency) const {
    return (double)frequency / rate_;
  }

  double absolute(double relative) const { return rate_ * relative; }

  double period() const { return 1.0 / rate_; }

  double time_to_samples(double time) {
    return absolute(time);
  }

  double time_to_samples_rounded(double time) {
    return std::round(absolute(time));
  }

private:
  double rate_;
};

} // namespace org::simple::dsp::frequency

#endif // ORG_SIMPLE_RATE_H
