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

const std::vector<size_t> create_dividers() {
  std::vector<size_t> v;

  v.emplace_back(2);
  v.emplace_back(4);
  v.emplace_back(8);
  v.emplace_back(16);
  v.emplace_back(20);
  v.emplace_back(24);

  return v;
}

const std::vector<size_t> &get_dividers() {
  static std::vector<size_t> v = create_dividers();

  return v;
}

static size_t get_max_divider() {
  size_t m = 0;
  for (auto f : get_dividers()) {
    m = std::max(m, f);
  }
  return m;
}

template <typename S>
size_t
effectiveIRLength(const org::simple::dsp::iir::CoefficientsFilter<S> &filter,
                  size_t maxLength, double errorRms,
                  double errorIntegratedAmplitude) {
  if (errorRms < 0 && errorIntegratedAmplitude < 0) {
    throw std::invalid_argument(
        "Need at least one positive epsilon (RMS or integrated amplitude)");
  }
  size_t order = filter.getOrder();
  // Create history
  S history[2 * order];
  S *ffh = &history[0];
  S *fbh = &history[order];
  // Zero history
  for (size_t i = 0; i < order; i++) {
    ffh[i] = 0;
    fbh[i] = 0;
  }
  // check if filter outputs anything in the first N samples, where N is the
  // order. If not: there will be no further output.
  S output = filter.single(ffh, fbh, 1.0);
  S totalRms = output * output;
  for (size_t i = 0; i <= order; i++) {
    output = filter.single(ffh, fbh, 0.0);
    totalRms += output * output;
  }
  if (totalRms < std::numeric_limits<double>::min()) {
    return 0;
  }
  size_t minSamples = org::simple::core::Bits<size_t>::fill(order) / 2 + 1;
  size_t maxSamples =
      std::min(std::max(minSamples, maxLength),
               std::numeric_limits<size_t>::max() / 2 / sizeof(S));
  size_t maxBlockEnd = org::simple::core::Bits<size_t>::fill(maxSamples) + 1;

  std::cout << "Sample ranges: min=" << minSamples << "; max=" << maxSamples
            << std::endl;

  // Zero history
  for (size_t i = 0; i < order; i++) {
    ffh[i] = 0;
    fbh[i] = 0;
  }
  // square RMS epsilon
  double energyEpsilonSquared = errorRms * errorRms;
  // start iterating
  bool sigRms = true;
  bool sigIntegrated = true;
  double totalIntegrated = 0;
  totalRms = 0;
  output = filter.single(ffh, fbh, 1.0);
  //  std::cout << "\t" << 0 << "\t" << output << std::endl;
  double blockRms = output * output;
  double blockIntegrated = fabs(output);
  size_t blockEnd = 1;
  while (blockEnd <= maxBlockEnd &&
         (blockEnd < minSamples || sigRms || sigIntegrated)) {
    size_t blockStart = blockEnd;
    blockEnd += blockStart;
    for (size_t i = blockStart; i < blockEnd; i++) {
      output = filter.single(ffh, fbh, 0.0);
      //      std::cout << "\t" << i << "\t" << output << std::endl;
      blockRms += output * output;
      blockIntegrated += fabs(output);
    }
    std::cout << "[" << blockStart << ".." << blockEnd << "]: Rms=" << blockRms
              << " (" << sqrt(blockRms / totalRms)
              << "); int=" << blockIntegrated << " ("
              << blockIntegrated / totalIntegrated << ")" << std::endl;
    sigRms = errorRms > 0 && blockRms > energyEpsilonSquared * totalRms;
    sigIntegrated =
        errorIntegratedAmplitude > 0 &&
        blockIntegrated > errorIntegratedAmplitude * totalIntegrated;
    if (!(sigRms || sigIntegrated)) {
      break;
    }
    totalRms += blockRms;
    totalIntegrated += blockIntegrated;
    blockRms = 0;
    blockIntegrated = 0;
  }
  // It is possible to backtrack in the previous last block for more accurate
  // measurement.
  std::cout << "IIR length=" << blockEnd << "; rms-err=" << sigRms
            << "; amp-err=" << sigIntegrated << std::endl;
  return blockEnd;
}

// class DoubleFilterMetrics {
//   size_t order_;
//
// private:
//   size_t max_divider_;
//   size_t response_periods_;
//   size_t data_length_;
//
// public:
//   static constexpr double ERROR = 1e-3;
//   static constexpr size_t HEADROOM = 2;
//
//   DoubleFilterMetrics(unsigned order) : order_(order) {
//     response_periods_ = (0.5 + -1.0 * order_ * log(ERROR));
//     max_divider_ = get_max_divider();
//     data_length_ = (response_periods_ + HEADROOM) * max_divider_ + 2 * order;
//   }
//
//   size_t getOrder() const { return order_; }
//   size_t getMaxDivider() const { return max_divider_; }
//   size_t getResponsePeriods() const { return response_periods_; }
//   size_t getDataLength() const { return data_length_; }
// };
//
// class DoubleFilterCalculator : public DoubleFilterMetrics {
//   double *data_;
//
//   void zero() {
//     for (size_t i = 0; i < getDataLength(); i++) {
//       data_[i] = 0;
//     }
//   }
//
// public:
//   static constexpr double ERROR = 1e-3;
//   static constexpr size_t HEADROOM = 2;
//
//   DoubleFilterCalculator(size_t order)
//       : DoubleFilterMetrics(order), data_(new double[getDataLength()]) {}
//
//   double calculate(FilterType ft, size_t divider) {
//     if (divider < 2 || divider > getMaxDivider()) {
//       throw std::invalid_argument("Invalid divider");
//     }
//     zero();
//   }
//
//   ~DoubleFilterCalculator() { delete[] data_; }
// };

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
        return get_wb_low_pass_gain(order, relative);
      case FilterType::HIGH_PASS:
        return get_wb_high_pass_gain(order, relative);
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
    double g1 = get_wb_low_pass_gain(sample.order, sample.relative);
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

BOOST_AUTO_TEST_CASE(estimateFilterLength) {
  FixedOrderCoefficients<double, 2> coefficients;
  double err = pow(0.5, 24);
  double rate = 48000;
  for (double hz = 8000; hz >= 79; hz /= 10) {
    double f = hz / rate;
    Butterworth::create(coefficients, f, FilterType::LOW_PASS, 1.0);
    std::cout << "Low pass filter with frequency " << hz << " relative " << f << std::endl;
    for (size_t i = 0; i < coefficients.getCoefficientCount(); i++) {
      std::cout << "[" << i << "] FF=" << coefficients.getFF(i)
                << " \tFB=" << coefficients.getFB(i) << std::endl;
    }
    size_t length = effectiveIRLength(coefficients, 1048576, err, err);
    std::cout << "Length of 2nd order butterworth with f=" << f << "  is "
              << length << " or " << (1.0 * length / rate) << " seconds"
              << " f times samples " << (f * length) << std::endl;
  }
}

BOOST_AUTO_TEST_SUITE_END()
