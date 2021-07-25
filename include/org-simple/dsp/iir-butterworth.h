#ifndef ORG_SIMPLE_IIR_BUTTERWORTH_H
#define ORG_SIMPLE_IIR_BUTTERWORTH_H
/*
 * org-simple/dsp/iir-butterworth.h
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
#include <org-simple/dsp/iir-coefficients.h>
#include <org-simple/dsp/rate.h>
#include <stdexcept>

namespace org::simple::dsp::iir {

static constexpr bool is_valid_bw_order(size_t order) {
  return order >= 1 && order <= 20;
}

static unsigned short valid_bw_order(size_t order) {
  if (is_valid_bw_order(order)) {
    return order;
  }
  throw std::invalid_argument(
      "valid_bw_order: order must be between 1 and 20.");
}

template <typename... A>
static size_t valid_bw_order(const Coefficients<A...> &coeffs) {
  return valid_bw_order(coeffs.getOrder());
}

static bool is_supported_bw_type(FilterType type) {
  return type == FilterType::HIGH_PASS || type == FilterType::LOW_PASS;
}

static double get_wb_high_pass_gain(size_t order, double relative_w0_freq) {
  double alpha = pow(fabs(relative_w0_freq), valid_bw_order(order));
  return alpha / sqrt(1.0 + alpha * alpha);
}

static double get_wb_low_pass_gain(size_t order, double relative_w0_freq) {
  double alpha2 = pow(fabs(relative_w0_freq), valid_bw_order(order) * 2);
  return 1.0 / sqrt(1.0 + alpha2);
}

static bool is_bw_valid_relative_frequency(double relative) {
  return relative >= std::numeric_limits<double>::epsilon() && relative <= 0.5;
}

template <typename... S>
[[nodiscard]] static bool
are_bw_valid_parameters(Coefficients<S...> &coefficients,
                        double relativeFrequency, FilterType filterType) {
  return is_bw_valid_relative_frequency(relativeFrequency) &&
         is_supported_bw_type(filterType) &&
         is_valid_bw_order(coefficients.getOrder());
}

template <typename... S>
[[nodiscard]] static bool
are_bw_valid_parameters(Coefficients<S...> &coefficients, Rate sampleRate,
                        double frequency, FilterType filterType) {
  return are_bw_valid_parameters(coefficients, sampleRate.relative(frequency),
                                 filterType);
}

struct Butterworth {
  static constexpr size_t getMaxOrder() { return 20; }

  static bool isValidOrder(size_t order) {
    return order > 0 && order <= getMaxOrder();
  }

  static size_t validOrder(size_t order) {
    if (isValidOrder(order)) {
      return order;
    }
    throw std::invalid_argument("Butterworth order must be between 1 and 20");
  }

  static double getHighPassGain(size_t order, double relative_w0_freq) {
    return get_wb_high_pass_gain(order, relative_w0_freq);
  }

  static double getLowPassGain(size_t order, double relative_w0_freq) {
    return get_wb_low_pass_gain(order, relative_w0_freq);
  }

  static double relativeNycquistLimitedFrequency(Rate sampleRate,
                                                           double frequency) {
    return std::clamp(sampleRate.relative(frequency),
                        -0.5, 0.5);
  }

  template <typename Coefficient>
  static void create(CoefficientsSetter<Coefficient> &coefficients,
                     Rate sampleRate, double frequency, FilterType filterType,
                     Coefficient scale = 1.0) {
    create(coefficients,
           relativeNycquistLimitedFrequency(frequency, sampleRate), filterType,
           scale);
  }

  template <typename Coefficient>
  static void create(CoefficientsSetter<Coefficient> &coefficients,
                     double relativeFrequency, FilterType filterType,
                     Coefficient scale = 1.0) {
    switch (filterType) {
    case FilterType::LOW_PASS:
      getLowPassCoefficients(coefficients, relativeFrequency, scale);
      return;
    case FilterType::HIGH_PASS:
      getHighPassCoefficients(coefficients, relativeFrequency, scale);
      return;
    default:
      throw std::invalid_argument("Unknown filter type (must be low or high");
    }
  }

  template <typename Coefficient>
  static void
  getLowPassCoefficients(CoefficientsSetter<Coefficient> &coefficients,
                         double relativeFrequency, Coefficient scale = 1.0) {
    size_t order = validOrder(coefficients.order());
    int unscaledCCoefficients[coefficients.getCoefficientCount()];
    getUnscaledLowPassCCoefficients(order, unscaledCCoefficients);

    setFeedbackCoefficients(coefficients, relativeFrequency);

    double scaleOfC = scale * getLowPassScalingFactor(order, relativeFrequency);

    for (size_t i = 0; i <= order; i++) {
      coefficients.setFF(i, scaleOfC * unscaledCCoefficients[i]);
    }
  }

  template <typename Coefficient>
  static void
  getHighPassCoefficients(CoefficientsSetter<Coefficient> &coefficients,
                          double relativeFrequency, Coefficient scale = 1.0) {
    size_t order = validOrder(coefficients.order());
    int unscaledCCoefficients[coefficients.getCoefficientCount()];
    getUnscaledHighPassCCoefficients(order, unscaledCCoefficients);

    setFeedbackCoefficients(coefficients, relativeFrequency);

    double scaleOfC =
        scale * getHighPassScalingFactor(order, relativeFrequency);

    for (size_t i = 0; i <= order; i++) {
      coefficients.setFF(i, scaleOfC * unscaledCCoefficients[i]);
    }
  }

private:
  template <typename Coefficient>
  static void
  setFeedbackCoefficients(CoefficientsSetter<Coefficient> &d_coefficients,
                          double relativeFrequency) {
    const size_t order = d_coefficients.getOrder();
    double fbCoeffs[order * 2 + 1];
    memset(&fbCoeffs, 0, sizeof(fbCoeffs));
    // binomial coefficients
    double binomials[2 * order + 2];
    memset(&binomials, 0, sizeof(binomials));

    double theta = M_PI * 2 * relativeFrequency;
    double st = sin(theta);
    double ct = cos(theta);

    for (int k = 0; k < order; ++k) {
      double parg = M_PI * (double)(2 * k + 1) / (double)(2 * order);
      double sparg = sin(parg);
      double cparg = cos(parg);
      double a = 1.0 + st * sparg;
      binomials[2 * k] = -ct / a;
      binomials[2 * k + 1] = -st * cparg / a;
    }

    for (int i = 0; i < order; ++i) {
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
    for (int k = 3; k <= order; ++k) {
      fbCoeffs[k] = fbCoeffs[2 * k - 2];
    }
    for (int i = 0; i <= order; i++) {
      /*
       * Negate coefficients as this calculus was meant for recursive equations
       * where they where subtracted instead of added. We do adds only, so we
       * need to negate them here.
       */
      d_coefficients.setFB(i, -fbCoeffs[i]);
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

  static double getLowPassScalingFactor(size_t order, double relativeFrequency) {
    double omega = M_PI * 2 * relativeFrequency;
    double fomega = sin(omega);
    double parg0 = M_PI / (double)(2 * order);

    double sf = 1.0;     // scaling factor
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

  static double getHighPassScalingFactor(size_t order, double relativeFrequency) {
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

} // namespace org::simple::dsp::iir

#endif // ORG_SIMPLE_IIR_BUTTERWORTH_H
