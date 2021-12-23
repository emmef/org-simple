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

#include <org-simple/util/Array.h>
#include <org-simple/util/dsp/iir-coefficients.h>
#include <iostream>

namespace {

using namespace org::simple::util::dsp;

static constexpr size_t TEST_SAMPLERATE = 65536;


// TODO: Retrofit to allow for dynamic allocation
template <unsigned ORDER> class FilterScenarioBuffer {
  static_assert(ORDER > 0 && ORDER <= 4);
  static constexpr double RANDOM_SCALE = 1.0;
  static constexpr double EQUAL_DELTA = 1e-10 * RANDOM_SCALE;
  static constexpr size_t MAX_BUFFERS = 10;
  static constexpr const char *BUFFER_NAME[MAX_BUFFERS + 1] = {
      "input",  "output", "misc_1", "misc_2", "misc_3", "misc_4",
      "misc_5", "misc_6", "misc_7", "misc_8", "invalid"};

  using Elements = org::simple::core::SizeMetric::Elements<sizeof(double)>;
  const size_t buffers_;
  const size_t samples_;
  const size_t size_;
  const size_t generateStart_ = ORDER;
  const size_t generateEnd_ = generateStart_ + samples_;
  org::simple::util::ArrayAllocated<double> data_;

  static size_t valid_buffers(size_t buffers) {
    if (buffers > 0 && buffers < 10) {
      return buffers;
    }
    throw std::invalid_argument(
        "FilterScenarioBuffer: number of buffers must be in range [1..10].");
  }

  static unsigned long valid_size(size_t samples) {
    return Elements::Valid::sum(samples, 2lu * ORDER);
  }

  static size_t valid_samples(size_t samples, size_t buffers) {
    if (Elements::IsValid::product(buffers, valid_size(samples))) {
      return samples;
    }
    throw std::invalid_argument(
        "FilterScenarioBuffer: combination of samples and buffers too large.");
  }

  static size_t calculate_size(size_t samples) { return samples + 2 * ORDER; }

  static double random_sample(double scale = 1.0) {
    static constexpr double scale_ = 1.0 / RAND_MAX;

    return scale * scale_ * (2.0 * rand() - RAND_MAX);
  }

  static bool equals(double sample1, double sample2) {
    return fabs(sample1 - sample2) <= EQUAL_DELTA;
  }

  static const char *buffer_name(size_t selector) {
    return BUFFER_NAME[std::min(selector, MAX_BUFFERS)];
  }

  double *get_buffer(size_t selector) {
    if (selector < buffers_) {
      return data_ + selector * size_;;
    }
    throw std::out_of_range("FilterScenarioBuffer: buffer index out of range.");
  }

  const double *get_buffer(size_t selector) const {
    if (selector < buffers_) {
      return data_ + selector * size_;
    }
    throw std::out_of_range("FilterScenarioBuffer: buffer index out of range.");
  }

  static void print_samples(size_t i, double s1, double s2) {
    const char *eq = equals(s1, s2) ? "== " : "!= ";

    std::cout << "\t" << i << ":\t" << s1 << "\t" << eq << s2 << std::endl;
  }

  static void check_compatible_buffers(const FilterScenarioBuffer &first,
                                       const FilterScenarioBuffer &second,
                                       size_t selector) {
    if (first.samples_ != second.samples_) {
      throw std::invalid_argument(
          "FilterScenarioBuffer: buffers of different size used.");
    }
    if (selector >= first.buffers_ || selector >= second.buffers_) {
      throw std::invalid_argument(
          "FilterScenarioBuffer: buffer selector invalid.");
    }
  }

  static void print_both(const FilterScenarioBuffer &first,
                         const FilterScenarioBuffer &second, size_t selector,
                         const char *msg = nullptr) {
    check_compatible_buffers(first, second, selector);
    std::cout << "Comparison of buffers (";
    std::cout << buffer_name(selector);
    if (msg) {
      std::cout << "; " << msg;
    }
    std::cout << ")" << std::endl;
    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout.precision(4);
    const double *p1 = first.get_buffer(selector);
    const double *p2 = second.get_buffer(selector);

    size_t i;
    for (i = 0; i < first.generateStart_; i++) {
      print_samples(i, p1[i], p2[i]);
    }
    std::cout << "\t---" << std::endl;
    for (; i < first.generateEnd_; i++) {
      print_samples(i, p1[i], p2[i]);
    }
    std::cout << "\t---" << std::endl;
    for (; i < first.size_; i++) {
      print_samples(i, p1[i], p2[i]);
    }
    std::cout << "done" << std::endl;
  }

  void zero_buffer(size_t selector) {
    auto dst = get_buffer(selector);
    for (size_t i = 0; i < size_; i++) {
      dst[i] = 0;
    }
  }

public:
  FilterScenarioBuffer(size_t samples, size_t buffers)
      : buffers_(valid_buffers(buffers)),
        samples_(valid_samples(samples, buffers_)), size_(valid_size(samples_)),
        generateStart_(ORDER), generateEnd_(generateStart_ + samples_),
        data_(size_ * buffers_) {
    for (size_t selector = 0; selector < buffers_; selector++) {
      zero_buffer(selector);
    }
  }

  ~FilterScenarioBuffer() { }

  void print_output_comparison(const FilterScenarioBuffer &first,
                               const char *msg) const {
    print_both(*this, first, true, msg);
  }

  void print_input_comparison(const FilterScenarioBuffer &first,
                              const char *msg) const {
    print_both(*this, first, false, msg);
  }

  void copy_from(const FilterScenarioBuffer &source, size_t selector) {
    check_compatible_buffers(*this, source, selector);
    auto dst = get_buffer(selector);
    auto src = source.get_buffer(selector);
    for (size_t i = 0; i < size_; i++) {
      dst[i] = src[i];
    }
  }

  void copy_from_reverse(const FilterScenarioBuffer &source,
                         size_t selector) {
    check_compatible_buffers(*this, source, selector);
    auto dst = get_buffer(selector);
    auto src = source.get_buffer(selector);
    for (size_t i = 0, j = size_ - 1; i < size_; i++, j--) {
      dst[i] = src[j];
    }
  }

  bool equals(const FilterScenarioBuffer &other, size_t selector) {
    auto me = get_buffer(selector);
    auto you = other.get_buffer(selector);
    for (size_t i = 0; i < size_; i++) {
      if (!equals(me[i], you[i])) {
        print_both(*this, other, true, "Equal fail");
        return false;
      }
    }
    return true;
  }

  bool equals_reverse(const FilterScenarioBuffer &other, size_t selector) {
    auto me = get_buffer(selector);
    auto you = other.get_buffer(selector);
    for (size_t i = 0, j = size_ - 1; i < size_; i++, j--) {
      if (!equals(me[i], you[j])) {
        print_both(*this, other, true, "Reverse equal fail");
        return false;
      }
    }
    return true;
  }

  void fill_with_random_zero_padded(size_t selector = 0) {
    auto dst = get_buffer(selector);
    size_t i;
    for (i = 0; i < generateStart_; i++) {
      dst[i] = 0;
    }
    for (; i < generateEnd_; i++) {
      dst[i] = random_sample();
    }
    for (; i < size_; i++) {
      dst[i] = 0;
    }
  }

  void fill_with_random(size_t selector) {
    auto dst = get_buffer(selector);
    for (size_t i = 0; i < size_; i++) {
      dst[i] = random_sample();
    }
  }

  void filter_forward_offs(const FixedOrderCoefficients<double, ORDER> &coeffs_,
                           size_t input_selector, size_t output_selector) {
    zero_buffer(output_selector);
    coeffs_.filter_forward_offs(samples_, get_buffer(input_selector),
                                get_buffer(output_selector));
  }

  void filter_backward_offs(const FixedOrderCoefficients<double, ORDER> &coeffs_,
                            size_t input_selector, size_t output_selector) {
    zero_buffer(output_selector);
    coeffs_.filter_backward_offs(samples_, get_buffer(input_selector) + ORDER,
                                 get_buffer(output_selector) + ORDER);
  }

  void filter_forward_zero(const FixedOrderCoefficients<double, ORDER> &coeffs_,
                           size_t input_selector, size_t output_selector) {
    zero_buffer(output_selector);
    coeffs_.filter_forward_history_zero(samples_,
                                        get_buffer(input_selector) + ORDER,
                                        get_buffer(output_selector) + ORDER);
  }

  void filter_backward_zero(const FixedOrderCoefficients<double, ORDER> &coeffs_,
                            size_t input_selector, size_t output_selector) {
    zero_buffer(output_selector);
    coeffs_.filter_backward_history_zero(samples_,
                                         get_buffer(input_selector) + ORDER,
                                         get_buffer(output_selector) + ORDER);
  }

  void filter_forward_single(const FixedOrderCoefficients<double, ORDER> &coeffs,
                             size_t input_selector, size_t output_selector) {
    double in_history[ORDER];
    double out_history[ORDER];
    double *in = get_buffer(input_selector);
    double *out = get_buffer(output_selector);
    zero_buffer(output_selector);
    // reverse copy (in history, the past has higher memory offset)
    for (size_t i = 0, j = ORDER - 1; i < ORDER; i++, j--) {
      in_history[i] = in[j];
      out_history[i] = out[j];
    }
    for (size_t i = ORDER; i < ORDER + samples_; i++) {
      out[i] = coeffs.filter_single(in_history, out_history, in[i]);
    }
  }

  void generate_random_filter(FixedOrderCoefficients<double, ORDER> &coeffs_) const {
    double scale = 0.45 / ORDER;
    for (size_t i = 0; i <= ORDER; i++) {
      coeffs_.setFB(i, random_sample(scale));
      coeffs_.setFF(i, random_sample(scale));
    }
  }
};

const std::vector<size_t> test_create_periods() {
  std::vector<size_t> v;

  v.emplace_back(4);
  v.emplace_back(8);
  v.emplace_back(16);
  v.emplace_back(64);
  v.emplace_back(512);
  v.emplace_back(2048);

  return v;
}

const std::vector<size_t> &get_test_periods() {
  static std::vector<size_t> v = test_create_periods();

  return v;
}

template <typename C>
C measureFilterGain(const CoefficientsFilter<C> &filter, size_t signal_period, size_t &effective_IR_length) {
  effective_IR_length = effectiveIRLength(filter, TEST_SAMPLERATE, 1e-6);
  size_t settleSamples =
      3 * signal_period +
      signal_period * ((effective_IR_length + signal_period / 2) / signal_period);
  size_t totalSamples = signal_period + 2 * settleSamples;
  std::unique_ptr<double> buffer(new double[totalSamples]);
  FilterHistory<double> history(filter);
  history.zero();
  for (size_t sample = 0; sample < totalSamples; sample++) {
    double input = cos(M_PI * 2 * (sample % signal_period) / signal_period);
    buffer.get()[sample] =
        filter.single(history.inputs(), history.outputs(), input);
  }
  history.zero();
  for (ptrdiff_t sample = totalSamples; sample > 0;) {
    double &p = buffer.get()[--sample];
    p = filter.single(history.inputs(), history.outputs(), p);
  }
  double gainSquared = 0.0;
  double *p = buffer.get() + settleSamples;
  for (size_t sample = 0; sample < signal_period; sample++) {
    gainSquared = std::max(gainSquared, *p++);
  }
  return sqrt(gainSquared);
}

struct FilterScenario {
  FilterType type;
  int order;

  FilterScenario(FilterType t, unsigned o)
  : type(t), order(validated_order(o)) {}

  const char *typeName() const {
    return get_filter_type_name(type);
  }

  virtual const char *typeOfScenario() const = 0;
  virtual void parameters(std::ostream &out) const = 0;

  std::ostream &write(std::ostream &out) const {
    out << typeOfScenario() << "(type=\"" << typeName()
    << "\"; order=" << order;
    parameters(out);
    out << ")";
    return out;
  }

  virtual ~FilterScenario() = default;
};

} // end of anonymous namespace
static std::ostream &operator<<(std::ostream &out,
    const FilterScenario &scenario) {
  scenario.write(out);
  return out;
}


#endif // ORG_SIMPLE_IIR_COEFFICIENTS_TEST_HELPER_H
