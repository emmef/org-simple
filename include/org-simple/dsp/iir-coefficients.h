#ifndef ORG_SIMPLE_IIR_COEFFICIENTS_H
#define ORG_SIMPLE_IIR_COEFFICIENTS_H
/*
 * org-simple/dsp/iir-coefficients.h
 *
 * Added by michel on 2021-03-29
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

#include <cstddef>
#include <org-simple/core/Index.h>
#include <org-simple/core/denormal.h>
#include <org-simple/util/Array.h>

namespace org::simple::dsp::iir {

using namespace org::simple::core;
using namespace org::simple::util;
using namespace org::simple::denormal;

/**
 * Libraries can have different conventions for calculation and applying
 * feedback-coefficients, the coefficients applied to historic output samples.
 *
 * This library adds the historic outputs to the newly calculated output, after
 * multiplying them with the feedback-coefficients.
 *
 * If coefficients are obtained from a library that assumes subtraction of the
 * products of historic outputs and their feedback-coefficients, you MUST
 * provide FeedbackConvention#SUBTRACT when setting the coefficient or negate
 * the sign yourself.
 */
enum class FeedbackConvention { ADD, SUBTRACT };

enum class FilterType {
  ALL_PASS,
  LOW_PASS,
  LOW_SHELVE,
  BAND_PASS,
  PARAMETRIC,
  HIGH_SHELVE,
  HIGH_PASS
};

template <typename S> class CoefficientsFilter {
public:
  /**
   * Filters a single input, using the history of inputs and outputs for the
   * recursive filter. The histories will be updated with the most recent
   * values where element zero is the most recent element.
   *
   * In the histories, the most recent sample is at index 0, and the oldest
   * sample has index order - 1.
   * @tparam S The type of sample for inputs and outputs.
   * @param in_history The history of previous inputs, that must be larger than
   * the order of the filter.
   * @param out_history The history of outputs, that must be larger than the
   * order of the filter.
   * @param input The input to filter that is also inserted as first element of
   * \c in_history)
   * @return The output (filtered value), that is also inserted as first element
   * of \c out_history.
   */
  [[nodiscard]] virtual S single(S *__restrict in_history,
                                 S *__restrict out_history,
                                 const S input) const = 0;

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples, starting from an offset that equals the filter. Notice that "the
   * past" starts at elemet zero and that therefore the length of both input and
   * output arrays must be at least count + the order of the filter.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count + the filter
   * order long.
   * @param out An array of output samples that is at least count + the filter
   * order long.
   */
  virtual void forward_offs(size_t count, const S *__restrict in,
                            S *__restrict out) const = 0;

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples, starting from offset zero and all "past" inputs and outputs
   * outside of the arrays are considered zero.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count long.
   * @param out An array of output samples that is at least count long.
   */
  virtual void forward_history_zero(size_t count, const S *__restrict in,
                                    S *__restrict out) const = 0;

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples backwards in time, starting from offset count - 1. Notice that "the
   * past" starts at elemet zero and that therefore the length of both input and
   * output arrays must be at least count + the order of the filter.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count + the filter
   * order long.
   * @param out An array of output samples that is at least count + the filter
   * order long.
   */
  virtual void backward_offs(size_t count, const S *__restrict in,
                             S *__restrict out) const = 0;

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples backwards in time, starting from offset zero and all "past" inputs
   * and outputs outside of the arrays are considered zero.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count long.
   * @param out An array of output samples that is at least count long.
   */
  virtual void backward_history_zero(size_t count, const S *__restrict in,
                                     S *__restrict out) const = 0;

  [[nodiscard]] inline unsigned getOrder() const { return getValidOrder(); }
  [[nodiscard]] size_t getCoefficientCount() const {
    return getValidOrder() + 1;
  }
  [[nodiscard]] inline S getFB(size_t i) const {
    return getValidFB(Index::checked(i, getCoefficientCount()));
  }
  [[nodiscard]] inline S getFF(size_t i) const {
    return getValidFF(Index::checked(i, getCoefficientCount()));
  }

  CoefficientsFilter &setFF(size_t i, S value) {
    setValidFB(Index::checked(i, getCoefficientCount()), value);
    return *this;
  }

  CoefficientsFilter &setFB(size_t i, S value,
                            FeedbackConvention conv = FeedbackConvention::ADD) {
    setValidFF(Index::checked(i, getCoefficientCount()),
               conv == FeedbackConvention::ADD ? value : -value);
    return *this;
  }

  ~CoefficientsFilter() = default;

  friend void setAmplifyOnly(CoefficientsFilter &setter, S scale) {
    setter.setValidFF(0, scale);
    setter.setValidFB(0, 0.0);
    for (size_t i = 0; i <= setter.getOrder(); i++) {
      setter.setValidFF(i, 0.0);
      setter.setValidFB(i, 0.0);
    }
  }

protected:
  [[nodiscard]] virtual unsigned getValidOrder() const = 0;
  virtual void setValidFF(size_t i, S value) = 0;
  virtual void setValidFB(size_t i, S value) = 0;
  [[nodiscard]] virtual S getValidFF(size_t i) const = 0;
  [[nodiscard]] virtual S getValidFB(size_t i) const = 0;

private:
};

/**
 * Wraps access to coefficient implementation so that algorithms do not need to
 * care about internals nor have to dealwith polymorphism.
 * @tparam coeff Type of coefficient, that MUST be a floating-point.
 * @tparam CoefficientsClass A class that contains coefficients.
 */
template <typename coeff, unsigned FIXED_ORDER, class CoefficientsClass>
class Coefficients : public CoefficientsFilter<coeff> {
  inline const coeff &getFB_(size_t i) const {
    return static_cast<const CoefficientsClass *>(this)->getFB_(i);
  }

  inline const coeff &getFF_(size_t i) const {
    return static_cast<const CoefficientsClass *>(this)->getFF_(i);
  }

  inline coeff &getFB_(size_t i) {
    return static_cast<CoefficientsClass *>(this)->getFB_(i);
  }

  inline coeff &getFF_(size_t i) {
    return static_cast<CoefficientsClass *>(this)->getFF_(i);
  }

  inline unsigned getOrder_() const {
    return static_cast<const CoefficientsClass *>(this)->getOrder_();
  }

protected:
  [[nodiscard]] unsigned getValidOrder() const final { return getOrder_(); }
  void setValidFF(size_t i, coeff value) final { getFF_(i) = value; }
  void setValidFB(size_t i, coeff value) final { getFB_(i) = value; }
  [[nodiscard]] coeff getValidFF(size_t i) const final { return getFF_(i); }
  [[nodiscard]] coeff getValidFB(size_t i) const final { return getFB_(i); }

public:

  /**
   * Filters a single input, using the history of inputs and outputs for the
   * recursive filter. The histories will be updated with the most recent
   * values where element zero is the most recent element.
   * @tparam S The type of sample for inputs and outputs.
   * @param in_history The history of previous inputs, that must be larger than
   * the order of the filter.
   * @param out_history The history of outputs, that must be larger than the
   * order of the filter.
   * @param input The input to filter that is also inserted as first element of
   * \c in_history)
   * @return The output (filtered value), that is also inserted as first element
   * of \c out_history.
   */
  template <typename S>
  inline S filter_single(S *__restrict in_history, S *__restrict out_history,
                         const S input) const {
    size_t order;
    if constexpr (FIXED_ORDER == 0) {
      order = getOrder_();
    } else {
      order = FIXED_ORDER;
    }
    S Y = 0;
    S xN0 = input;
    S yN0 = 0.0;
    size_t i, j;
    for (i = 0, j = 1; i < order; i++, j++) {
      const S xN1 = in_history[i];
      const S yN1 = out_history[i];
      in_history[i] = xN0;
      xN0 = xN1;
      out_history[i] = Y;
      Y = yN1;
      yN0 += xN1 * getFF_(j) + yN1 * getFB_(j);
    }
    yN0 += getFF_(0) * input;

    out_history[0] = flush_to_zero(yN0);

    return yN0;
  }

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples, starting from an offset that equals the filter. Notice that "the
   * past" starts at elemet zero and that therefore the length of both input and
   * output arrays must be at least count + the order of the filter.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count + the filter
   * order long.
   * @param out An array of output samples that is at least count + the filter
   * order long.
   */
  template <typename S>
  void filter_forward_offs(size_t count, const S *__restrict in,
                           S *__restrict out) const {
    size_t order;
    if constexpr (FIXED_ORDER == 0) {
      order = getOrder_();
    } else {
      order = FIXED_ORDER;
    }
    const size_t end = count + order;
    for (size_t n = order; n < end; n++) {
      S yN = getFF_(0) * in[n];
      for (size_t j = 1; j <= order; j++) {
        yN += in[n - j] * getFF_(j) + out[n - j] * getFB_(j);
      }
      out[n] = flush_to_zero(yN);
    }
  }

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples, starting from offset zero and all "past" inputs and outputs
   * outside of the arrays are considered zero.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count long.
   * @param out An array of output samples that is at least count long.
   */
  template <typename S>
  void filter_forward_history_zero(size_t count, const S *__restrict in,
                                   S *__restrict out) const {
    size_t order;
    if constexpr (FIXED_ORDER == 0) {
      order = getOrder_();
    } else {
      order = FIXED_ORDER;
    }
    for (size_t n = 0; n < order; n++) {
      S yN = getFF_(0) * in[n];
      for (size_t j = 1; j <= order; j++) {
        if (j <= n) {
          yN += in[n - j] * getFF_(j) + out[n - j] * getFB_(j);
        }
      }
      out[n] = flush_to_zero(yN);
    }
    for (size_t n = order; n < count; n++) {
      S yN = getFF_(0) * in[n];
      for (size_t j = 1; j <= order; j++) {
        yN += in[n - j] * getFF_(j) + out[n - j] * getFB_(j);
      }
      out[n] = flush_to_zero(yN);
    }
  }

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples backwards in time, starting from offset count - 1. Notice that "the
   * past" starts at elemet zero and that therefore the length of both input and
   * output arrays must be at least count + the order of the filter.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count + the filter
   * order long.
   * @param out An array of output samples that is at least count + the filter
   * order long.
   */
  template <typename S>
  void filter_backward_offs(size_t count, const S *__restrict in,
                            S *__restrict out) const {

    size_t order;
    if constexpr (FIXED_ORDER == 0) {
      order = getOrder_();
    } else {
      order = FIXED_ORDER;
    }
    for (ptrdiff_t n = count - 1; n >= 0; n--) {
      S yN = getFF_(0) * in[n];
      for (size_t j = 1; j <= order; j++) {
        yN += in[n + j] * getFF_(j) + out[n + j] * getFB_(j);
      }
      out[n] = flush_to_zero(yN);
    }
  }

  /**
   * Filters \c count samples of an array of input samples to an array of output
   * samples backwards in time, starting from offset zero and all "past" inputs
   * and outputs outside of the arrays are considered zero.
   *
   * @tparam S The type of sample for inputs and outputs.
   * @param count The number of samples to filter.
   * @param in An array of input samples that is at least count long.
   * @param out An array of output samples that is at least count long.
   */
  template <typename S>
  void filter_backward_history_zero(size_t count, const S *__restrict in,
                                    S *__restrict out) const {
    size_t order;
    if constexpr (FIXED_ORDER == 0) {
      order = getOrder_();
    } else {
      order = FIXED_ORDER;
    }
    const ptrdiff_t start = count - 1;
    const ptrdiff_t end = count - order;
    ptrdiff_t n;
    for (n = start; n >= end && n >= 0; n--) {
      S yN = getFF_(0) * in[n];
      for (size_t j = 1; j <= order; j++) {
        ptrdiff_t i = n + j;
        if (i <= start) {
          yN += in[i] * getFF_(j) + out[i] * getFB_(j);
        }
      }
      out[n] = flush_to_zero(yN);
    }
    for (; n >= 0; n--) {
      S yN = getFF_(0) * in[n];
      for (size_t j = 1; j <= order; j++) {
        yN += in[n + j] * getFF_(j) + out[n + j] * getFB_(j);
      }
      out[n] = flush_to_zero(yN);
    }
  }

  [[nodiscard]] inline coeff single(coeff *__restrict in_history,
                                    coeff *__restrict out_history,
                                    const coeff input) const final {
    return filter_single(in_history, out_history, input);
  }

  inline void forward_offs(size_t count, const coeff *__restrict in,
                           coeff *__restrict out) const final {
    filter_forward_offs(count, in, out);
  }

  inline void forward_history_zero(size_t count, const coeff *__restrict in,
                                   coeff *__restrict out) const final {
    filter_forward_history_zero(count, in, out);
  }

  inline void backward_offs(size_t count, const coeff *__restrict in,
                            coeff *__restrict out) const final {
    filter_backward_offs(count, in, out);
  }

  inline void backward_history_zero(size_t count, const coeff *__restrict in,
                                    coeff *__restrict out) const final {
    filter_backward_history_zero(count, in, out);
  }
};

template <typename C, unsigned O, size_t A = 0>
class FixedOrderCoefficients
    : public Coefficients<C, O, FixedOrderCoefficients<C, O, A>> {

  using Parent = Coefficients<C, O, FixedOrderCoefficients<C, O, A>>;
  friend Parent;
  static_assert(O > 0, "Filter order must be positive.");
  static_assert(O <= 32, "Filter order cannot exceed 32.");
  static constexpr size_t FF = 0;
  static constexpr size_t FB = O + 1;
  Array<C, 2 * (O + 1), A> coeffs;

protected:
  inline const C &getFB_(size_t i) const { return coeffs[i + FB]; }
  inline const C &getFF_(size_t i) const { return coeffs[i + FF]; }
  inline C &getFB_(size_t i) { return coeffs[i + FB]; }
  inline C &getFF_(size_t i) { return coeffs[i + FF]; }
  inline unsigned getOrder_() const { return O; }
};

template <typename C, unsigned O, size_t A = 0>
class FixedOrderCoefficientsRef
    : public Coefficients<C, O, FixedOrderCoefficientsRef<C, O, A>> {

  using Parent = Coefficients<C, O, FixedOrderCoefficientsRef<C, O, A>>;
  friend Parent;
  static_assert(O > 0, "Filter order must be positive.");
  static_assert(O <= 32, "Filter order cannot exceed 32.");
  static constexpr size_t FF = 0;
  static constexpr size_t FB = O + 1;
  ArrayDataRefFixedSize<C, 2 * (O + 1), A> coeffs;

protected:
  inline const C &getFB_(size_t i) const { return coeffs[i + FB]; }
  inline const C &getFF_(size_t i) const { return coeffs[i + FF]; }
  inline C &getFB_(size_t i) { return coeffs[i + FB]; }
  inline C &getFF_(size_t i) { return coeffs[i + FF]; }
  inline unsigned getOrder_() const { return O; }

public:
  FixedOrderCoefficientsRef(C *const pool_location) : coeffs(pool_location) {}
};

// ArrayDataRefFixedSize

} // namespace org::simple::dsp::iir

#endif // ORG_SIMPLE_IIR_COEFFICIENTS_H
