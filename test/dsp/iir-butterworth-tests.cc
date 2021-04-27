//
// Created by michel on 27-04-21.
//

#include "test-helper.h"
#include <cmath>
#include <org-simple/dsp/iir-butterworth.h>
#include <vector>

using namespace org::simple::iir;

static bool same(double v1, double v2, double epsilon = 1e-12) {
  return org::simple::test::same(v1, v2, epsilon);
}

static double reference_gain(size_t order, double rel) {
  return 1.0 / sqrt(1.0 + pow(rel, 2 * order));
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
        return reference_gain(order, relative);
      case FilterType::HIGH_PASS:
        return relative > std::numeric_limits<double>::epsilon()
                   ? reference_gain(order, 1.0 / relative)
                   : 0;
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
  scenarios.push_back({FilterType::LOW_PASS, 1, 1e20, 0, true});
  scenarios.push_back({FilterType::HIGH_PASS, 1, 1e20, 1, true});

  for (size_t order = 1; is_valid_bw_order(order); order++) {
    scenarios.push_back({FilterType::LOW_PASS, order, 1.0, M_SQRT1_2, true});
    scenarios.push_back({FilterType::HIGH_PASS, order, 1.0, M_SQRT1_2, true});
  }

  for (size_t order = 1; is_valid_bw_order(order); order++) {
    for (double relative = 0.125; relative <= 8; relative *= 2) {
      scenarios.push_back({FilterType::LOW_PASS, order, relative,
                           reference_gain(order, 1.0 / relative)});
      scenarios.push_back({FilterType::HIGH_PASS, order, relative, reference_gain(order, relative)});
    }
  }

  return scenarios;
}

BOOST_AUTO_TEST_SUITE(org_simple_dsp_iir_butterworth_tests)

BOOST_DATA_TEST_CASE(sample, createTestScenarios()) {
  if (!same(sample.expected_gain, sample.actual())) {
    BOOST_CHECK_EQUAL(sample.expected_gain, sample.actual());
  }
  else {
    BOOST_CHECK(same(sample.expected_gain, sample.actual()));
  }
}

BOOST_AUTO_TEST_SUITE_END()
