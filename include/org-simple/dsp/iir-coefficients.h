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

namespace org::simple::iir {

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

template <typename T> class CoefficientsSetter {
protected:
  virtual void setValidFF(size_t i, T value) = 0;
  virtual void setValidFB(size_t i, T value) = 0;

public:
  [[nodiscard]] virtual unsigned getOrder() const = 0;
  virtual ~CoefficientsSetter() = default;

  [[nodiscard]] size_t getCoefficients() const { return getOrder() + 1; }

  CoefficientsSetter &setFF(size_t i, T value) {
    setValidX(Index::checked(i, getCoefficients()), value);
    return *this;
  }

  CoefficientsSetter &setFB(size_t i, T value,
                            FeedbackConvention conv = FeedbackConvention::ADD) {
    setValidY(Index::checked(i, getCoefficients()),
              conv == FeedbackConvention::ADD ? value : -value);
    return *this;
  }

  friend void setAmplifyOnly(CoefficientsSetter &setter, T scale) {
    setter.setValidFF(0, scale);
    setter.setValidFB(0, 0.0);
    for (size_t i = 0; i <= setter.getOrder(); i++) {
      setter.setValidFF(i, 0.0);
      setter.setValidFB(i, 0.0);
    }
  }
};

/**
 * Wraps access to coefficient implementation so that algorithms do not need to
 * care about internals nor have to dealwith polymorphism.
 * @tparam coeff Type of coefficient, that MUST be a floating-point.
 * @tparam CoefficientsClass A class that contains coefficients.
 */
template <typename coeff, size_t FIXED_ORDER, class CoefficientsClass>
class Coefficients {
  inline const coeff &getFB(size_t i) const {
    return static_cast<const CoefficientsClass *>(this)->getFB(i);
  }

  inline const coeff &getFF(size_t i) const {
    return static_cast<const CoefficientsClass *>(this)->getFF(i);
  }

  template <size_t Z = FIXED_ORDER>
  requires(Z == 0) size_t inline getOrder() const {
    return static_cast<const CoefficientsClass *>(this)->getOrder();
  }

  template <size_t Z = FIXED_ORDER>
  requires(Z != 0) static constexpr size_t getOrder() { return FIXED_ORDER; }

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
    S Y = 0;
    S xN0 = input;
    S yN0 = 0.0;
    size_t i, j;
    for (i = 0, j = 1; i < getOrder(); i++, j++) {
      const S xN1 = in_history[i];
      const S yN1 = out_history[i];
      in_history[i] = xN0;
      xN0 = xN1;
      out_history[i] = Y;
      Y = yN1;
      yN0 += xN1 * getFF(j) + yN1 * getFB(j);
    }
    yN0 += getFF(0) * input;

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
    const size_t end = count + getOrder();
    for (size_t n = getOrder(); n < end; n++) {
      S yN = getFF(0) * in[n];
      for (size_t j = 1; j <= getOrder(); j++) {
        yN += in[n - j] * getFF(j) + out[n - j] * getFB(j);
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
  void filter_forward_zero(size_t count, const S *__restrict in,
                           S *__restrict out) {
    for (size_t n = 0; n < getOrder(); n++) {
      S yN = getFF(0) * in[n];
      for (size_t j = 1; j <= getOrder(); j++) {
        if (j <= n) {
          yN += in[n - j] * getFF(j) + out[n - j] * getFB(j);
        }
      }
      out[n] = flush_to_zero(yN);
    }
    for (size_t n = getOrder(); n < count; n++) {
      S yN = getFF(0) * in[n];
      for (size_t j = 1; j <= getOrder(); j++) {
        yN += in[n - j] * getFF(j) + out[n - j] * getFB(j);
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
                            S *__restrict out) {

    for (ptrdiff_t n = count - 1; n >= 0; n--) {
      S yN = getFF(0) * in[n];
      for (size_t j = 1; j <= getOrder(); j++) {
        yN += in[n + j] * getFF(j) + out[n + j] * getFB(j);
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
  void filter_backward_zero(size_t count, const S *__restrict in,
                            S *__restrict out) {
    const ptrdiff_t start = count - 1;
    const ptrdiff_t end = count - getOrder();
    ptrdiff_t n;
    for (n = start; n >= end && n >= 0; n--) {
      S yN = getFF(0) * in[n];
      for (size_t j = 1; j <= getOrder(); j++) {
        ptrdiff_t i = n + j;
        if (i <= start) {
          yN += in[i] * getFF(j) + out[i] * getFB(j);
        }
      }
      out[n] = flush_to_zero(yN);
    }
    for (; n >= 0; n--) {
      S yN = getFF(0) * in[n];
      for (size_t j = 1; j <= getOrder(); j++) {
        yN += in[n + j] * getFF(j) + out[n + j] * getFB(j);
      }
      out[n] = flush_to_zero(yN);
    }
  }
};

template <typename C, size_t O, size_t A = 0>
class FixedOrderCoefficients
    : public Coefficients<C, O, FixedOrderCoefficients<C, O, A>>,
      public CoefficientsSetter<C> {

  static_assert(O > 0, "Filter order must be positive.");
  static_assert(O <= 32, "Filter order cannot exceed 32.");
  static constexpr size_t FF = 0;
  static constexpr size_t FB = O + 1;
  Array<C, 2 * (O + 1), A> coeffs;

  void setValidFF(size_t i, C value) override { coeffs[i + FF] = value; }
  void setValidFB(size_t i, C value) override { coeffs[i + FB] = value; }

public:
  inline const C &getFB(size_t i) const { return coeffs[i + FB]; }

  inline const C &getFF(size_t i) const { return coeffs[i + FF]; }

  size_t getOrder() const { return O; }
};

template <typename C, size_t O, size_t A = 0>
class FixedOrderCoefficientsRef
    : public Coefficients<C, O, FixedOrderCoefficientsRef<C, O, A>>,
      public CoefficientsSetter<C> {

  static_assert(O > 0, "Filter order must be positive.");
  static_assert(O <= 32, "Filter order cannot exceed 32.");
  static constexpr size_t FF = 0;
  static constexpr size_t FB = O + 1;
  ArrayDataRefFixedSize<C, 2 * (O + 1), A> coeffs;

  void setValidFF(size_t i, C value) override { coeffs[i + FF] = value; }
  void setValidFB(size_t i, C value) override { coeffs[i + FB] = value; }

public:
  FixedOrderCoefficientsRef(C *const pool_location) : coeffs(pool_location) {}

  inline const C &getFB(size_t i) const { return coeffs[i + FB]; }

  inline const C &getFF(size_t i) const { return coeffs[i + FF]; }

  size_t getOrder() const { return O; }
};

// ArrayDataRefFixedSize

} // namespace org::simple::iir

#endif // ORG_SIMPLE_IIR_COEFFICIENTS_H
