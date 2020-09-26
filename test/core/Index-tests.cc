//
// Created by michel on 24-09-20.
//

#include "boost-unit-tests.h"
#include <org-simple/core/Index.h>

using Exclusive = org::simple::core::Index;
using Inclusive = org::simple::core::Index::Inclusive;

static constexpr size_t SIZE = 10;
static constexpr size_t ZERO = 0;
static constexpr size_t ONE = 1;

BOOST_AUTO_TEST_SUITE(org_simple_core_Index)

BOOST_AUTO_TEST_CASE(testIndexCheckedZeroSizeZeroThrows) {
  [[maybe_unused]] size_t out;
  BOOST_CHECK_THROW(out = Exclusive::checked(ZERO, ZERO), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(testIndexCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, Exclusive::checked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Exclusive::checked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Exclusive::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Exclusive::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexCheckedSizeThrows) {
  [[maybe_unused]] size_t out;
  BOOST_CHECK_THROW(out = Exclusive::checked(SIZE, SIZE), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Exclusive::unchecked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, Exclusive::unchecked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Exclusive::unchecked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Exclusive::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Exclusive::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, Exclusive::unchecked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Inclusive::checked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, Inclusive::checked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Inclusive::checked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Inclusive::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Inclusive::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedSize) {
  BOOST_CHECK_EQUAL(SIZE, Inclusive::checked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedSizePlusOneThrows) {
  [[maybe_unused]] size_t out;
  BOOST_CHECK_THROW(out = Inclusive::checked(SIZE + ONE, SIZE),
                    std::out_of_range);
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Inclusive::unchecked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, Inclusive::unchecked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Inclusive::unchecked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Inclusive::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Inclusive::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, Inclusive::unchecked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedSizePlusOneDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE + ONE, Inclusive::unchecked(SIZE + ONE, SIZE));
}

/**
 * Tests for safe and unsafe do not include situations that can throw exception.
 * Creating multiple test sources with different values for
 * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED and
 * ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED does not work.
 */

BOOST_AUTO_TEST_CASE(testIndexSafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, Exclusive::safe(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexSafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Exclusive::safe(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Exclusive::safe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexSafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Exclusive::safe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Exclusive::unsafe(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, Exclusive::unsafe(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Exclusive::unsafe(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Exclusive::unsafe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Exclusive::unsafe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, Exclusive::unsafe(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Inclusive::safe(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, Inclusive::safe(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Inclusive::safe(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Inclusive::safe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Inclusive::safe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeSize) {
  BOOST_CHECK_EQUAL(SIZE, Inclusive::safe(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Inclusive::unsafe(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, Inclusive::unsafe(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Inclusive::unsafe(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Inclusive::unsafe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Inclusive::unsafe(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, Inclusive::unsafe(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeSizePlusOneDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE + ONE, Inclusive::unsafe(SIZE + ONE, SIZE));
}

BOOST_AUTO_TEST_SUITE_END()
