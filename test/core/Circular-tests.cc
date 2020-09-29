//
// Created by michel on 21-09-20.
//

#include "boost-unit-tests.h"
#include <org-simple/core/Circular.h>

namespace {

using SizeMetric = org::simple::core::SizeMetric<size_t>;

using Metric =
    org::simple::core::WrappedIndex<org::simple::core::WrappingType::BIT_MASK,
                                    SizeMetric>;

constexpr size_t requestedSize = 13;
constexpr size_t properSize = 16;
constexpr size_t properMask = 15;

Metric metric(requestedSize);

template <typename T>
static void checkExpectedAndActual(const char *description, T expected,
                                   T actual) {
  if (actual != expected) {
    std::stringstream msg;
    msg << description << ": expected " << expected << " got " << actual;
    BOOST_FAIL(msg.str().c_str());
  }
}
} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_circular)

BOOST_AUTO_TEST_CASE(testProperSize) {
  checkExpectedAndActual("Proper circular size for 13", properSize,
                         metric.size());
}

BOOST_AUTO_TEST_CASE(testAddNoWrap) {
  BOOST_CHECK_MESSAGE(metric.add(7, 5) == 12,
                      "WrappedIndex(16)::add(7, 5) == 12");
}

BOOST_AUTO_TEST_CASE(testLargeAddNoWrap) {
  BOOST_CHECK_MESSAGE(metric.add(7 + properSize, 5) == 12,
                      "WrappedIndex(16)::add(7 + size, 5) == 12");
}

BOOST_AUTO_TEST_CASE(testLargeAddLargeNoWrap) {
  BOOST_CHECK_MESSAGE(metric.add(7 + properSize, 5 + properSize) == 12,
                      "WrappedIndex(16)::add(7 + size, 5 + size) == 12");
}

BOOST_AUTO_TEST_CASE(testAddWrapZero) {
  BOOST_CHECK_MESSAGE(metric.add(7, 9) == 0,
                      "WrappedIndex(16)::add(7, 9) == 0");
}

BOOST_AUTO_TEST_CASE(testLargeAddWrapZero) {
  BOOST_CHECK_MESSAGE(metric.add(7 + properSize, 9) == 0,
                      "WrappedIndex(16)::add(7 + size, 9) == 0");
}

BOOST_AUTO_TEST_CASE(testLargeAddLargeWrapZero) {
  BOOST_CHECK_MESSAGE(metric.add(7 + properSize, 9 + properSize) == 0,
                      "WrappedIndex(16)::add(7 + size, 9 + size) == 0");
}

BOOST_AUTO_TEST_CASE(testAddWrapOne) {
  BOOST_CHECK_MESSAGE(metric.add(7, 10) == 1,
                      "WrappedIndex(16)::add(7, 10) == 1");
}

BOOST_AUTO_TEST_CASE(testLargeAddWrapOne) {
  BOOST_CHECK_MESSAGE(metric.add(7 + properSize, 10) == 1,
                      "WrappedIndex(16)::add(7 + size, 10) == 1");
}

BOOST_AUTO_TEST_CASE(testLargeAddLargeWrapOne) {
  BOOST_CHECK_MESSAGE(metric.add(7 + properSize, 10 + properSize) == 1,
                      "WrappedIndex(16)::add(7 + size, 10) == 1");
}

BOOST_AUTO_TEST_CASE(testAddNoWrapFromZero) {
  BOOST_CHECK_MESSAGE(metric.add(0, 5) == 5,
                      "WrappedIndex(16)::add(0, 5) == 5");
}

BOOST_AUTO_TEST_CASE(testLargeAddNoWrapFromZero) {
  BOOST_CHECK_MESSAGE(metric.add(0 + properSize, 5) == 5,
                      "WrappedIndex(16)::add(size, 5) == 5");
}

BOOST_AUTO_TEST_CASE(testLargeAddLargeNoWrapFromZero) {
  BOOST_CHECK_MESSAGE(metric.add(0 + properSize, 5 + properSize) == 5,
                      "WrappedIndex(16)::add(size, 5 + size) == 5");
}

BOOST_AUTO_TEST_CASE(testSubtractNoWrap) {
  BOOST_CHECK_MESSAGE(metric.sub(7, 5) == 2,
                      "WrappedIndex(16)::subtract(7, 5) == 2");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractNoWrap) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 5) == 2,
                      "WrappedIndex(16)::subtract(7 + size, 5) == 2");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractLargeNoWrap) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 5 + properSize) == 2,
                      "WrappedIndex(16)::subtract(7 + size, 5 + size) == 2");
}

BOOST_AUTO_TEST_CASE(testSubtractNoWrapZero) {
  BOOST_CHECK_MESSAGE(metric.sub(7, 7) == 0,
                      "WrappedIndex(16)::subtract(7, 7) == 0");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractNoWrapZero) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 7) == 0,
                      "WrappedIndex(16)::subtract(7 + size, 7) == 0");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractLargeNoWrapZero) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 7 + properSize) == 0,
                      "WrappedIndex(16)::subtract(7 + size, 7 + size) == 0");
}

BOOST_AUTO_TEST_CASE(testSubtractWrapOne) {
  BOOST_CHECK_MESSAGE(metric.sub(7, 8) == properMask,
                      "WrappedIndex(16)::subtract(7, 8) == mask");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractWrapOne) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 8) == properMask,
                      "WrappedIndex(16)::subtract(7 + size, 8) == mask");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractLargeWrapOne) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 8 + properSize) == properMask,
                      "WrappedIndex(16)::subtract(7 + size, 8 + size) == mask");
}

BOOST_AUTO_TEST_CASE(testSubtractWrapTwo) {
  BOOST_CHECK_MESSAGE(metric.sub(7, 9) == properMask - 1,
                      "WrappedIndex(16)::subtract(7, 9) == mask - 1");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractWrapTwo) {
  BOOST_CHECK_MESSAGE(metric.sub(7 + properSize, 9) == properMask - 1,
                      "WrappedIndex(16)::subtract(7 + size, 9) == mask - 1");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractLargeWrapTwo) {
  BOOST_CHECK_MESSAGE(
      metric.sub(7 + properSize, 9 + properSize) == properMask - 1,
      "WrappedIndex(16)::subtract(7 + size, 9 + size) == mask - 1");
}

BOOST_AUTO_TEST_CASE(testSubtractNoWrapFromMask) {
  BOOST_CHECK_MESSAGE(metric.sub(properMask, 5) == 10,
                      "WrappedIndex(16)::subtract(mask, 5) == 10");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractNoWrapFromMask) {
  BOOST_CHECK_MESSAGE(metric.sub(properMask + properSize, 5) == 10,
                      "WrappedIndex(16)::subtract(mask + size, 5) == 10");
}

BOOST_AUTO_TEST_CASE(testLargeSubtractLargeNoWrapFromMask) {
  BOOST_CHECK_MESSAGE(
      metric.sub(properMask + properSize, 5 + properSize) == 10,
      "WrappedIndex(16)::subtract(mask + size, 5 + size) == 10");
}

BOOST_AUTO_TEST_CASE(testRoundtripWithSetNext) {
  size_t reference = 0;
  size_t actual = 0;
  for (size_t i = 0; i <= properSize; i++) {
    reference = (reference + 1) % properSize;
    actual = metric.inc(actual);
    BOOST_CHECK_EQUAL(reference, actual);
  }
}

BOOST_AUTO_TEST_CASE(testRoundtripWithNext) {
  size_t reference = 0;
  size_t actual = 0;
  for (size_t i = 0; i <= properSize; i++) {
    reference = (reference + 1) % properSize;
    actual = metric.unsafe_inc(actual);
    BOOST_CHECK_EQUAL(reference, actual);
  }
}

BOOST_AUTO_TEST_CASE(testRoundtripWithSetPrevious) {
  size_t reference = 0;
  size_t actual = 0;
  for (size_t i = 0; i <= properSize; i++) {
    reference = (reference > 0) ? reference - 1 : properMask;
    actual = metric.dec(actual);
    BOOST_CHECK_EQUAL(reference, actual);
  }
}

BOOST_AUTO_TEST_CASE(testRoundtripWithPrevious) {
  size_t reference = 0;
  size_t actual = 0;
  for (size_t i = 0; i <= properSize; i++) {
    reference = (reference > 0) ? reference - 1 : properMask;
    actual = metric.unsafe_dec(actual);
    BOOST_CHECK_EQUAL(reference, actual);
  }
}

BOOST_AUTO_TEST_CASE(testMetricSetSizeOneSmaller) {
  Metric m(requestedSize);
  auto oldSize = m.size();
  m.set_element_count(properSize - 1);
  BOOST_CHECK_MESSAGE(m.size() == oldSize,
                      "Setting size one below current size yields same size");
}

BOOST_AUTO_TEST_CASE(testMetricSetSizeSame) {
  Metric m(requestedSize);
  auto oldSize = m.size();
  m.set_element_count(properSize);
  BOOST_CHECK_EQUAL(m.size(), oldSize);
  BOOST_CHECK_MESSAGE(m.size() == oldSize,
                      "Setting size to same value should not change anything");
}

BOOST_AUTO_TEST_CASE(testMetricSetSizeOneBigger) {
  Metric m(requestedSize);
  auto oldSize = m.size();
  m.set_element_count(properSize + 1);
  BOOST_CHECK_MESSAGE(m.size() == 2 * oldSize,
                      "Setting size one above yields twice as big size");
}

BOOST_AUTO_TEST_CASE(testMetricSetSizeHalf) {
  Metric m(requestedSize);
  auto oldSize = m.size();
  m.set_element_count(properSize / 2);
  BOOST_CHECK_EQUAL(m.size(), oldSize / 2);
  BOOST_CHECK_MESSAGE(m.size() == oldSize / 2,
                      "Setting size to half, yields half size");
}

BOOST_AUTO_TEST_SUITE_END()
