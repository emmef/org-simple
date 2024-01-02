#ifndef ORG_SIMPLE_DSP_M_BIQUAD_H
#define ORG_SIMPLE_DSP_M_BIQUAD_H
/*
 * org-simple/dsp/Biquad.h
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
#include <cmath>

namespace org::simple::dsp {

struct [[maybe_unused]] BiQuad {
  /**
   * Holds the history (past input and output values) to implement a biquad
   * filter.
   * @tparam V The value type that can be floating point, but also a numerical
   * vector or something.
   */
  template <class V> struct History {
    V x1 = 0;
    V x2 = 0;
    V y1 = 0;
    V y2 = 0;

    inline void push(const V &x, const V &y) {
      x2 = x1;
      x1 = x;
      y2 = y1;
      y1 = y;
    }

    inline void pushYInX2(const V &x) {
      y2 = y1;
      y1 = x2;
      x2 = x1;
      x1 = x;
    }

    inline void zero() { *this = {}; }
  };

  /**
   * Defines the Biquad IIR (infinite impulse response) filter coefficients.
   * The default constructed value implements an identity filter.
   * @tparam F The floating-point type of coefficient values.
   */
  template <class F> struct Coefficients {
    static_assert(std::is_floating_point_v<F>);
    F a1 = 0;
    F a2 = 0;
    F b0 = 1;
    F b1 = 0;
    F b2 = 0;

    template <class V>
    [[maybe_unused]] void run(History<V> &h, const V &x, V &y) const {
      y = b0 * x + b1 * h.x1 + b2 * h.x2 + a1 * h.y1 + a2 * h.y2;

      h.push(x, y);
    }

    template <class V>
    [[maybe_unused]] V runAndGet(History<V> &h, const V &x) const {
      h.x2 = b0 * x + b1 * h.x1 + b2 * h.x2 + a1 * h.y1 + a2 * h.y2;

      h.pushYInX2(x);
      return h.y1;
    }

    /**
     * Applies the filter implemented by the coefficients using a block of input
     * samples and a block of output samples. The history is assumed zero.
     * @tparam V The value type that can be floating point, but also a numerical
     * vector or something.
     * @param x The block of input values with a length of at least len.
     * @param y The block of output values with a length of at least len.
     * @param len The number of samples to process.
     * @return The first processed output sample in y.
     */
    template <class V>
    [[maybe_unused]] void apply(const V *__restrict x, V *__restrict y,
                                const size_t len) const {

      y[0] = b0 * x[0];
      y[1] = b0 * x[1] + b1 * x[0] + a1 * y[0];
      if (len < 2) {
        return;
      }
      const V *input = x;
      const V *const end = input + len - 2;
      V *output = y;
      while (input < end) {
        output[2] = b0 * input[2] + b1 * input[1] + b2 * input[0] +
                    a1 * output[1] + a2 * output[0];
        input++;
        output++;
      }
    }

    /**
     * Applies the filter implemented by the coefficients backwards, using a
     * block of input samples and a block of output samples. The history is
     * assumed zero.
     * @tparam V The value type that can be floating point, but also a numerical
     * vector or something.
     * @param x The block of input values with a length of at least len.
     * @param y The block of output values with a length of at least len.
     * @param len The number of samples to process.
     * @return The first processed output sample in y.
     */
    template <class V>
    [[maybe_unused]] void applyBackwards(const V *__restrict x, V *__restrict y,
                                         const size_t len) const {
      const size_t offs0 = len - 1;
      y[offs0] = b0 * x[offs0];
      const size_t offs1 = offs0 - 1;
      y[offs1] = b0 * x[offs1] + b1 * x[offs0] + a1 * y[offs0];
      if (len < 2) {
        return;
      }
      const V *input = x + offs1 - 1;
      V *output = y + offs1 - 1;
      while (input >= x) {
        output[0] = b0 * input[0] + b1 * input[1] + b2 * input[2] +
                    a1 * output[1] + a2 * output[2];
        input--;
        output--;
      }
    }

    /**
     * Applies the filter implemented by the coefficients using a block of input
     * samples and a block of output samples. The history is provided and kept
     * by the history parameter.
     * @tparam V The value type that can be floating point, but also a numerical
     * vector or something.
     * @param x The block of input values with a length of at least len.
     * @param y The block of output values with a length of at least len.
     * @param len The number of samples to process.
     * @param history The history that contains initial history and the history
     * after.
     */
    template <class V>
    [[maybe_unused]] void applyWithHistory(const V *__restrict x,
                                           V *__restrict y, const size_t len,
                                           History<V> &history) const {

      const V *input = x;
      V *output = y;
      const size_t smallCount = std::min(static_cast<size_t>(2u), len);
      for (size_t i = 0; i < smallCount; i++) {

        output[i] = b0 * input[i] + b1 * history.x1 + b2 * history.x2 +
                    a1 * history.y1 + a2 * history.y2;
        history.push(input[i], output[i]);
      }
      if (len > 2) {
        const V *const end = input + len - 2;

        while (input < end) {
          output[2] = b0 * input[2] + b1 * input[1] + b2 * input[0] +
                      a1 * output[1] + a2 * output[0];
          input++;
          output++;
        }
        history.push(input[0], output[0]);
        history.push(input[1], output[1]);
      }
    }
  };

  struct Butterworth {

    /**
     * Calculates the bandwidth/Q parameter so that the biquad filter emulates a
     * butterworth second order filter with high accuracy.
     * @param frequency The cutoff frequency
     * @param sampleRate The sample rate
     * @return The necessary bandwidth parameter.
     */
    static inline float butterworthBandwidth(const double frequency,
                                             const double sampleRate) {
      static constexpr double minRatio = 1e-10;
      static constexpr double minFs = 1;
      static constexpr double fudgeFrequencyFactor = 0.311971724033356;
      static constexpr double fudgeLowCorrection = 1.209553281779139;

      const double c =
          std::max(frequency / std::max(static_cast<double>(sampleRate), minFs),
                   minRatio);
      const double x = fudgeFrequencyFactor / c;
      return static_cast<float>(fudgeLowCorrection * atan(x * x));
    }

    /**
     * Configures the coefficients to implement a low-pass Butterworth filter.
     * @tparam F The coefficient value type.
     * @param coefficients The coefficients that implement the filter.
     * @param frequency The cutoff frequency in Hz.
     * @param sampleRate The sample rate in Hz.
     */
    template <class F>
    [[maybe_unused]] static void
    configureHighPass(Coefficients<F> &coefficients, const double sampleRate,
                      const double frequency) {
      auto fc = static_cast<double>(frequency);
      auto fs = static_cast<double>(sampleRate);
      const double bandwidth = butterworthBandwidth(fc, sampleRate);
      double omega = 2.0 * M_PI * fc / fs;
      double sn = sin(omega);
      double cs = cos(omega);
      double alpha = sn * sinh((M_LN2 / 2.0) * bandwidth * omega / sn);

      const double a0R = 1.0 / (1.0 + alpha);

      coefficients.b0 = static_cast<F>(a0R * (1.0 - cs) * 0.5);
      coefficients.b1 = static_cast<F>(a0R * (1.0 - cs));
      coefficients.b2 = static_cast<F>(a0R * (1.0 - cs) * 0.5);
      coefficients.a1 = static_cast<F>(a0R * (2.0 * cs));
      coefficients.a2 = static_cast<F>(a0R * (alpha - 1.0));
    }

    /**
     * Configures the coefficients to implement a high-pass Butterworth filter.
     * @tparam F The coefficient value type.
     * @param coefficients The coefficients that implement the filter.
     * @param frequency The cutoff frequency in Hz.
     * @param sampleRate The sample rate in Hz.
     */
    template <class F>
    [[maybe_unused]] static void configureLowPass(Coefficients<F> &coefficients,
                                                  const double sampleRate,
                                                  const double frequency) {
      const auto fs = static_cast<double>(sampleRate);
      const double bandwidth = butterworthBandwidth(frequency, sampleRate);
      double omega = 2.0 * M_PI * frequency / fs;
      double sn = sin(omega);
      double cs = cos(omega);
      double alpha = sn * sinh(M_LN2 / 2.0 * bandwidth * omega / sn);

      const double a0R = 1.0 / (1.0 + alpha);

      coefficients.b0 = static_cast<F>(a0R * (1.0 + cs) * 0.5);
      coefficients.b1 = static_cast<F>(a0R * -(1.0 + cs));
      coefficients.b2 = static_cast<F>(a0R * (1.0 + cs) * 0.5);
      coefficients.a1 = static_cast<F>(a0R * (2.0 * cs));
      coefficients.a2 = static_cast<F>(a0R * (alpha - 1.0));
    }

    template <class F>
    [[maybe_unused]] Coefficients<F> createLowPass(const double sampleRate,
                                                   const double frequency) {
      Coefficients<F> coefficients;
      configureLowPass(coefficients, sampleRate, frequency);
      return coefficients;
    }

    template <class F>
    [[maybe_unused]] Coefficients<F> createHighPass(const double sampleRate,
                                                    const double frequency) {
      Coefficients<F> coefficients;
      configureHighPass(coefficients, sampleRate, frequency);
      return coefficients;
    }
  };

  struct [[maybe_unused]] Parametric {
    template <class F>
    [[maybe_unused]] static void
    configure(Coefficients<F> &result, const double sampleRate,
              const double centerFrequency, const double gain,
              const double bandwidth) {
      double omega = 2.0 * M_PI * centerFrequency / sampleRate;
      double cw = cos(omega);
      double sw = sin(omega);
      double j = sqrt(gain);
      double g = sw * sinh((M_LN2 / 2.0) * bandwidth * omega / sw);
      double a0R = 1.0f / (1.0f + (g / j));

      result.b0 = static_cast<F>((1.0f + (g * j)) * a0R);
      result.b1 = static_cast<F>((-2.0f * cw) * a0R);
      result.b2 = static_cast<F>((1.0f - (g * j)) * a0R);
      result.a1 = -result.C1;
      result.a2 = static_cast<F>(((g / j) - 1.0f) * a0R);

      return result;
    }
    template <class F>
    [[maybe_unused]] static void
    create(const double sampleRate,
           const double centerFrequency, const double gain,
           const double bandwidth) {
      Coefficients<F> coefficients;
      configure(coefficients, sampleRate, centerFrequency, gain, bandwidth);
    }
  };
};

} // namespace org::simple::dsp

#endif // ORG_SIMPLE_DSP_M_BIQUAD_H
