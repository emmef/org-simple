//
// Created by michel on 30-07-21.
//

#include "iir-coefficients-test-helper.h"
#include <org-simple/util/dsp/iir-biquad.h>

using namespace org::simple::util::dsp;

using Filter = FixedOrderCoefficients<double, 2>;
using IncompatibleFilter = FixedOrderCoefficients<double, 3>;

/**
 * Test coverage is not complete, as the characteristics of the filters can so
 * widely vary.
 */

class BiQuadScenario : public FilterScenario {
  class Center : public Biquad::ConfigCenter {
    size_t center_period_;
    size_t signal_period_;
    FilterType type_;

  public:
    Center(FilterType type, size_t center_period, size_t signal_period)
        : Biquad::ConfigCenter(1.0 / std::max(center_period, 1lu)),
          center_period_(std::max(center_period, 1lu)),
          signal_period_(std::max(signal_period, 1lu)), type_(type) {}

    [[nodiscard]] size_t signal_period() const { return signal_period_; }
    [[nodiscard]] size_t center_period() const { return center_period_; }
    [[nodiscard]] FilterType type() const override { return type_; }
    [[nodiscard]] double center() const {
      return Biquad::ConfigCenter::center();
    }
  };

  Center center_;

public:
  BiQuadScenario(FilterType type, size_t center_period, size_t signal_period)
      : FilterScenario(type, 2), center_(type, center_period, signal_period) {}

  [[nodiscard]] size_t signal_period() const { return center_.signal_period(); }
  [[nodiscard]] size_t center_period() const { return center_.center_period(); }
  [[nodiscard]] double center() const { return center_.center(); }
  [[nodiscard]] double signal() const { return 1.0 / signal_period(); }

  void parameters(std::ostream &out) const override {
    out << "; center_period=" << center_period() << " (" << center()
        << "); signal_period=" << signal_period() << "(" << signal() << ")";
  }
};

class ParametricScenario : public BiQuadScenario {
  Biquad::Parametric para_;
  Filter filter_;

public:
  ParametricScenario(size_t center_period, size_t signal_period, double gain,
                     double bandwidth)
      : BiQuadScenario(FilterType::PARAMETRIC, center_period, signal_period),
        para_(center()) {
    para_.setGain(gain).setBandwidth(bandwidth);
    para_.build(filter_);
  }

  const char *typeOfScenario() const override { return "ParametricScenario"; }
  [[nodiscard]] double gain() const { return para_.gain(); }
  [[nodiscard]] double bandwidth() const { return para_.bandwidth(); }

  void parameters(std::ostream &out) const override {
    BiQuadScenario::parameters(out);
    out << "; gain=" << para_.gain() << "; bandwidth=" << para_.bandwidth();
  }

  bool testCenterGain(double &measured, double &expected) const {
    expected = gain();
    if (signal_period() == center_period()) {
      double error = signal_period() / (signal_period() - 0.5);
      size_t length;
      measured = measureFilterGain(filter_, center_period(), length);
      double min = std::min(expected, measured) * error;
      return measured <= min && expected <= min;
    }
    measured = expected;
    return true;
  }

  void printCoefficients() const {

    for (unsigned i = 0; i <= filter_.getOrder(); i++) {
      std::cout << "\t" << i << " \tFF " << filter_.getFF(i) << " \tFB "
                << filter_.getFB(i) << std::endl;
    }
  }
};

std::vector<ParametricScenario> createScenarios() {
  std::vector<ParametricScenario> result;

  for (size_t center_period : get_test_periods()) {
    for (size_t signal_period : get_test_periods()) {
      for (double gain = 0.25; gain <= 4; gain *= 4) {
        for (double bandwidth = 0.25; bandwidth <= 4; bandwidth *= 2) {
          result.emplace_back(
              ParametricScenario(center_period, signal_period, gain, 1.0));
        }
      }
    }
  }
  return result;
}

#include "test-helper.h"

BOOST_AUTO_TEST_SUITE(org_simple_dsp_iir_Biquad_Tests)

BOOST_AUTO_TEST_CASE(testIncompatibleFilter) {
  Filter f;
  IncompatibleFilter fi;
  auto parametric = Biquad::parametric(0.25);
  BOOST_CHECK_NO_THROW(parametric.build(f));
  BOOST_CHECK_THROW(parametric.build(fi), std::invalid_argument);
}

BOOST_DATA_TEST_CASE(testParametricCenterGain, createScenarios()) {
  double measured;
  double expected;
  if (!sample.testCenterGain(measured, expected)) {
    BOOST_CHECK_EQUAL(measured, expected);
    sample.testCenterGain(measured, expected);
    sample.printCoefficients();
  }
}

BOOST_AUTO_TEST_SUITE_END()
