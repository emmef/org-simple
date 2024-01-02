#ifndef ORG_SIMPLE_UTIL_DSP_M_INTEGRATION_H
#define ORG_SIMPLE_UTIL_DSP_M_INTEGRATION_H
/*
 * org-simple/util/dsp/integration.h
 *
 * Added by michel on 2021-04-12
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
#include <type_traits>

/**
 * Basic integration, that is defined by the recursive expression:
 *
 * y[n] = historyMultiplier * y[n - 1] + inputMultiplier * x[n]
 *
 * The relation between bot multipliers is
 *
 * historyMultiplier = 1 - inputMultiplier
 *
 * and yields an integrator with a scaling factor of exactly one.
 *
 * The history multiplier is calculated using
 *
 * history_multipler = exp( -1 / samples)
 */
namespace org::simple::util::dsp {

/**
 * The minimum accuracy in "one out of N" that must be met by generated
 * integration multipliers. Admitted, this is only relevant for very large
 * counts that will lead to history multipliers very close to one.
 */
#ifndef ORG_SIMPLE_DSP_INTEGRATION_COUNT_ACCURACY_OVERRIDE
static constexpr double integration_count_accuracy_minimum = 10;
#else
static constexpr double integration_count_accuracy_minimum =
    ORG_SIMPLE_DSP_INTEGRATION_COUNT_ACCURACY_OVERRIDE;
#endif

/**
 * Implements a classic RC-filter using an recursive filter with one input and
 * one feedback coefficient.
 * The impulse response of the filter is a decaying exponent that will reach
 * \em 1/e of its original value after the number of "characteristic" samples.
 * Characteristic samples larger than about 400M may yield inaccurate results
 * due to repeated calculations and floating point precision.
 */
template <class T> class IntegratorCoefficients {
  static_assert(std::is_floating_point_v<T>);
  /*
   * The constructed defaults constitute an integrator that has zero
   * characteristic samples, meaning it just echoes the input.
   */
  T historyMultiplier_ = 0;
  T inputMultiplier_ = 1;

public:
  /**
   * The minimum sample count (can be a fraction) that can be accurately
   * represented by a pair of history and input multipliers.
   *
   * @tparam T The type of the multipliers that must represent the count.
   */
  static T minimumCharacteristicSamples() {
    static const T min = -1.0 / log(std::numeric_limits<T>::epsilon() *
                                    integration_count_accuracy_minimum);
    return min;
  }

  static T maximumCharacteristicSamples() {
    static const T max =
        -1.0 / log(1.0 - std::numeric_limits<T>::epsilon() *
                             integration_count_accuracy_minimum);
    return max;
  }

  void setSamplesAndScale(const double samples, const double scale = 1.0) {
    const double count = fabs(samples);
    historyMultiplier_ =
        (count < minimumCharacteristicSamples())
            ? static_cast<T>(0.0)
            : exp(-1.0 / std::min(count, maximumCharacteristicSamples()));
    setScale(scale);
  }

  void setSecondsForRateAndScale(const double seconds, const int rate,
                                 const double scale = 1.0) {
    setSamplesAndScale(seconds * static_cast<double>(rate), scale);
  }

  [[maybe_unused]] void setScale(const double scale) {
    inputMultiplier_ = static_cast<T>(fabs(scale) * (1.0 - historyMultiplier_));
  }

  void integrate(T &history, const T input) const {
    history = historyMultiplier_ * history + inputMultiplier_ * input;
  }

  T integrateAndGet(T &history, const T input) const {
    history = historyMultiplier_ * history + inputMultiplier_ * input;
    return history;
  }

  T getIntegrated(const T history, const T input) const {
    return historyMultiplier_ * history + inputMultiplier_ * input;
  }

  [[nodiscard]] [[maybe_unused]] T scale() const {
    return inputMultiplier_ / (static_cast<T>(1.0) - historyMultiplier_);
  }

  T historyMultiplier() const { return historyMultiplier_; }
  T inputMultiplier() const { return inputMultiplier_; }

  [[nodiscard]] [[maybe_unused]] T samples() const {
    return static_cast<T>(-1.0 / log(historyMultiplier_));
  }

  static IntegratorCoefficients fromCount(const double samples,
                                          const double scale = 1.0) {
    IntegratorCoefficients c;
    c.setSamplesAndScale(samples, scale);
    return c;
  }

  IntegratorCoefficients withScale(const double scale) const {
    IntegratorCoefficients c = *this;
    c.setScale(scale);
    return c;
  }
};

template <typename T> class Integrator : public IntegratorCoefficients<T> {
  T integrated_ = 0;

public:
  T integrate(const T input) {
    IntegratorCoefficients<T>::integrate(integrated_, input);
    return integrated_;
  }

  template <typename V> T integrate(const V input) {
    return static_cast<V>(integrate(static_cast<T>(input)));
  }

  void setOutput(const T new_value) { integrated_ = new_value; }
};

/**
 * Implements an envelope function that follows rises in input - attack -
 * immediately and on input that is lowe than the current output, optionally the
 * value for a number of samples and then smoothly integrates towards the lower
 * input value using a double integrator.
 */
template <typename T> class FastAttackSmoothRelease {
  IntegratorCoefficients<T> coefficients_;
  T integrated_1 = 0;
  T integrated_2 = 0;
  size_t hold_samples_ = 0;
  size_t hold_count_ = 0;

public:
  /**
   * Applying double integration, to be really smooth, effectively increases
   * the number of characteristic samples in the impulse response of the
   * output. This factor compensates for that.
   */
  static constexpr double CHARACTERISTIC_SAMPLE_CORRECTION = 0.465941272863;

  void setSamplesAndHoldSamples(const double samples, int hold_samples) {
    coefficients_.setSamples(samples * CHARACTERISTIC_SAMPLE_CORRECTION, 1.0);
    hold_samples_ = std::max(0, hold_samples);
  }

  void setSecondsForRateAndHoldSamples(const double seconds, const int rate,
                                       int hold_samples) {
    coefficients_.setSecondsForRate(seconds * CHARACTERISTIC_SAMPLE_CORRECTION,
                                    rate, 1.0);
    hold_samples_ = std::max(0, hold_samples);
  }

  [[nodiscard]] [[maybe_unused]] const IntegratorCoefficients<T> &
  coefficients() const {
    return coefficients_;
  }

  void setOutput(const T output) {
    integrated_2 = output;
    integrated_1 = output;
    hold_count_ = hold_samples_;
  }

  T getEnvelope(T signal) {
    if (signal > integrated_2) {
      setOutput(signal);
    } else if (hold_count_) {
      hold_count_--;
    } else {
      coefficients_.integrate(integrated_1, signal);
      coefficients_.integrate(integrated_2, integrated_1);
    }
    return integrated_2;
  }

  template <typename V> V get_envelope(const V signal) {
    return static_cast<V>(get_envelope(static_cast<T>(signal)));
  }
};

typedef IntegratorCoefficients<double> Coefficients;

} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_UTIL_DSP_M_INTEGRATION_H
