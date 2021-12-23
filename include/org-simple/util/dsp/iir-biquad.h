#ifndef ORG_SIMPLE_IIR_BIQUAD_H
#define ORG_SIMPLE_IIR_BIQUAD_H
/*
 * org-simple/util/dsp/iir-biquad.h
 *
 * Added by michel on 2021-07-30
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

namespace org::simple::util::dsp {

class Biquad {

  struct BiQuadCoefficients {
    double C0 = 0;
    double C1 = 0;
    double C2 = 0;
    double D1 = 0;
    double D2 = 0;
  };

  template <typename C>
  static inline void setCoefficients(CoefficientsFilter<C> &coefficients,
                                     const BiQuadCoefficients bqc) {
    coefficients.setFB(0, bqc.C0);
    coefficients.setFB(1, bqc.C1);
    coefficients.setFB(2, bqc.C2);
    coefficients.setFF(0, 0);
    coefficients.setFF(1, bqc.D1);
    coefficients.setFF(2, bqc.D2);
  }

public:
  static constexpr double BANDWITH_MINIMUM = 0.0625;
  static constexpr double BANDWITH_MAXIMUM = 16;
  static constexpr double SLOPE_MINIMUM = 0.0001;
  static constexpr double SLOPE_MAXIMUM = 1;
  static constexpr double CENTER_RELATIVE_MINIMUM =
      std::numeric_limits<double>::epsilon();
  static constexpr double CENTER_RELATIVE_MAXIMUM =
      0.5 - std::numeric_limits<double>::epsilon();
  static constexpr double GAIN_MINIMUM = 0.01;
  static constexpr double GAIN_MAXIMUM = 100;
  static constexpr FilterType SUPPORTED_FILTER_TYPES[] = {
      FilterType::BAND_PASS, FilterType::HIGH_PASS,  FilterType::HIGH_SHELVE,
      FilterType::LOW_PASS,  FilterType::LOW_SHELVE, FilterType::PARAMETRIC};

  static inline bool isValidFilterType(FilterType type) {
    for (const FilterType supported : SUPPORTED_FILTER_TYPES) {
      if (type == supported) {
        return true;
      }
    }
    return false;
  }

  static inline double clampedRelativeFrequency(double relative) {
    return std::clamp(relative, CENTER_RELATIVE_MINIMUM,
                      CENTER_RELATIVE_MAXIMUM);
  }

  static inline double clampedRelativeFrequency(Rate rate, double frequency) {
    return clampedRelativeFrequency(rate.relative(frequency));
  }

  static inline double clampedGain(double gain) {
    return std::clamp(gain, GAIN_MINIMUM, GAIN_MAXIMUM);
  }

  static inline double clampedSlope(double slope) {
    return std::clamp(slope, SLOPE_MINIMUM, SLOPE_MAXIMUM);
  }

  static inline double clampedBandwidth(double bandwidth) {
    return std::clamp(bandwidth, BANDWITH_MINIMUM, BANDWITH_MAXIMUM);
  }

  template <class T>
  static inline void validateCoefficients(CoefficientsFilter<T> &filter) {
    if (filter.getOrder() != 2) {
      throw std::invalid_argument(
          "org::simple::dsp::iir::Biquad: filter order must be 2.");
    }
  }

  template <class T>
  static inline void generateParametric(CoefficientsFilter<T> &coefficients,
                                        double center, double gain,
                                        double bandwidth) {
    validateCoefficients(coefficients);
    static constexpr double LN_2_2 = (M_LN2 / 2);
    double omega = M_PI * 2 * clampedRelativeFrequency(center);
    double cw = cos(omega);
    double sw = sin(omega);
    double J = sqrt(clampedGain(gain));
    double g = sw * sinh(LN_2_2 * clampedBandwidth(bandwidth) * omega / sw);
    double a0r = 1.0 / (1.0 + (g / J));

    BiQuadCoefficients result;
    result.C0 = (1.0 + (g * J)) * a0r;
    result.C1 = (-2.0 * cw) * a0r;
    result.C2 = (1.0 - (g * J)) * a0r;
    result.D1 = -result.C1;
    result.D2 = ((g / J) - 1.0) * a0r;

    setCoefficients(coefficients, result);
  }

  template <class T>
  static inline void generateParametric(CoefficientsFilter<T> &coefficients,
                                        Rate rate, double center, double gain,
                                        double bandwidth) {
    generateParametric(coefficients, rate.relative(center), gain, bandwidth);
  }

  template <class T>
  static inline void generateHighShelve(CoefficientsFilter<T> &coefficients,
                                        double center, double gain,
                                        double slope) {
    double omega = M_PI * 2 * clampedRelativeFrequency(center);
    double cw = cos(omega);
    double sw = sin(omega);
    double A = sqrt(clampedGain(gain));
    double b = sqrt(((1.0f + A * A) / clampedSlope(slope)) -
                    ((A - 1.0f) * (A - 1.0f)));
    double apc = cw * (A + 1.0f);
    double amc = cw * (A - 1.0f);
    double bs = b * sw;
    double a0r = 1.0f / (A + 1.0f - amc + bs);

    BiQuadCoefficients result;
    result.C0 = a0r * A * (A + 1.0f + amc + bs);
    result.C1 = a0r * -2.0f * A * (A - 1.0f + apc);
    result.C2 = a0r * A * (A + 1.0f + amc - bs);
    result.D1 = a0r * -2.0f * (A - 1.0f - apc);
    result.D2 = a0r * (-A - 1.0f + amc + bs);

    setCoefficients(coefficients, result);
  }

  template <class T>
  static inline void generateHighShelve(CoefficientsFilter<T> &coefficients,
                                        Rate rate, double center, double gain,
                                        double slope) {
    generateHighShelve(coefficients, rate.relative(center), gain, slope);
  }

  template <class T>
  static inline void generateLowShelve(CoefficientsFilter<T> &coefficients,
                                       double center, double gain,
                                       double slope) {
    validateCoefficients(coefficients);
    double omega = M_PI * 2 * clampedRelativeFrequency(center);
    double cw = cos(omega);
    double sw = sin(omega);
    double A = sqrt(clampedGain(gain));
    double b =
        sqrt(((1.0f + A * A) / clampedSlope(slope)) - ((A - 1.0) * (A - 1.0)));
    double apc = cw * (A + 1.0f);
    double amc = cw * (A - 1.0f);
    double bs = b * sw;
    double a0r = 1.0f / (A + 1.0f + amc + bs);

    BiQuadCoefficients result;
    result.C0 = a0r * A * (A + 1.0f - amc + bs);
    result.C1 = a0r * 2.0f * A * (A - 1.0f - apc);
    result.C2 = a0r * A * (A + 1.0f - amc - bs);
    result.D1 = a0r * 2.0f * (A - 1.0f + apc);
    result.D2 = a0r * (-A - 1.0f - amc + bs);

    setCoefficients(coefficients, result);
  }

  template <class T>
  static inline void generateLowShelve(CoefficientsFilter<T> &coefficients,
                                       Rate rate, double center, double gain,
                                       double slope) {
    generateLowShelve(coefficients, rate.relative(center), gain, slope);
  }

  template <class T>
  static inline void generateBandPass(CoefficientsFilter<T> &coefficients,
                                      double center, double bandwidth) {
    validateCoefficients(coefficients);
    double omega = M_PI * 2 * clampedRelativeFrequency(center);
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha =
        sn * sinh(M_LN2 / 2.0 * clampedBandwidth(bandwidth) * omega / sn);
    const double a0r = 1.0 / (1.0 + alpha);

    BiQuadCoefficients result;
    result.C0 = a0r * alpha;
    result.C1 = 0.0;
    result.C2 = a0r * -alpha;
    result.D1 = a0r * (2.0 * cs);
    result.D2 = a0r * (alpha - 1.0);

    setCoefficients(coefficients, result);
  }

  template <class T>
  static inline void generateBandPass(CoefficientsFilter<T> &coefficients,
                                      Rate rate, double center,
                                      double bandwidth) {
    generateBandPass(coefficients, rate.relative(center), bandwidth);
  }

  template <class T>
  static inline void generateHighPass(CoefficientsFilter<T> &coefficients,
                                      double center, double bandwidth) {
    validateCoefficients(coefficients);
    double omega = M_PI * 2 * clampedRelativeFrequency(center);
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha =
        sn * sinh(M_LN2 / 2.0 * clampedBandwidth(bandwidth) * omega / sn);
    const double a0r = 1.0 / (1.0 + alpha);

    BiQuadCoefficients result;
    result.C0 = a0r * (1.0 + cs) * 0.5;
    result.C1 = a0r * -(1.0 + cs);
    result.C2 = a0r * (1.0 + cs) * 0.5;
    result.D1 = a0r * (2.0 * cs);
    result.D2 = a0r * (alpha - 1.0);

    setCoefficients(coefficients, result);
  }

  template <class T>
  static inline void generateHighPass(CoefficientsFilter<T> &coefficients,
                                      Rate rate, double center,
                                      double bandwidth) {
    generateHighPass(coefficients, rate.relative(center), bandwidth);
  }

  template <class T>
  static inline void generateLowPass(CoefficientsFilter<T> &coefficients,
                                     double center, double bandwidth) {
    validateCoefficients(coefficients);
    double omega = M_PI * 2 * clampedRelativeFrequency(center);
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha =
        sn * sinh(M_LN2 / 2.0 * clampedBandwidth(bandwidth) * omega / sn);
    const double a0r = 1.0 / (1.0 + alpha);

    BiQuadCoefficients result;
    result.C0 = a0r * (1.0 - cs) * 0.5;
    result.C1 = a0r * (1.0 - cs);
    result.C2 = a0r * (1.0 - cs) * 0.5;
    result.D1 = a0r * (2.0 * cs);
    result.D2 = a0r * (alpha - 1.0);

    setCoefficients(coefficients, result);
  }

  template <class T>
  static inline void generateLowPass(CoefficientsFilter<T> &coefficients,
                                     Rate rate, double center,
                                     double bandwidth) {
    generateLowPass(coefficients, rate.relative(center), bandwidth);
  }

  class ConfigCenter {
    double center_;

  protected:
    explicit ConfigCenter(double relativeCenter)
        : center_(clampedRelativeFrequency(relativeCenter)) {}
    void set(double relativeCenter) {
      center_ = clampedRelativeFrequency(relativeCenter);
    }
    void set(Rate rate, double center) {
      center_ = clampedRelativeFrequency(rate, center);
    }

  public:
    [[nodiscard]] inline double center() const { return center_; }
    [[nodiscard]] virtual FilterType type() const = 0;
    [[nodiscard]] inline const char *typeName() const {
      return get_filter_type_name(type(), isValidFilterType);
    }
    ~ConfigCenter() = default;
  };

  class ConfigGain {
    double gain_ = 1.0;

  protected:
    inline void set(double gain) { gain_ = clampedGain(gain); }

  public:
    [[nodiscard]] inline double gain() const { return gain_; }
  };

  class ConfigSlope {
    double slope_ = 1.0;

  protected:
    inline void set(double slope) { slope_ = clampedSlope(slope); }

  public:
    [[nodiscard]] inline double slope() const { return slope_; }
  };

  class ConfigBandwidth {
    double bandwidth_ = 1.0;

  protected:
    inline void set(double bandwidth) {
      bandwidth_ = clampedBandwidth(bandwidth);
    }

  public:
    [[nodiscard]] inline double bandwidth() const { return bandwidth_; }
  };

  class Parametric : public ConfigCenter,
                     public ConfigGain,
                     public ConfigBandwidth {
  public:
    explicit Parametric(double relativeCenter) : ConfigCenter(relativeCenter) {}

    inline Parametric &setCenter(double relativeCenter) {
      ConfigCenter::set(relativeCenter);
      return *this;
    }

    inline Parametric &setCenter(Rate rate, double center) {
      ConfigCenter::set(rate, center);
      return *this;
    }

    inline Parametric &setGain(double gain) {
      ConfigGain::set(gain);
      return *this;
    }

    inline Parametric &setBandwidth(double bandwidth) {
      ConfigBandwidth::set(bandwidth);
      return *this;
    }

    [[nodiscard]] inline FilterType type() const final {
      return FilterType::PARAMETRIC;
    }

    template <typename S>
    inline void build(CoefficientsFilter<S> &coefficients) {
      generateParametric(coefficients, center(), gain(), bandwidth());
    }
  };

  class HighShelve : public ConfigCenter,
                     public ConfigGain,
                     public ConfigSlope {
  public:
    explicit HighShelve(double relativeCenter) : ConfigCenter(relativeCenter) {}

    inline HighShelve &setCenter(double relativeCenter) {
      ConfigCenter::set(relativeCenter);
      return *this;
    }

    inline HighShelve &setCenter(Rate rate, double center) {
      ConfigCenter::set(rate, center);
      return *this;
    }

    inline HighShelve &setGain(double gain) {
      ConfigGain::set(gain);
      return *this;
    }

    inline HighShelve &setSlope(double slope) {
      ConfigSlope::set(slope);
      return *this;
    }

    [[nodiscard]] inline FilterType type() const final {
      return FilterType::HIGH_SHELVE;
    }

    template <typename S>
    inline void build(CoefficientsFilter<S> &coefficients) {
      generateHighShelve(coefficients, center(), gain(), slope());
    }
  };

  class LowShelve : public ConfigCenter, public ConfigGain, public ConfigSlope {
  public:
    explicit LowShelve(double relativeCenter) : ConfigCenter(relativeCenter) {}

    inline LowShelve &setCenter(double relativeCenter) {
      ConfigCenter::set(relativeCenter);
      return *this;
    }

    inline LowShelve &setCenter(Rate rate, double center) {
      ConfigCenter::set(rate, center);
      return *this;
    }

    inline LowShelve &setGain(double gain) {
      ConfigGain::set(gain);
      return *this;
    }

    inline LowShelve &setSlope(double slope) {
      ConfigSlope::set(slope);
      return *this;
    }

    [[nodiscard]] inline FilterType type() const final {
      return FilterType::LOW_SHELVE;
    }

    template <typename S>
    inline void build(CoefficientsFilter<S> &coefficients) {
      generateLowShelve(coefficients, center(), gain(), slope());
    }
  };

  class BandPass : public ConfigCenter, public ConfigBandwidth {
  public:
    explicit BandPass(double relativeCenter) : ConfigCenter(relativeCenter) {}

    inline BandPass &setCenter(double relativeCenter) {
      ConfigCenter::set(relativeCenter);
      return *this;
    }

    inline BandPass &setCenter(Rate rate, double center) {
      ConfigCenter::set(rate, center);
      return *this;
    }

    inline BandPass &setBandwidth(double bandwidth) {
      ConfigBandwidth::set(bandwidth);
      return *this;
    }

    [[nodiscard]] inline FilterType type() const final {
      return FilterType::BAND_PASS;
    }

    template <typename S>
    inline void build(CoefficientsFilter<S> &coefficients) {
      generateBandPass(coefficients, center(), bandwidth());
    }
  };

  class LowPass : public ConfigCenter, public ConfigBandwidth {
  public:
    explicit LowPass(double relativeCenter) : ConfigCenter(relativeCenter) {}

    inline LowPass &setCenter(double relativeCenter) {
      ConfigCenter::set(relativeCenter);
      return *this;
    }

    inline LowPass &setCenter(Rate rate, double center) {
      ConfigCenter::set(rate, center);
      return *this;
    }

    inline LowPass &setBandWidth(double bandwidth) {
      ConfigBandwidth::set(bandwidth);
      return *this;
    }

    [[nodiscard]] inline FilterType type() const final {
      return FilterType::LOW_PASS;
    }

    template <typename S>
    inline void build(CoefficientsFilter<S> &coefficients) {
      generateLowPass(coefficients, center(), bandwidth());
    }
  };

  class HighPass : public ConfigCenter, public ConfigBandwidth {
  public:
    explicit HighPass(double relativeCenter) : ConfigCenter(relativeCenter) {}

    inline HighPass &setCenter(double relativeCenter) {
      ConfigCenter::set(relativeCenter);
      return *this;
    }

    inline HighPass &setCenter(Rate rate, double center) {
      ConfigCenter::set(rate, center);
      return *this;
    }

    inline HighPass &setBandWidth(double bandwidth) {
      ConfigBandwidth::set(bandwidth);
      return *this;
    }

    [[nodiscard]] inline FilterType type() const final {
      return FilterType::HIGH_PASS;
    }

    template <typename S>
    inline void build(CoefficientsFilter<S> &coefficients) {
      generateHighPass(coefficients, center(), bandwidth());
    }
  };

  static inline Parametric parametric(double relativeCenter) {
    return Parametric(relativeCenter);
  }

  static inline Parametric parametric(Rate rate, double center) {
    return parametric(rate.relative(center));
  }

  static inline HighShelve highShelve(double relativeCenter) {
    return HighShelve(relativeCenter);
  }

  static inline HighShelve highShelve(Rate rate, double center) {
    return highShelve(rate.relative(center));
  }

  static inline LowShelve lowShelve(double relativeCenter) {
    return LowShelve(relativeCenter);
  }

  static inline LowShelve lowShelve(Rate rate, double center) {
    return lowShelve(rate.relative(center));
  }

  static inline BandPass bandPass(double relativeCenter) {
    return BandPass(relativeCenter);
  }

  static inline BandPass bandPass(Rate rate, double center) {
    return bandPass(rate.relative(center));
  }

  static inline HighPass highPass(double relativeCenter) {
    return HighPass(relativeCenter);
  }

  static inline HighPass highPass(Rate rate, double center) {
    return highPass(rate.relative(center));
  }

  static inline HighPass lowPass(double relativeCenter) {
    return HighPass(relativeCenter);
  }

  static inline HighPass lowPass(Rate rate, double center) {
    return lowPass(rate.relative(center));
  }
};
} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_IIR_BIQUAD_H
