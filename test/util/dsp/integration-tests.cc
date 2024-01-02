//
// Created by michel on 13-04-21.
//

#include "test-helper.h"
#include "org-simple/dsp/integration.h"
#include <boost/math/special_functions/relative_difference.hpp>

using namespace org::simple::util::dsp;

static bool same(double v1, double v2, double epsilon = 1e-12) {
  return boost::math::relative_difference(v1, v2) < epsilon;
}

BOOST_AUTO_TEST_SUITE(org_simple_dsp_integration_Tests)

BOOST_AUTO_TEST_CASE(testInitWithScaleOne) {
  Coefficients coeffs = Coefficients ::fromCount(1.0, 1.0);

  BOOST_CHECK_EQUAL(1.0, coeffs.scale());

  BOOST_CHECK_EQUAL(exp(-1), coeffs.historyMultiplier());

  BOOST_CHECK_EQUAL(1.0 - exp(-1), coeffs.inputMultiplier());

  BOOST_CHECK_EQUAL(1.0, coeffs.samples());
}

BOOST_AUTO_TEST_CASE(testInitWithScaleOneAndImplicitOne) {
  Coefficients coeffs1n = Coefficients ::fromCount(1.0);
  Coefficients coeffs11 = Coefficients ::fromCount(1.0, 1.0);

  BOOST_CHECK_EQUAL(coeffs11.scale(), coeffs1n.scale());

  BOOST_CHECK_EQUAL(coeffs11.historyMultiplier(),
                    coeffs1n.historyMultiplier());

  BOOST_CHECK_EQUAL(coeffs11.inputMultiplier(), coeffs1n.inputMultiplier());

  BOOST_CHECK_EQUAL(coeffs11.samples(), coeffs1n.samples());
}

BOOST_AUTO_TEST_CASE(testInitWithScaleTwo) {
  const double scale = 2.0;
  Coefficients coeffs1 = Coefficients ::fromCount(1.0, 1.0);
  Coefficients coeffs2 = Coefficients ::fromCount(1.0, scale);

  BOOST_CHECK_EQUAL(scale, coeffs2.scale());

  BOOST_CHECK_EQUAL(coeffs1.historyMultiplier(), coeffs2.historyMultiplier());

  BOOST_CHECK_EQUAL(scale * coeffs1.inputMultiplier(), coeffs2.inputMultiplier());

  BOOST_CHECK_EQUAL(1.0, coeffs2.samples());
}

BOOST_AUTO_TEST_CASE(testCOuntOneScaleOneIntegrationConsistency) {
  Coefficients coeffs = Coefficients ::fromCount(1.0, 1.0);
  double input = 1.0;
  double history1 = 0;
  double history2 = 0;
  double history3 = 0;
  double output1;
  double output2;

  output1 = coeffs.getIntegrated(history1, input);
  output2 = coeffs.integrateAndGet(history2, input);
  coeffs.integrate(history3, input);

  BOOST_CHECK_EQUAL(output1, output2);
  BOOST_CHECK_EQUAL(output2, history2);
  BOOST_CHECK_EQUAL(output2, history3);

  BOOST_CHECK_EQUAL(input * coeffs.inputMultiplier(), output1);

  history1 = history2;
  output1 = coeffs.getIntegrated(history1, input);
  output2 = coeffs.integrateAndGet(history2, input);
  coeffs.integrate(history3, input);

  BOOST_CHECK_EQUAL(output1, output2);
  BOOST_CHECK_EQUAL(output2, history2);
  BOOST_CHECK_EQUAL(output2, history3);

  BOOST_CHECK_EQUAL(
      coeffs.inputMultiplier() * (coeffs.historyMultiplier() + input), output1);
}

BOOST_AUTO_TEST_CASE(testCountOneScaleTwoIntegrationConsistency) {
  const double scale = 2;
  Coefficients coeffs1 = Coefficients ::fromCount(1.0, 1.0);
  Coefficients coeffs2 = Coefficients ::fromCount(1.0, scale);
  double input = 1.0;
  double history1 = 0;
  double history2 = 0;

  coeffs1.integrate(history1, input);
  coeffs2.integrate(history2, input);
  BOOST_CHECK_EQUAL(scale * history1, history2);
  coeffs1.integrate(history1, input);
  coeffs2.integrate(history2, input);
  BOOST_CHECK_EQUAL(scale * history1, history2);
}

BOOST_AUTO_TEST_CASE(testImpulseResponseSumIsScale) {
  Coefficients coeffs;
  for (double samples = 0.5; samples < 5; samples += 0.5) {
    for (double scale = 0.5; scale < 5; scale += 0.5) {
      coeffs = Coefficients::fromCount(samples, scale);
      double input = 1;
      double previous_output = 10 * scale * samples;
      double output = 0;
      double previous_sum = -1;
      double sum = 0;
      int i = 0;
      while (++i < 10 || (sum > previous_sum && output < previous_output && output > 1e-8)) {
        previous_output = output;
        previous_sum = sum;
        sum += coeffs.integrateAndGet(output, input);
        input = 0;
      }
      BOOST_CHECK(same(sum, scale, 1e-7));
      if (!same(sum, scale, 1e-7)) {
        std::cout << "Sum=" << sum << "; scale=" << scale << std::endl;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testStepResponseIsScale) {
  Coefficients coeffs;
  for (double samples = 0.5; samples < 5; samples += 0.5) {
    for (double scale = 0.5; scale < 5; scale += 0.5) {
      coeffs = Coefficients::fromCount(samples, scale);
      double input = 1;
      double output = 0;
      int i = 0;
      while (++i < 10000 || !same(output, scale)) {
        coeffs.integrate(output, input);
      }
      BOOST_CHECK(same(output, scale, 1e-7));
      if (!same(output, scale, 1e-7)) {
        std::cout << "Output=" << output << "; scale=" << scale << std::endl;
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
