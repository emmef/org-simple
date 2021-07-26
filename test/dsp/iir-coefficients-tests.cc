//
// Created by michel on 11-04-21.
//

#include <cstdio>
#include <test-helper.h>
#include "iir-coefficients-test-helper.h"

BOOST_AUTO_TEST_SUITE(org_simple_dsp_iir_Coefficients)

BOOST_AUTO_TEST_CASE(testFilterScenarioBufferCopy) {
  static constexpr size_t SIZE = 10;
  FilterScenarioBuffer<2, SIZE> filter1;
  FilterScenarioBuffer<2, SIZE> filter2;
  filter1.fill_with_random();
  filter2.copy_from(filter1);

  BOOST_CHECK_MESSAGE(filter1.equals_input(filter2), "copy_from: target should be equal to source");
  BOOST_CHECK_MESSAGE(filter2.equals_input(filter1), "copy_from: source should be equal to target");

  BOOST_CHECK(filter1.equals(filter2));
}

BOOST_AUTO_TEST_CASE(testFilterScenarioBufferCopyReverse) {
  static constexpr size_t SIZE = 10;
  FilterScenarioBuffer<2, SIZE> filter1;
  FilterScenarioBuffer<2, SIZE> filter2;
  filter1.fill_with_random();
  filter2.copy_from_reverse(filter1);

  BOOST_CHECK_MESSAGE(filter1.equals_reverse_input(filter2), "copy_from_reverse: target should be equal to source");
  BOOST_CHECK_MESSAGE(filter2.equals_reverse_input(filter1), "copy_from_reverse: source should be equal to target");

  BOOST_CHECK(filter1.equals(filter2));
}

BOOST_AUTO_TEST_CASE(testFilterSingleEqualsForwardWithOffset) {
  static constexpr size_t SIZE = 10;
  FilterScenarioBuffer<2, SIZE> filter1;
  FilterScenarioBuffer<2, SIZE> filter2;
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
  FilterScenarioBuffer<2, SIZE> filter1;
  FilterScenarioBuffer<2, SIZE> filter2;
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
  FilterScenarioBuffer<2, SIZE> filter1;
  FilterScenarioBuffer<2, SIZE> filter2;
  Coefficients<2> coeffs;
  filter1.generate_random(coeffs);

  filter1.fill_with_random();
  filter2.copy_from_reverse(filter1);

  filter1.filter_forward_zero(coeffs);
  filter2.filter_backward_zero(coeffs);

  BOOST_CHECK(filter1.equals_reverse(filter2));
}

BOOST_AUTO_TEST_SUITE_END()

