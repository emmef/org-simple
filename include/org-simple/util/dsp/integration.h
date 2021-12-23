#ifndef ORG_SIMPLE_INTEGRATION_H
#define ORG_SIMPLE_INTEGRATION_H
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
 * y[n] = history_multiplier * y[n - 1] + input_multiplier * x[n]
 *
 * The relation between bot multipliers is
 *
 * history_multiplier = 1 - input_multiplier
 *
 * and yields an integrator with a scaling factor of exactly one.
 *
 * The history multiplier is calculated using
 *
 * history_multipler = exp( -1 / integration_sample_count)
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

struct Integration {
/**
 * The minimum sample count (can be a fraction) that can be accurately
 * represented by a pair of history and input multipliers.
 *
 * @tparam T The type of the multipliers that must represent the count.
 */
template <typename T>
requires(std::is_floating_point_v<T>) static T sample_count_min() {
  static const T min = -1.0 / log(std::numeric_limits<T>::epsilon() *
                                  integration_count_accuracy_minimum);
  return min;
}

template <typename T>
requires(std::is_floating_point_v<T>) static T sample_count_max() {
  static const T max = -1.0 / log(1.0 - std::numeric_limits<T>::epsilon() *
                                            integration_count_accuracy_minimum);
  return max;
}

/**
 * Gets the history multiplier for integrating the provided number of samples.
 * @tparam T The type of the multiplier.
 * @param count The number of samples to integrate over.
 * @return The unchecked multiplier.
 */
template <typename T>
requires(std::is_floating_point_v<T>) static constexpr T
    sample_count_to_history_multiplier_unchecked(T count) {
  return exp((T)-1.0 / count);
}

/**
 * Gets the sample count that is represented by the provided history multiplier.
 * @tparam T The type of the multiplier.
 * @param history_multiplier The history multiplier/
 * @return The number of samples to integrate over.
 */
template <typename T>
requires(std::is_floating_point_v<T>) static constexpr T
    history_multiplier_to_sample_count_unchecked(T history_multiplier) {
  return -1.0 / log(history_multiplier);
}

/**
 * Gets the history multiplier for integrating the provided number of samples.
 * If the number of samples is too low it is considered zero and a zero
 * multiplier is returned. If the number of samples is higher that the
 * accurately representable sample_count_max<T>, the multiplier for that number
 * is returned.
 * @tparam T The type of the multiplier.
 * @param count The number of samples to integrate over.
 * @return The unchecked multiplier.
 */
template <typename T>
requires(std::is_floating_point_v<T>) static constexpr T
    sample_count_to_history_multiplier_bound(T count) {
  return count <= -sample_count_min<T>()
             ? 0
             : sample_count_to_history_multiplier_unchecked(
                   std::min(count, sample_count_max<T>()));
}

/**
 * Gets the history multiplier for integrating the provided number of samples.
 * If the number of samples is too low it is considered zero and a zero
 * multiplier is returned. If the number of samples is higher that the
 * accurately representable sample_count_max<T>, a std::invalid_argument is
 * thrown.
 * @tparam T The type of the multiplier.
 * @param count The number of samples to integrate over.
 * @return The unchecked multiplier.
 */
template <typename T>
requires(std::is_floating_point_v<T>) static constexpr T
    sample_count_to_history_multiplier(T count) {
  if (count <= -sample_count_min<T>()) {
    return 0;
  }
  if (count <= sample_count_max<T>()) {
    return sample_count_to_history_multiplier_unchecked(count);
  }
  throw std::invalid_argument(
      "org::simple::dsp::integration::sample_count_to_history_multiplier: "
      "count too large to represent accurately.");
}

/**
 * Get the history multiplier from a input multiplier or vice versa.
 * @tparam T The type of the multiplier.
 * @param multiplier The multiplier to transform.
 * @return the other multiplier.
 */
template <typename T>
requires(std::is_floating_point_v<T>) static constexpr T
    multiplier_to_multiplier(T multiplier) {
  return (T)1.0 - multiplier;
}

template <typename T, typename S>
requires(std::is_floating_point_v<T> &&std::is_arithmetic_v<S>) inline static S
    get_integrated(T history_multiplier, T input_multiplier, S history,
                   S input) {
  return history_multiplier * history + input_multiplier * input;
}

template <typename T, typename S>
requires(std::is_floating_point_v<T> &&std::is_arithmetic_v<
         S>) inline static void integrate(T history_multiplier,
                                          T input_multiplier, S &history,
                                          S input) {
  history = history_multiplier * history + input_multiplier * input;
}

template <typename T, typename S>
requires(std::is_floating_point_v<T> &&std::is_arithmetic_v<S>) inline static S
    integrate_and_get(T history_multiplier, T input_multiplier, S &history,
                      S input) {
  integrate(history_multiplier, input_multiplier, history, input);
  return history;
}
};

template <typename T> class BaseCoefficients {
  static_assert(std::is_floating_point_v<T>);
  T history_multiplier_;
  T input_multiplier_;

  BaseCoefficients(T hm, T scale)
      : history_multiplier_(hm),
        input_multiplier_(scale * (Integration::multiplier_to_multiplier(hm))) {}

public:
  BaseCoefficients(const BaseCoefficients &c) = default;
  BaseCoefficients(BaseCoefficients &&c) noexcept = default;

  BaseCoefficients() : BaseCoefficients(0, 1) {}

  BaseCoefficients &operator=(const BaseCoefficients &other) = default;

  static BaseCoefficients from_count(T integration_sample_count, T scale = 1) {
    return BaseCoefficients(
        Integration::sample_count_to_history_multiplier(integration_sample_count), scale);
  }

  static BaseCoefficients from_count_bound(T integration_sample_count,
                                           T scale = 1) {
    return BaseCoefficients(
        Integration::sample_count_to_history_multiplier_bound(integration_sample_count),
        scale);
  }

  void set_count(T integration_sample_count, T scale = 1) {
    T hm = sample_count_to_history_multiplier(integration_sample_count);
    history_multiplier_ = scale * hm;
    input_multiplier_ = scale - history_multiplier_;
  }

  void set_count_bound(T integration_sample_count, T scale = 1) {
    T hm = sample_count_to_history_multiplier_bound(integration_sample_count);
    history_multiplier_ = scale * hm;
    input_multiplier_ = scale - history_multiplier_;
  }

  template <typename S>
  requires(std::is_arithmetic_v<S>) inline S
      get_integrated(S history, S input) const {
    return Integration::get_integrated(
        history_multiplier_, input_multiplier_, history, input);
  }

  template <typename S>
  requires(std::is_arithmetic_v<S>) inline void integrate(S &history,
                                                          S input) const {
    Integration::integrate<T, S>(
        history_multiplier_, input_multiplier_, history, input);
  }

  template <typename S>
  requires(std::is_arithmetic_v<S>) inline S
      integrate_and_get(S &history, S input) const {
    return Integration::integrate_and_get<T, S>(
        history_multiplier_, input_multiplier_, history, input);
  }

  T scale() const {
    return input_multiplier_ /
           Integration::multiplier_to_multiplier(
               history_multiplier_);
  }

  T integration_sample_count() const {
    return Integration::history_multiplier_to_sample_count_unchecked(history_multiplier_);
  }

  T history_multiplier() const { return history_multiplier_; }
  T input_multiplier() const {
    return Integration::multiplier_to_multiplier(
        history_multiplier_);
  }
  T input_multiplier_scaled() const { return input_multiplier_; }
};


typedef BaseCoefficients<double> Coefficients;

} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_INTEGRATION_H
