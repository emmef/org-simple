#ifndef ORG_SIMPLE_UTIL_DSP_M_IIR_FILTER_H
#define ORG_SIMPLE_UTIL_DSP_M_IIR_FILTER_H
/*
 * org-simple/util/dsp/iir-filter.h
 *
 * Added by michel on 2021-03-21
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
#include <org-simple/core/ZeroNonNormal.h>
#include <type_traits>

namespace org::simple::util::dsp {

/**
 * Filters a single sample using the feed forward coefficients \c ff_coeffs,
 * feed backward coefficients \c fb_coeffs, sample histories for input and
 * outputs \c xHistory and \c yHistory and the filter order \c order.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param xHistory The history of input samples (from recent to past).
 * @param yHistory The history og output samples (from recent to past).
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param x The sample to filter
 * @return The filtered sample value.
 */
template <typename S, typename C, size_t ORDER>
S filter_single_fo(S *__restrict xHistory, S *__restrict yHistory,
                   const C *__restrict ff_coeffs, const C *__restrict fb_coeffs,
                   const S x) {
  static_assert(std::is_floating_point_v<C>);
  S Y = 0;
  S X = x; // input is xN0
  S yN0 = 0.0;
  size_t i, j;
  for (i = 0, j = 1; i < ORDER; i++, j++) {
    const S xN1 = xHistory[i];
    const S yN1 = yHistory[i];
    xHistory[i] = X;
    X = xN1;
    yHistory[i] = Y;
    Y = yN1;
    yN0 += xN1 * ff_coeffs[j] - yN1 * fb_coeffs[j];
  }
  yN0 += ff_coeffs[0] * x;

  yHistory[0] = flush_to_zero(yN0);

  return yN0;
}

/**
 * Filters a single sample using the feed forward coefficients \c ff_coeffs,
 * feed backward coefficients \c fb_coeffs, sample histories for input and
 * outputs \c xHistory and \c yHistory and the filter order \c order.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param order The order of the filter.
 * @param xHistory The history of input samples (from recent to past).
 * @param yHistory The history og output samples (from recent to past).
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param x The sample to filter
 * @return The filtered sample value.
 */
template <typename S, typename C>
S filter_single(size_t order, S *__restrict xHistory, S *__restrict yHistory,
                const C *__restrict ff_coeffs, const C *__restrict fb_coeffs,
                const S x) {

  S Y = 0;
  S X = x; // input is xN0
  S yN0 = 0.0;
  size_t i, j;
  for (i = 0, j = 1; i < order; i++, j++) {
    const S xN1 = xHistory[i];
    const S yN1 = yHistory[i];
    xHistory[i] = X;
    X = xN1;
    yHistory[i] = Y;
    Y = yN1;
    yN0 += xN1 * ff_coeffs[j] - yN1 * fb_coeffs[j];
  }
  yN0 += ff_coeffs[0] * x;

  yHistory[0] = flush_to_zero(yN0);

  return yN0;
}

/**
 * Filters \c count samples from an input buffer \c in to an output buffer \c
 * out, starting at offset \c ORDER, using feed forward coefficients \c
 * ff_coeffs, feed backward coefficients \c fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 * @param out The output sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 */
template <typename S, typename C, size_t ORDER>
void filter_forward_foo(size_t count, const C *__restrict ff_coeffs,
                        const C *__restrict fb_coeffs, const S *__restrict in,
                        S *__restrict out) {
  const size_t end = count + ORDER;
  for (size_t n = ORDER; n < end; n++) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= ORDER; j++) {
      yN += in[n - j] * ff_coeffs[j] - out[n - j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples from an input buffer \c in to an output buffer \c
 * out, starting at offset 0, assuming all past input and output values zero and
 * using feed forward coefficients \c ff_coeffs, feed backward coefficients \c
 * fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count.
 * @param out The output sample buffer, that must at least have a length of \c
 * count.
 */
template <typename S, typename C, size_t ORDER>
void filter_forward_fzp(size_t count, const C *__restrict ff_coeffs,
                        const C *__restrict fb_coeffs, const S *__restrict in,
                        S *__restrict out) {
  for (size_t n = 0; n < ORDER; n++) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= ORDER; j++) {
      if (j <= n) {
        yN += in[n - j] * ff_coeffs[j] - out[n - j] * fb_coeffs[j];
      }
    }
    out[n] = flush_to_zero(yN);
  }
  for (size_t n = ORDER; n < count; n++) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= ORDER; j++) {
      yN += in[n - j] * ff_coeffs[j] - out[n - j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples from an input buffer \c in to an output buffer \c
 * out, starting at offset \c order, using feed forward coefficients \c
 * ff_coeffs, feed backward coefficients \c fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 * @param out The output sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 */
template <typename S, typename C>
void filter_forward_oo(size_t order, size_t count,
                       const C *__restrict ff_coeffs,
                       const C *__restrict fb_coeffs, const S *__restrict in,
                       S *__restrict out) {
  const size_t end = count + order;
  for (size_t n = order; n < end; n++) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= order; j++) {
      yN += in[n - j] * ff_coeffs[j] - out[n - j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples from an input buffer \c in to an output buffer \c
 * out, starting at offset 0, assuming all past input and output values zero and
 * using feed forward coefficients \c ff_coeffs, feed  backward coefficients \c
 * fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count.
 * @param out The output sample buffer, that must at least have a length of \c
 * count.
 */
template <typename S, typename C>
void filter_forward_zp(size_t order, size_t count,
                       const C *__restrict ff_coeffs,
                       const C *__restrict fb_coeffs, const S *__restrict in,
                       S *__restrict out) {
  for (size_t n = 0; n < order; n++) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= order; j++) {
      if (j <= n) {
        yN += in[n - j] * ff_coeffs[j] - out[n - j] * fb_coeffs[j];
      }
    }
    out[n] = flush_to_zero(yN);
  }
  for (size_t n = order; n < count; n++) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= order; j++) {
      yN += in[n - j] * ff_coeffs[j] - out[n - j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples backwards in time from an input buffer \c in to an
 * output buffer \c out, starting at offset \c count - 1, using feed forward
 * coefficients \c ff_coeffs, feed backward coefficients \c fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 * @param out The output sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 */
template <typename S, typename C, size_t ORDER>
void filter_backward_foo(size_t count, const C *__restrict ff_coeffs,
                         const C *__restrict fb_coeffs, const S *__restrict in,
                         S *__restrict out) {

  for (ptrdiff_t n = count - 1; n >= 0; n--) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= ORDER; j++) {
      yN += in[n + j] * ff_coeffs[j] - out[n + j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples backwards in time from an input buffer \c in to an
 * output buffer \c out, starting at offset \c count - 1, assuming all past
 * input and output values zero and using feed forward coefficients \c
 * ff_coeffs, feed backward coefficients \c fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam ORDER The order of the filter.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count.
 * @param out The output sample buffer, that must at least have a length of \c
 * count.
 */
template <typename S, typename C, size_t ORDER>
void filter_backward_fzp(size_t count, const C *__restrict ff_coeffs,
                         const C *__restrict fb_coeffs, const S *__restrict in,
                         S *__restrict out) {
  const ptrdiff_t start = count - 1;
  const ptrdiff_t end = count - ORDER;
  ptrdiff_t n;
  for (n = start; n >= end && n >= 0; n--) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= ORDER; j++) {
      ptrdiff_t i = n + j;
      if (i <= start) {
        yN += in[i] * ff_coeffs[j] - out[i] * fb_coeffs[j];
      }
    }
    out[n] = flush_to_zero(yN);
  }
  for (; n >= 0; n--) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= ORDER; j++) {
      yN += in[n + j] * ff_coeffs[j] - out[n + j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples backwards in time from an input buffer \c in to an
 * output buffer \c out, starting at offset \c count - 1, using feed forward
 * coefficients \c ff_coeffs, feed backward coefficients \c fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param order The order of the filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 * @param out The output sample buffer, that must at least have a length of \c
 * count + \c ORDER.
 */
template <typename S, typename C>
void filter_backward_oo(size_t order, size_t count,
                        const C *__restrict ff_coeffs,
                        const C *__restrict fb_coeffs, const S *__restrict in,
                        S *__restrict out) {
  for (ptrdiff_t n = count - 1; n >= 0; n--) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= order; j++) {
      yN += in[n + j] * ff_coeffs[j] - out[n + j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

/**
 * Filters \c count samples backwards in time from an input buffer \c in to an
 * output buffer \c out, starting at offset \c count - 1, assuming all past
 * input and output values zero and using feed forward coefficients \c
 * ff_coeffs, feed backward coefficients \c fb_coeffs.
 * @tparam S The type of sample.
 * @tparam C The type of coefficients, which must be a floating point.
 * @tparam FB_SIGN The sign convention of the feedback coefficients of the
 * filter.
 * @param order The order of the filter.
 * @param count The number of samples to filter.
 * @param ff_coeffs The filter feed-forward coefficients.
 * @param fb_coeffs The filter feedback coefficients.
 * @param in The input sample buffer, that must at least have a length of \c
 * count.
 * @param out The output sample buffer, that must at least have a length of \c
 * count.
 */
template <typename S, typename C>
void filter_backward_zp(size_t order, size_t count,
                        const C *__restrict ff_coeffs,
                        const C *__restrict fb_coeffs, const S *__restrict in,
                        S *__restrict out) {
  const ptrdiff_t start = count - 1;
  const ptrdiff_t end = count - order;
  ptrdiff_t n;
  for (n = start; n >= end && n >= 0; n--) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= order; j++) {
      ptrdiff_t i = n + j;
      if (i <= start) {
        yN += in[i] * ff_coeffs[j] - out[i] * fb_coeffs[j];
      }
    }
    out[n] = flush_to_zero(yN);
  }
  for (; n >= 0; n--) {
    S yN = ff_coeffs[0] * in[n];
    for (size_t j = 1; j <= order; j++) {
      yN += in[n + j] * ff_coeffs[j] - out[n + j] * fb_coeffs[j];
    }
    out[n] = flush_to_zero(yN);
  }
}

} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_UTIL_DSP_M_IIR_FILTER_H
