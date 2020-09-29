//
// Created by michel on 26-09-20.
//

#include "boost-unit-tests.h"
#include <org-simple/core/Size.h>


BOOST_AUTO_TEST_SUITE(org_simple_core_SizeMetric)

BOOST_AUTO_TEST_CASE(testSizeMetricBaseValues) {
  typedef org::simple::core::SizeMetric<size_t> Size;

  BOOST_CHECK_EQUAL(Size::max, ~size_t(0));
  BOOST_CHECK_EQUAL(Size::max_mask, ~size_t(0));
  BOOST_CHECK_EQUAL(Size::max_index, ~size_t(0));
}

BOOST_AUTO_TEST_CASE(testSizeMetricValues) {
  typedef org::simple::core::SizeMetric<size_t> Size;

  BOOST_CHECK_EQUAL(Size::max, ~size_t(0));
  BOOST_CHECK_EQUAL(Size::max_mask, ~size_t(0));
  BOOST_CHECK_EQUAL(Size::max_index, ~size_t(0));
}

BOOST_AUTO_TEST_CASE(testSizeMetricElementValues) {
  typedef org::simple::core::SizeMetric<size_t> Size;
  static constexpr size_t element_size = 13;
  typedef Size::Elements<element_size> Elements;
  BOOST_CHECK_EQUAL(Elements::max, ~size_t(0) / element_size);
  static constexpr size_t group_size = 13;
  typedef Elements::Elements<group_size> Groups;
  BOOST_CHECK_EQUAL(Groups::max, ~size_t(0) / element_size / group_size);
}

BOOST_AUTO_TEST_CASE(testBitLimitedSizeMetric16) {
  typedef org::simple::core::SizeMetricWithBitLimit<size_t, 16> Size;

  BOOST_CHECK_EQUAL(Size::max, 0x10000);
  BOOST_CHECK_EQUAL(Size::max_mask, 0xffff);
  BOOST_CHECK_EQUAL(Size::max_index, 0xffff);
}

BOOST_AUTO_TEST_CASE(testLimitedSizeMetric) {
  typedef org::simple::core::SizeMetricWithLimit<size_t, 48000> Size;
  size_t max = 48000;
  size_t mask = org::simple::core::Bits<size_t>::bit_mask_not_exceeding(48000);
  size_t index = max - 1;

  BOOST_CHECK_EQUAL(Size::max, max);
  BOOST_CHECK_EQUAL(Size::max_mask, mask);
  BOOST_CHECK_EQUAL(Size::max_index, index);
}

BOOST_AUTO_TEST_CASE(test) {
}

BOOST_AUTO_TEST_SUITE_END()
