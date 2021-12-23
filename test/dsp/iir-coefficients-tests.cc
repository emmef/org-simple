//
// Created by michel on 11-04-21.
//

#include <cstdio>
#include <test-helper.h>
#include "iir-coefficients-test-helper.h"

static constexpr size_t SIZE = 10;
static constexpr size_t INPUT = 0;
static constexpr size_t OUTPUT = 1;
static constexpr size_t BUFFERS = 2;

BOOST_AUTO_TEST_SUITE(org_simple_dsp_iir_Coefficients)

BOOST_AUTO_TEST_CASE(testFilterScenarioBufferCopy) {
  FilterScenarioBuffer<2> filter1(SIZE, BUFFERS);
  FilterScenarioBuffer<2> filter2(SIZE, BUFFERS);
  filter1.fill_with_random(INPUT);
  filter2.copy_from(filter1, INPUT);

  BOOST_CHECK_MESSAGE(filter1.equals(filter2, INPUT), "copy_from: target should be equal to source");
  BOOST_CHECK_MESSAGE(filter2.equals(filter1, INPUT), "copy_from: source should be equal to target");

  BOOST_CHECK(filter1.equals(filter2, INPUT));
}

BOOST_AUTO_TEST_CASE(testFilterScenarioBufferCopyReverse) {
  FilterScenarioBuffer<2> filter1(SIZE, BUFFERS);
  FilterScenarioBuffer<2> filter2(SIZE, BUFFERS);
  FilterScenarioBuffer<2> filter3(SIZE, BUFFERS);
  filter1.fill_with_random(INPUT);
  filter2.copy_from_reverse(filter1, INPUT);
  filter3.copy_from(filter1, INPUT);

  BOOST_CHECK_MESSAGE(filter1.equals_reverse(filter2, INPUT), "copy_from_reverse: target should be equal to source");
  BOOST_CHECK_MESSAGE(filter2.equals_reverse(filter1, INPUT), "copy_from_reverse: source should be equal to target");

  BOOST_CHECK(filter1.equals(filter3, INPUT));
}

BOOST_AUTO_TEST_CASE(testFilterSingleEqualsForwardWithOffset) {
  FilterScenarioBuffer<2> filter1(SIZE, BUFFERS);
  FilterScenarioBuffer<2> filter2(SIZE, BUFFERS);
  org::simple::util::dsp::FixedOrderCoefficients<double, 2> coeffs;
  filter1.generate_random_filter(coeffs);

  filter1.fill_with_random(INPUT);
  filter2.copy_from(filter1, INPUT);
  filter1.filter_forward_offs(coeffs, INPUT, OUTPUT);
  filter2.filter_forward_single(coeffs, INPUT, OUTPUT);

  BOOST_CHECK(filter1.equals(filter2, OUTPUT));
}

BOOST_AUTO_TEST_CASE(testForwardEqualsBackwardBothWithOffset) {
  FilterScenarioBuffer<2> filter1(SIZE, BUFFERS);
  FilterScenarioBuffer<2> filter2(SIZE, BUFFERS);
  org::simple::util::dsp::FixedOrderCoefficients<double, 2> coeffs;
  filter1.generate_random_filter(coeffs);

  filter1.fill_with_random(INPUT);
  filter2.copy_from_reverse(filter1, INPUT);

  filter1.filter_forward_offs(coeffs, INPUT, OUTPUT);
  filter2.filter_backward_offs(coeffs, INPUT, OUTPUT);

  BOOST_CHECK(filter1.equals_reverse(filter2, OUTPUT));
}

BOOST_AUTO_TEST_CASE(testForwardEqualsBackwardBothWithZeroHistory) {
  FilterScenarioBuffer<2> filter1(SIZE, BUFFERS);
  FilterScenarioBuffer<2> filter2(SIZE, BUFFERS);
  org::simple::util::dsp::FixedOrderCoefficients<double, 2> coeffs;
  filter1.generate_random_filter(coeffs);

  filter1.fill_with_random(INPUT);
  filter2.copy_from_reverse(filter1, INPUT);

  filter1.filter_forward_zero(coeffs, INPUT, OUTPUT);
  filter2.filter_backward_zero(coeffs, INPUT, OUTPUT);

  BOOST_CHECK(filter1.equals_reverse(filter2, OUTPUT));
}

BOOST_AUTO_TEST_SUITE_END()

