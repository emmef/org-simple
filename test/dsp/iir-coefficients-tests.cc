//
// Created by michel on 11-04-21.
//

#include <cstdio>
#include <org-simple/dsp/iir-coefficients.h>
#include <test-helper.h>

template <unsigned ORDER>
using Coefficients =
    org::simple::dsp::iir::FixedOrderCoefficients<double, ORDER>;

namespace {

template <unsigned ORDER, unsigned SAMPLES> class SampleBufferWithFilter {
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

  static void print_both(const SampleBufferWithFilter &first,
                         const SampleBufferWithFilter &second, bool output,
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
  void print_output_comparison(const SampleBufferWithFilter &first,
                               const char *msg) const {
    print_both(*this, first, true, msg);
  }

  void print_input_comparison(const SampleBufferWithFilter &first,
                              const char *msg) const {
    print_both(*this, first, false, msg);
  }

  void copy_from(const SampleBufferWithFilter &source) {
    for (size_t i = 0; i < SIZE; i++) {
      input_[i] = source.input_[i];
    }
  }

  void copy_from_reverse(const SampleBufferWithFilter &source) {
    for (size_t i = 0, j = SIZE - 1; i < SIZE; i++, j--) {
      input_[i] = source.input_[j];
    }
  }

  bool equals(const SampleBufferWithFilter &source) {
    for (size_t i = 0; i < SIZE; i++) {
      if (!equals(output_[i], source.output_[i])) {
        print_both(*this, source, true, "Equal fail");
        return false;
      }
    }
    return true;
  }

  bool equals_reverse(const SampleBufferWithFilter &source) {
    for (size_t i = 0, j = SIZE - 1; i < SIZE; i++, j--) {
      if (!equals(output_[i], source.output_[j])) {
        print_both(*this, source, true, "Reverse equal fail");
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

  SampleBufferWithFilter() { zero_output_and_history(); }
};

BOOST_AUTO_TEST_SUITE(org_simple_dsp_iir_Coefficients)

BOOST_AUTO_TEST_CASE(testFilterSingleEqualsForwardWithOffset) {
  static constexpr size_t SIZE = 10;
  SampleBufferWithFilter<2, SIZE> filter1;
  SampleBufferWithFilter<2, SIZE> filter2;
  Coefficients<2> coeffs;
  filter1.generate_random(coeffs);

  filter1.fill_with_random();
  filter2.copy_from(filter1);
  filter1.filter_forward_offs(coeffs);
  filter2.filter_forward_single(coeffs);

  BOOST_CHECK(filter1.equals(filter2));
}

BOOST_AUTO_TEST_CASE(testForwardEqualsBackwardBothWithOffset) {
  static constexpr size_t SIZE = 10;
  SampleBufferWithFilter<2, SIZE> filter1;
  SampleBufferWithFilter<2, SIZE> filter2;
  Coefficients<2> coeffs;
  filter1.generate_random(coeffs);

  filter1.fill_with_random();
  filter2.copy_from_reverse(filter1);

  filter1.filter_forward_offs(coeffs);
  filter2.filter_backward_offs(coeffs);

  BOOST_CHECK(filter1.equals_reverse(filter2));
}

BOOST_AUTO_TEST_CASE(testForwardEqualsBackwardBothWithZeroHistory) {
  static constexpr size_t SIZE = 10;
  SampleBufferWithFilter<2, SIZE> filter1;
  SampleBufferWithFilter<2, SIZE> filter2;
  Coefficients<2> coeffs;
  filter1.generate_random(coeffs);

  filter1.fill_with_random();
  filter2.copy_from_reverse(filter1);

  filter1.filter_forward_zero(coeffs);
  filter2.filter_backward_zero(coeffs);

  BOOST_CHECK(filter1.equals_reverse(filter2));
}


BOOST_AUTO_TEST_SUITE_END()

} // namespace

// TODO Test coverage!
