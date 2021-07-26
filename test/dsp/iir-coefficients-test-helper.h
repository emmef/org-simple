#ifndef ORG_SIMPLE_IIR_COEFFICIENTS_TEST_HELPER_H
#define ORG_SIMPLE_IIR_COEFFICIENTS_TEST_HELPER_H
/*
 * org-simple/iir-coefficients-test-helper.h
 *
 * Added by michel on 2021-07-26
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

#include <org-simple/dsp/iir-coefficients.h>


namespace {

template <unsigned ORDER>
using Coefficients =
org::simple::dsp::iir::FixedOrderCoefficients<double, ORDER>;

// TODO: Retrofit to allow for dynamic allocation
template <unsigned ORDER, unsigned SAMPLES> class FilterScenarioBuffer {
  static_assert(ORDER > 0 && ORDER <= 4);
  static_assert(SAMPLES > 0);
  static constexpr size_t SIZE = SAMPLES + 2 * ORDER;
  static constexpr size_t GENERATE_START = ORDER;
  static constexpr size_t GENERATE_END = GENERATE_START + SAMPLES;
  static constexpr double RANDOM_SCALE = 1.0;
  static constexpr double EQUAL_DELTA = 1e-10 * RANDOM_SCALE;
  double input_[SIZE];
  double output_[SIZE];

  static double random_sample(double scale = 1.0) {
    static constexpr double scale_ = 1.0 / RAND_MAX;

    return scale * scale_ * (2.0 * rand() - RAND_MAX);
  }

  void zero_output_and_history() {
    for (size_t i = 0; i < SIZE; i++) {
      output_[i] = 0;
    }
  }

  static bool equals(double sample1, double sample2) {
    return fabs(sample1 - sample2) <= EQUAL_DELTA;
  }

  static void print_samples(size_t i, double s1, double s2) {
    const char *eq = equals(s1, s2) ? "== " : "!= ";

    std::cout << "\t" << i << ":\t" << s1 << "\t" << eq << s2 << std::endl;
  }

  static void print_both(const FilterScenarioBuffer &first,
                         const FilterScenarioBuffer &second, bool output,
                         const char *msg = nullptr) {
    std::cout << "Comparison of buffers (";
    std::cout << (output ? "output" : "input");
    if (msg) {
      std::cout << "; " << msg;
    }
    std::cout << ")" << std::endl;
    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout.precision(4);
    const double *p1 = output ? first.output_ : first.input_;
    const double *p2 = output ? second.output_ : second.input_;

    size_t i;
    for (i = 0; i < GENERATE_START; i++) {
      print_samples(i, p1[i], p2[i]);
    }
    std::cout << "\t---" << std::endl;
    for (; i < GENERATE_END; i++) {
      print_samples(i, p1[i], p2[i]);
    }
    std::cout << "\t---" << std::endl;
    for (; i < SIZE; i++) {
      print_samples(i, p1[i], p2[i]);
    }
    std::cout << "done" << std::endl;
  }

public:
  void print_output_comparison(const FilterScenarioBuffer &first,
                               const char *msg) const {
    print_both(*this, first, true, msg);
  }

  void print_input_comparison(const FilterScenarioBuffer &first,
                              const char *msg) const {
    print_both(*this, first, false, msg);
  }

  void copy_from(const FilterScenarioBuffer &source) {
    for (size_t i = 0; i < SIZE; i++) {
      input_[i] = source.input_[i];
    }
  }

  void copy_from_reverse(const FilterScenarioBuffer &source) {
    for (size_t i = 0, j = SIZE - 1; i < SIZE; i++, j--) {
      input_[i] = source.input_[j];
    }
  }

  bool equals(const FilterScenarioBuffer &source) {
    for (size_t i = 0; i < SIZE; i++) {
      if (!equals(output_[i], source.output_[i])) {
        print_both(*this, source, true, "Equal fail");
        return false;
      }
    }
    return true;
  }

  bool equals_input(const FilterScenarioBuffer &source) {
    for (size_t i = 0; i < SIZE; i++) {
      if (!equals(input_[i], source.input_[i])) {
        print_both(*this, source, false, "Equal fail");
        return false;
      }
    }
    return true;
  }

  bool equals_reverse(const FilterScenarioBuffer &source) {
    for (size_t i = 0, j = SIZE - 1; i < SIZE; i++, j--) {
      if (!equals(output_[i], source.output_[j])) {
        print_both(*this, source, true, "Reverse equal fail");
        return false;
      }
    }
    return true;
  }

  bool equals_reverse_input(const FilterScenarioBuffer &source) {
    for (size_t i = 0, j = SIZE - 1; i < SIZE; i++, j--) {
      if (!equals(input_[i], source.input_[j])) {
        print_both(*this, source, false, "Reverse equal fail");
        return false;
      }
    }
    return true;
  }

  void fill_with_random_zero_padded() {
    size_t i;
    for (i = 0; i < GENERATE_START; i++) {
      input_[i] = 0;
    }
    for (; i < GENERATE_END; i++) {
      input_[i] = random_sample();
    }
    for (; i < SIZE; i++) {
      input_[i] = 0;
    }
  }

  void fill_with_random() {
    for (size_t i = 0; i < SIZE; i++) {
      input_[i] = random_sample();
    }
  }

  void filter_forward_offs(const Coefficients<ORDER> &coeffs_) {
    zero_output_and_history();
    coeffs_.filter_forward_offs(SAMPLES, input_, output_);
  }

  void filter_backward_offs(const Coefficients<ORDER> &coeffs_) {
    zero_output_and_history();
    coeffs_.filter_backward_offs(SAMPLES, &input_[ORDER], &output_[ORDER]);
  }

  void filter_forward_zero(const Coefficients<ORDER> &coeffs_) {
    zero_output_and_history();
    coeffs_.filter_forward_history_zero(SAMPLES, &input_[ORDER],
                                        &output_[ORDER]);
  }

  void filter_backward_zero(const Coefficients<ORDER> &coeffs_) {
    zero_output_and_history();
    coeffs_.filter_backward_history_zero(SAMPLES, &input_[ORDER],
                                         &output_[ORDER]);
  }

  void filter_forward_single(const Coefficients<ORDER> &coeffs) {
    double in_history[ORDER];
    double out_history[ORDER];
    zero_output_and_history();
    // reverse copy (in history, the past has higher memory offset)
    for (size_t i = 0, j = ORDER - 1; i < ORDER; i++, j--) {
      in_history[i] = input_[j];
      out_history[i] = output_[j];
    }
    for (size_t i = ORDER; i < ORDER + SAMPLES; i++) {
      output_[i] = coeffs.filter_single(in_history, out_history, input_[i]);
    }
  }

  void generate_random(Coefficients<ORDER> &coeffs_) const {
    double scale = 0.45 / ORDER;
    for (size_t i = 0; i <= ORDER; i++) {
      coeffs_.setFB(i, random_sample(scale));
      coeffs_.setFF(i, random_sample(scale));
    }
  }

  FilterScenarioBuffer() { zero_output_and_history(); }
};

} // end of anonymous namespace

#endif // ORG_SIMPLE_IIR_COEFFICIENTS_TEST_HELPER_H
