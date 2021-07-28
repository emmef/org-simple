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

class DoubleFilterMetrics {
  size_t order_;

private:
  size_t max_divider_;
  size_t response_periods_;
  size_t data_length_;

public:
  static constexpr double ERROR = 1e-3;
  static constexpr size_t HEADROOM = 2;

  DoubleFilterMetrics(unsigned order) : order_(order) {
    response_periods_ = (0.5 + -1.0 * order_ * log(ERROR));
    max_divider_ = get_max_divider();
    data_length_ = (response_periods_ + HEADROOM) * max_divider_ + 2 * order;
  }

  size_t getOrder() const { return order_; }
  size_t getMaxDivider() const { return max_divider_; }
  size_t getResponsePeriods() const { return response_periods_; }
  size_t getDataLength() const { return data_length_; }

};

class DoubleFilterCalculator : public DoubleFilterMetrics {
  double *data_;

  void zero() {
    for (size_t i = 0; i < getDataLength(); i++) {
      data_[i] = 0;
    }
  }

public:
  static constexpr double ERROR = 1e-3;
  static constexpr size_t HEADROOM = 2;

  DoubleFilterCalculator(size_t order) : DoubleFilterMetrics(order), data_(new double[getDataLength()]) {
  }

  double calculate(FilterType ft, size_t divider) {
    if (divider < 2 || divider > getMaxDivider()) {
      throw std::invalid_argument("Invalid divider");
    }
    zero();

  }

  ~DoubleFilterCalculator() {
    delete[] data_;
  }
};

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

BOOST_AUTO_TEST_SUITE_END()
