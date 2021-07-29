//
// Created by michel on 27-04-21.
//

#include "test-helper.h"
#include <boost/math/special_functions/relative_difference.hpp>
#include <cmath>
#include <org-simple/dsp/iir-butterworth.h>
#include <vector>

#include "iir-coefficients-test-helper.h"

using namespace org::simple::dsp::iir;

static constexpr size_t SAMPLERATE = 65536;

const std::vector<size_t> create_periods() {
  std::vector<size_t> v;

  v.emplace_back(4);
  v.emplace_back(8);
  v.emplace_back(16);
  v.emplace_back(64);
  v.emplace_back(512);
  v.emplace_back(2048);

  return v;
}

const std::vector<size_t> &get_periods() {
  static std::vector<size_t> v = create_periods();

  return v;
}

struct FilterGainScenario {
  size_t signal_period;
  size_t filter_period;
  FilterType type;
  unsigned order;

  const char *typeName() const {
    switch (type) {
    case FilterType::HIGH_PASS:
      return "highpass";
    case FilterType::LOW_PASS:
      return "lowpass";
    default:
      return "unknown";
    }
  }
  FilterGainScenario(size_t sp, size_t fp, FilterType t, unsigned o)
      : signal_period(sp), filter_period(fp), type(t), order(o) {}
};

static std::ostream &operator<<(std::ostream &out,
                                const FilterGainScenario &scenario) {
  out << "FilterGainScenario(type=" << scenario.typeName()
      << "; order=" << scenario.order
      << "; signal_period=" << scenario.signal_period
      << "; filter_period=" << scenario.filter_period;
  return out;
}

const std::vector<FilterGainScenario> getFilterGainScenarios() {
  std::vector<FilterGainScenario> result;
  for (size_t signal_period : get_periods()) {
    for (size_t filter_period : get_periods()) {
      result.emplace_back(FilterGainScenario(signal_period, filter_period,
                                             FilterType::LOW_PASS, 1));
      result.emplace_back(FilterGainScenario(signal_period, filter_period,
                                             FilterType::HIGH_PASS, 1));
      result.emplace_back(FilterGainScenario(signal_period, filter_period,
                                             FilterType::LOW_PASS, 2));
      result.emplace_back(FilterGainScenario(signal_period, filter_period,
                                             FilterType::HIGH_PASS, 2));
      result.emplace_back(FilterGainScenario(signal_period, filter_period,
                                             FilterType::LOW_PASS, 4));
      result.emplace_back(FilterGainScenario(signal_period, filter_period,
                                             FilterType::HIGH_PASS, 4));
    }
  }
  return result;
}

template <size_t ORDER>
bool verifyGain(size_t signal_period, size_t filter_period, FilterType type,
                double &measured, double &calculated) {
  double filterRelativeFrequency = filter_period > 2 ? 1.0 / filter_period : 0.45;
  double signalRelativeFrequency = 1.0 / signal_period;
  size_t min_period = std::min(signal_period, filter_period);
  double error = 1.0 / (1.0 - 1.0 / min_period);
  FixedOrderCoefficients<double, ORDER> filter;
  Butterworth::create(filter, filterRelativeFrequency, type, 1.0);
  size_t responseSamples = effectiveIRLength(filter, SAMPLERATE, 1e-6);
  size_t settleSamples =
      3 * signal_period +
      signal_period * ((responseSamples + signal_period / 2) / signal_period);
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
  double gainSquared = 0;
  double *p = buffer.get() + settleSamples;
  for (size_t sample = 0; sample < signal_period; sample++) {
    gainSquared = std::max(gainSquared, *p++);
  }
  measured = sqrt(gainSquared);
  calculated = get_bw_gain(type, ORDER, signalRelativeFrequency / filterRelativeFrequency);
  double minGain = std::min(measured, calculated) * error;
  if (gainSquared < minGain && calculated < minGain) {
    return true;
  }
  return false;
}

static double reference_high_pass_gain(size_t order, double rel) {
  double alpha = pow(fabs(rel), order);
  return alpha / sqrt(1.0 + alpha * alpha);
}

static double reference_low_pass_gain(size_t order, double rel) {
  double alpha2 = pow(fabs(rel), order * 2);
  return 1.0 / sqrt(1.0 + alpha2);
}

struct GainScenario {
  FilterType type;
  size_t order;
  double relative;
  double expected_gain;
  bool ref = false;

  double actual() const {
    if (!ref) {
      switch (type) {
      case FilterType::LOW_PASS:
        return get_bw_low_pass_gain(order, relative);
      case FilterType::HIGH_PASS:
        return get_bw_high_pass_gain(order, relative);
      default:
        return std::numeric_limits<double>::quiet_NaN();
      }
    } else {
      switch (type) {
      case FilterType::LOW_PASS:
        return reference_low_pass_gain(order, relative);
      case FilterType::HIGH_PASS:
        return reference_high_pass_gain(order, relative);
      default:
        return std::numeric_limits<double>::quiet_NaN();
      }
    }
  }
};

static std::ostream &operator<<(std::ostream &out,
                                const GainScenario &scenario) {
  out << "GainScenario(type=";
  switch (scenario.type) {
  case FilterType::LOW_PASS:
    out << "low-pass";
    break;
  case FilterType::HIGH_PASS:
    out << "high-pass";
    break;
  default:
    out << "invalid";
  }
  out << "; order=" << scenario.order;
  out << "; w/w0=" << scenario.relative;
  out << "; gain=" << scenario.expected_gain;
  out << "; reference=" << scenario.ref;

  return out;
}

std::vector<GainScenario> createTestScenarios() {
  std::vector<GainScenario> scenarios;

  scenarios.push_back({FilterType::LOW_PASS, 1, 0, 1, true});
  scenarios.push_back({FilterType::HIGH_PASS, 1, 0, 0, true});
  scenarios.push_back({FilterType::LOW_PASS, 1,
                       std::numeric_limits<double>::epsilon(), 1, true});
  scenarios.push_back({FilterType::HIGH_PASS, 1,
                       1.0 / std::numeric_limits<double>::epsilon(), 1, true});

  for (size_t order = 1; is_valid_bw_order(order); order++) {
    scenarios.push_back({FilterType::LOW_PASS, order, 1.0, M_SQRT1_2, true});
    scenarios.push_back({FilterType::HIGH_PASS, order, 1.0, M_SQRT1_2, true});
  }

  for (size_t order = 1; is_valid_bw_order(order); order++) {
    for (double relative = 0.125; relative <= 8; relative *= 2) {
      scenarios.push_back({FilterType::LOW_PASS, order, relative,
                           reference_low_pass_gain(order, relative)});
      scenarios.push_back({FilterType::HIGH_PASS, order, relative,
                           reference_high_pass_gain(order, relative)});
    }
  }

  return scenarios;
}

static bool same(double x, double y) {
  return boost::math::relative_difference(x, y) <= 1e-11;
}

BOOST_AUTO_TEST_SUITE(org_simple_dsp_iir_butterworth_tests)

BOOST_DATA_TEST_CASE(sample, createTestScenarios()) {
  if (!same(sample.expected_gain, sample.actual())) {
    BOOST_CHECK_EQUAL(sample.expected_gain, sample.actual());
    double g1 = get_bw_low_pass_gain(sample.order, sample.relative);
    double g2 = reference_low_pass_gain(sample.order, sample.relative);
    BOOST_CHECK_EQUAL(g1, g2);
  } else {
    BOOST_CHECK(same(sample.expected_gain, sample.actual()));
  }
}

BOOST_AUTO_TEST_CASE(testSupportedFilterTypes) {
  BOOST_CHECK(!is_supported_bw_type(FilterType::ALL_PASS));
  BOOST_CHECK(is_supported_bw_type(FilterType::LOW_PASS));
  BOOST_CHECK(!is_supported_bw_type(FilterType::LOW_SHELVE));
  BOOST_CHECK(!is_supported_bw_type(FilterType::BAND_PASS));
  BOOST_CHECK(!is_supported_bw_type(FilterType::PARAMETRIC));
  BOOST_CHECK(!is_supported_bw_type(FilterType::HIGH_SHELVE));
  BOOST_CHECK(is_supported_bw_type(FilterType::HIGH_PASS));
}

BOOST_DATA_TEST_CASE(testFirstOrderHighPass, getFilterGainScenarios()) {
  double actualGain = 0.0;
  double calculatedGain = 0.0;
  if (verifyGain<1>(sample.signal_period, sample.filter_period, sample.type,
                    actualGain, calculatedGain)) {
    BOOST_CHECK(true);
  } else {
    BOOST_CHECK_EQUAL(actualGain, calculatedGain);
  }
}

#ifdef ORG_SIMPLE_IIR_BUTTERWORTH_PRINT_FILTER_LENGTHS

template <unsigned ORDER>
void printFilterLength(double hz, unsigned short bits) {
  static constexpr double rate = 96000;
  FixedOrderCoefficients<double, ORDER> coefficients;
  double f = hz / rate;
  double err = pow(0.5, bits);
  Butterworth::create(coefficients, f, FilterType::LOW_PASS, 1.0);
  size_t length = effectiveIRLength(coefficients, 1048576, err);
  std::cout << "Impulse response length of low pass butterworth (order" << ORDER
            << ") @ " << hz << " Hz. (" << f << ") for " << bits
            << " bits accuracy is " << length << " samples \t"
            << (1.0 * length / rate) << " seconds \t" << (f * length)
            << " periods." << std::endl;
}

BOOST_AUTO_TEST_CASE(estimateFilterLength) {
  for (double hz = 40; hz < 12000; hz *= 2) {
    printFilterLength<1>(hz, 16);
    printFilterLength<1>(hz, 24);
    printFilterLength<1>(hz, 25);
    printFilterLength<2>(hz, 16);
    printFilterLength<2>(hz, 24);
    printFilterLength<2>(hz, 25);
    printFilterLength<4>(hz, 16);
    printFilterLength<4>(hz, 24);
    printFilterLength<4>(hz, 25);
  }
}

#endif

BOOST_AUTO_TEST_SUITE_END()
