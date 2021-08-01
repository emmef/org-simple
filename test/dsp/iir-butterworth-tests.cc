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

namespace {
struct FilterScenario {
  FilterType type;
  int order;

  FilterScenario(FilterType t, unsigned o)
      : type(t), order(validated_order(o)) {}

  const char *typeName() const {
    return Butterworth::getTypeName(type);
  }

  virtual const char *typeOfScenario() const = 0;
  virtual std::ostream &parameters(std::ostream &out) const = 0;

  std::ostream &write(std::ostream &out) const {
    out << typeOfScenario() << "(type=\"" << typeName()
        << "\"; order=" << order;
    parameters(out);
    out << ")";
    return out;
  }
};

} // end of anonymous namespace

static std::ostream &operator<<(std::ostream &out,
                                const FilterScenario &scenario) {
  scenario.write(out);
  return out;
}

struct FilterGainScenario : public FilterScenario {
  size_t signal_period;
  size_t filter_period;

  const char *typeOfScenario() const override { return "FilterGainScenario"; }
  std::ostream &parameters(std::ostream &out) const override {
    out << "; signal_period=" << signal_period
        << "; filter_period=" << filter_period;
    return out;
  }

  FilterGainScenario(size_t sp, size_t fp, FilterType t, unsigned o)
      : FilterScenario(t, o), signal_period(sp), filter_period(fp) {}
};

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
  double filterRelativeFrequency =
      filter_period > 2 ? 1.0 / filter_period : 0.45;
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
  calculated = Butterworth::getGain(type, ORDER,
                           signalRelativeFrequency / filterRelativeFrequency);
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

struct GainScenario : public FilterScenario {
  double relative;
  double expected_gain;
  bool ref;

  double actual() const {
    if (!ref) {
      switch (type) {
      case FilterType::LOW_PASS:
        return Butterworth::getLowPassGain(order, relative, false);
      case FilterType::HIGH_PASS:
        return Butterworth::getHighPassGain(order, relative, false);
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

  GainScenario(FilterType t, unsigned o, double rel, double expGain, bool isRef = false)
      : FilterScenario(t, o), relative(rel), expected_gain(expGain),
        ref(isRef) {}

  const char *typeOfScenario() const override { return "GainCalculationScenario"; }
  std::ostream &parameters(std::ostream &out) const override {
    return out << "; relative-frequency=" << relative
        << "; expected gain=" << expected_gain
        << "; reference=" << (ref ? "true" : "false");
  }
};

// static std::ostream &operator<<(std::ostream &out,
//                                 const GainScenario &scenario) {
//   out << "GainScenario(type=";
//   switch (scenario.type) {
//   case FilterType::LOW_PASS:
//     out << "low-pass";
//     break;
//   case FilterType::HIGH_PASS:
//     out << "high-pass";
//     break;
//   default:
//     out << "invalid";
//   }
//   out << "; order=" << scenario.order;
//   out << "; w/w0=" << scenario.relative;
//   out << "; gain=" << scenario.expected_gain;
//   out << "; reference=" << scenario.ref;
//
//   return out;
// }

std::vector<GainScenario> createTestScenarios() {
  std::vector<GainScenario> scenarios;

  scenarios.emplace_back(GainScenario(FilterType::LOW_PASS, 1, 0, 1, true));
  scenarios.emplace_back(GainScenario(FilterType::HIGH_PASS, 1, 0, 0, true));
  scenarios.emplace_back(GainScenario(FilterType::LOW_PASS, 1,
                                      std::numeric_limits<double>::epsilon(), 1,
                                      true));
  scenarios.emplace_back(
      GainScenario(FilterType::HIGH_PASS, 1,
                   1.0 / std::numeric_limits<double>::epsilon(), 1, true));

  for (unsigned order = 1; Butterworth::isValidOrder(order); order++) {
    scenarios.emplace_back(
        GainScenario(FilterType::LOW_PASS, order, 1.0, M_SQRT1_2, true));
    scenarios.emplace_back(
        GainScenario(FilterType::HIGH_PASS, order, 1.0, M_SQRT1_2, true));
  }

  for (unsigned order = 1; Butterworth::isValidOrder(order); order++) {
    for (double relative = 0.125; relative <= 8; relative *= 2) {
      scenarios.emplace_back(
          GainScenario(FilterType::LOW_PASS, order, relative,
                       reference_low_pass_gain(order, relative)));
      scenarios.emplace_back(
          GainScenario(FilterType::HIGH_PASS, order, relative,
                       reference_high_pass_gain(order, relative)));
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
    double g1 = Butterworth::getLowPassGain(sample.order, sample.relative, false);
    double g2 = reference_low_pass_gain(sample.order, sample.relative);
    BOOST_CHECK_EQUAL(g1, g2);
  } else {
    BOOST_CHECK(same(sample.expected_gain, sample.actual()));
  }
}

BOOST_AUTO_TEST_CASE(testSupportedFilterTypes) {
  BOOST_CHECK(!Butterworth::isValidFilterType(FilterType::ALL_PASS));
  BOOST_CHECK(Butterworth::isValidFilterType(FilterType::LOW_PASS));
  BOOST_CHECK(!Butterworth::isValidFilterType(FilterType::LOW_SHELVE));
  BOOST_CHECK(!Butterworth::isValidFilterType(FilterType::BAND_PASS));
  BOOST_CHECK(!Butterworth::isValidFilterType(FilterType::PARAMETRIC));
  BOOST_CHECK(!Butterworth::isValidFilterType(FilterType::HIGH_SHELVE));
  BOOST_CHECK(Butterworth::isValidFilterType(FilterType::HIGH_PASS));
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
void printFilterLength(double hz, unsigned short bits, FilterType type) {
  static constexpr double rate = 96000;
  static constexpr size_t MAX_LENGTH = 1048576;
  FixedOrderCoefficients<double, ORDER> coefficients;
  double f = hz / rate;
  double err = pow(0.5, bits);
  Butterworth::create(coefficients, f, FilterType::LOW_PASS, 1.0);
  size_t length = effectiveIRLength(coefficients, MAX_LENGTH, err);
  std::cout << "Impulse response length of low pass butterworth (order" << ORDER
            << ") @ " << hz << " Hz. (" << f << ") for " << bits
            << " bits accuracy is " << length << " samples \t"
            << (1.0 * length / rate) << " seconds \t" << (f * length)
            << " periods." << std::endl;
}

BOOST_AUTO_TEST_CASE(estimateFilterLength) {
  FilterType types[2] = { FilterType::LOW_PASS, FilterType::HIGH_PASS };

  for (FilterType type : types) {
    printFilterLength<1>(0, 24, type);
    printFilterLength<2>(0, 24, type);
    printFilterLength<4>(0, 24, type);
    for (double hz = 40; hz < 12000; hz *= 2) {
      printFilterLength<1>(hz, 24, type);
      printFilterLength<2>(hz, 24, type);
      printFilterLength<4>(hz, 24, type);
    }
    printFilterLength<2>(80, 15, type);
    printFilterLength<2>(80, 23, type);
    printFilterLength<2>(80, 31, type);
  }
}

#endif

BOOST_AUTO_TEST_SUITE_END()
