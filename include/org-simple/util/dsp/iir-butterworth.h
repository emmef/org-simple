#ifndef ORG_SIMPLE_UTIL_DSP__IIR_BUTTERWORTH_H
#define ORG_SIMPLE_UTIL_DSP__IIR_BUTTERWORTH_H
/*
 * org-simple/util/dsp/iir-butterworth.h
 *
 * Added by michel on 2021-04-27
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
#include <org-simple/util/dsp/iir-coefficients.h>
#include <org-simple/util/dsp/rate.h>
#include <stdexcept>

namespace org::simple::util::dsp {

struct Butterworth {
  static constexpr size_t MAX_ORDER = 20;
  static constexpr const char *CATEGORY = "Butterworth";

  static inline bool isValidOrder(size_t order) {
    return order > 0 && order <= MAX_ORDER;
  }

  static inline size_t validatedOrder(size_t order) {
    if (isValidOrder(order)) {
      return order;
    }
    throw std::invalid_argument(
        "org::simple::dsp::iir:Butterworth order must be between 1 and 8");
  }

  static inline bool isValidFilterType(FilterType type) {
    return type == FilterType::HIGH_PASS || type == FilterType::LOW_PASS;
  }

  static inline FilterType validatedFilterType(FilterType type) {
    return validated_filter_type(type, isValidFilterType, CATEGORY);
  }

  static inline const char *getTypeName(FilterType type) {
    return get_filter_type_name(type, isValidFilterType);
  }

  static inline double getGain(FilterType type, size_t order,
                               double relative_w0_freq,
                               bool clampFrequency = false) {
    if (validatedFilterType(type) == FilterType::LOW_PASS) {
      return getLowPassGain(order, relative_w0_freq, clampFrequency);
    }
    return getHighPassGain(order, relative_w0_freq, clampFrequency);
  }

  static inline double getHighPassGain(size_t order, double relative_w0_freq,
                                       bool clampFrequency) {
    return get_bw_high_pass_gain(
        validatedOrder(order),
        clampFrequency ? getClampedRelativeFrequency(relative_w0_freq)
                       : relative_w0_freq);
  }

  static inline double getLowPassGain(size_t order, double relative_w0_freq,
                                      bool clampFrequency) {
    return get_bw_low_pass_gain(
        validatedOrder(order),
        clampFrequency ? getClampedRelativeFrequency(relative_w0_freq)
                       : relative_w0_freq);
  }

  static double getClampedRelativeFrequency(double frequency) {
    return frequency;
  }

  template <typename Coefficient>
  static void create(CoefficientsFilter<Coefficient> &coefficients,
                     Rate sampleRate, double frequency, FilterType filterType,
                     Coefficient scale = 1.0) {
    create(coefficients, sampleRate.relative(frequency), filterType, scale);
  }

  template <typename Coefficient>
  static void create(CoefficientsFilter<Coefficient> &coefficients,
                     double relativeFrequency, FilterType filterType,
                     Coefficient scale = 1.0) {
    if (validatedFilterType(filterType) == FilterType::LOW_PASS) {
      setLowPassCoefficients(
          coefficients, getClampedRelativeFrequency(relativeFrequency), scale);
    } else {
      setHighPassCoefficients(
          coefficients, getClampedRelativeFrequency(relativeFrequency), scale);
    }
  }

private:

  static double get_bw_high_pass_gain(size_t order, double relative_w0_freq) {
    double alpha = pow(fabs(relative_w0_freq), order);
    return alpha / sqrt(1.0 + alpha * alpha);
  }

  static double get_bw_low_pass_gain(size_t order, double relative_w0_freq) {
    double alpha2 = pow(fabs(relative_w0_freq), order * 2);
    return 1.0 / sqrt(1.0 + alpha2);
  }

  template <typename Coefficient>
  static void
  setLowPassCoefficients(CoefficientsFilter<Coefficient> &coefficients,
                         double relativeFrequency, Coefficient scale = 1.0) {
    size_t order = validatedOrder(coefficients.getOrder());
    int unscaledCCoefficients[MAX_ORDER + 1];
    getUnscaledLowPassCCoefficients(order, unscaledCCoefficients);

    setFeedForwardCoefficients(coefficients, relativeFrequency);

    double scaleOfC = scale * getLowPassScalingFactor(order, relativeFrequency);

    for (size_t i = 0; i <= order; i++) {
      coefficients.setFB(i, scaleOfC * unscaledCCoefficients[i]);
    }
  }

  template <typename Coefficient>
  static void
  setHighPassCoefficients(CoefficientsFilter<Coefficient> &coefficients,
                          double relativeFrequency, Coefficient scale = 1.0) {
    size_t order = validatedOrder(coefficients.getOrder());
    int unscaledCCoefficients[MAX_ORDER + 1];
    getUnscaledHighPassCCoefficients(order, unscaledCCoefficients);

    setFeedForwardCoefficients(coefficients, relativeFrequency);

    double scaleOfC =
        scale * getHighPassScalingFactor(order, relativeFrequency);

    for (size_t i = 0; i <= order; i++) {
      coefficients.setFB(i, scaleOfC * unscaledCCoefficients[i]);
    }
  }

  template <typename Coefficient>
  static void
  setFeedForwardCoefficients(CoefficientsFilter<Coefficient> &d_coefficients,
                             double relativeFrequency) {
    const size_t order = d_coefficients.getOrder();
    size_t totalCoefficients = order * 2 + 1;
    double fbCoeffs[MAX_ORDER * 2 + 1];
    memset(&fbCoeffs, 0, totalCoefficients * sizeof(double));
    // binomial coefficients
    double binomials[MAX_ORDER * 2 + 2];
    memset(&binomials, 0, (2 * order + 2) * sizeof(double));

    double theta = M_PI * 2 * relativeFrequency;
    double st = sin(theta);
    double ct = cos(theta);

    for (unsigned k = 0; k < order; ++k) {
      double parg = M_PI * (double)(2 * k + 1) / (double)(2 * order);
      double sparg = sin(parg);
      double cparg = cos(parg);
      double a = 1.0 + st * sparg;
      binomials[2 * k] = -ct / a;
      binomials[2 * k + 1] = -st * cparg / a;
    }

    for (unsigned i = 0; i < order; ++i) {
      for (int j = i; j > 0; --j) {
        fbCoeffs[2 * j] += binomials[2 * i] * fbCoeffs[2 * (j - 1)] -
                           binomials[2 * i + 1] * fbCoeffs[2 * (j - 1) + 1];
        fbCoeffs[2 * j + 1] += binomials[2 * i] * fbCoeffs[2 * (j - 1) + 1] +
                               binomials[2 * i + 1] * fbCoeffs[2 * (j - 1)];
      }
      fbCoeffs[0] += binomials[2 * i];
      fbCoeffs[1] += binomials[2 * i + 1];
    }

    fbCoeffs[1] = fbCoeffs[0];
    fbCoeffs[0] = 1.0;
    for (unsigned k = 3; k <= order; ++k) {
      fbCoeffs[k] = fbCoeffs[2 * k - 2];
    }
    for (unsigned i = 0; i <= order; i++) {
      /*
       * Negate coefficients as this calculus was meant for recursive equations
       * where they where subtracted instead of added. We do adds only, so we
       * need to negate them here.
       */
      d_coefficients.setFF(i, -fbCoeffs[i]);
    }
  }

  static void getUnscaledLowPassCCoefficients(size_t order, int *ffCoeffs) {
    for (size_t i = 0; i <= order; i++) {
      ffCoeffs[i] = 0;
    }

    ffCoeffs[0] = 1;
    ffCoeffs[1] = order;

    for (int m = order / 2, i = 2; i <= m; ++i) {
      ffCoeffs[i] = (order - i + 1) * ffCoeffs[i - 1] / i;
      ffCoeffs[order - i] = ffCoeffs[i];
    }
    ffCoeffs[order - 1] = order;
    ffCoeffs[order] = 1;
  }

  static double getLowPassScalingFactor(size_t order,
                                        double relativeFrequency) {
    double omega = M_PI * 2 * relativeFrequency;
    double fomega = sin(omega);
    double parg0 = M_PI / (double)(2 * order);

    double sf = 1.0; // scaling factor
    for (size_t k = 0; k < order / 2; ++k) {
      sf *= 1.0 + fomega * sin((double)(2 * k + 1) * parg0);
    }

    fomega = sin(omega / 2.0);

    if (order % 2) {
      sf *= fomega + cos(omega / 2.0);
    }
    sf = pow(fomega, order) / sf;

    return (sf);
  }

  static double getHighPassScalingFactor(size_t order,
                                         double relativeFrequency) {
    double omega = M_PI * 2 * relativeFrequency;

    double sf = 1.0;
    double sin_omega = sin(omega);
    double parg0 = M_PI / (double)(2 * order); // zero-th pole angle
    for (size_t k = 0; k < order / 2; ++k) {
      sf *= 1.0 + sin_omega * sin((double)(2 * k + 1) * parg0);
    }

    double cos_omega = cos(omega / 2.0);

    if (order % 2) {
      sf *= cos_omega + sin(omega / 2.0);
    }
    sf = pow(cos_omega, order) / sf;

    return (sf);
  }

  static void getUnscaledHighPassCCoefficients(size_t order, int *ccof) {
    getUnscaledLowPassCCoefficients(order, ccof);

    for (size_t i = 0; i <= order; ++i) {
      if (i % 2) {
        ccof[i] = -ccof[i];
      }
    }
  }
};

} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_UTIL_DSP__IIR_BUTTERWORTH_H
