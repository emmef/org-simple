//
// Created by michel on 24-09-20.
//

#include "boost-unit-tests.h"
#include <org-simple/Index.h>

using Index = org::simple::Index;
using EndIndex = org::simple::EndIndex;

static constexpr size_t SIZE = 10;
static constexpr size_t ZERO = 0;
static constexpr size_t ONE = 1;

BOOST_AUTO_TEST_SUITE(org_simple_core_Index)

BOOST_AUTO_TEST_CASE(testIndexCheckedZeroSizeZeroThrows) {
  [[maybe_unused]] size_t out;
  BOOST_CHECK_THROW(out = Index::checked(ZERO, ZERO), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(testIndexCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, Index::checked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Index::checked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Index::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Index::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexCheckedSizeThrows) {
  [[maybe_unused]] size_t out;
  BOOST_CHECK_THROW(out = Index::checked(SIZE, SIZE), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Index::unchecked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, Index::unchecked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Index::unchecked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Index::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Index::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnCheckedSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, Index::unchecked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, EndIndex::checked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, EndIndex::checked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, EndIndex::checked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, EndIndex::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, EndIndex::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedSize) {
  BOOST_CHECK_EQUAL(SIZE, EndIndex::checked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveCheckedSizePlusOneThrows) {
  [[maybe_unused]] size_t out;
  BOOST_CHECK_THROW(out = EndIndex::checked(SIZE + ONE, SIZE),
                    std::out_of_range);
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, EndIndex::unchecked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedZeroArgument) {
  BOOST_CHECK_EQUAL(0, EndIndex::unchecked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, EndIndex::unchecked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, EndIndex::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, EndIndex::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, EndIndex::unchecked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnCheckedSizePlusOneDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE + ONE, EndIndex::unchecked(SIZE + ONE, SIZE));
}

/**
 * Tests for safe and unsafe do not include situations that can throw exception.
 * Creating multiple test sources with different values for
 * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED and
 * ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED does not work.
 */

BOOST_AUTO_TEST_CASE(testIndexSafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, Index::checked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexSafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Index::checked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Index::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexSafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Index::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, Index::unchecked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, Index::unchecked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, Index::unchecked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, Index::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, Index::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testIndexUnsafeSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, Index::unchecked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, EndIndex::checked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, EndIndex::checked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, EndIndex::checked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, EndIndex::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, EndIndex::checked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveSafeSize) {
  BOOST_CHECK_EQUAL(SIZE, EndIndex::checked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeZeroSizeZero) {
  BOOST_CHECK_EQUAL(ZERO, EndIndex::unchecked(ZERO, ZERO));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeZeroArgument) {
  BOOST_CHECK_EQUAL(0, EndIndex::unchecked(ZERO, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeNonZeroArgument) {
  size_t value = 3;
  BOOST_CHECK_EQUAL(value, EndIndex::unchecked(value, SIZE));
  value = 5;
  BOOST_CHECK_EQUAL(value, EndIndex::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeSizeMinusOne) {
  size_t value = SIZE - 1;
  BOOST_CHECK_EQUAL(value, EndIndex::unchecked(value, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeSizeDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE, EndIndex::unchecked(SIZE, SIZE));
}

BOOST_AUTO_TEST_CASE(testInclusiveUnsafeSizePlusOneDoesNotThrow) {
  BOOST_CHECK_EQUAL(SIZE + ONE, EndIndex::unchecked(SIZE + ONE, SIZE));
}

BOOST_AUTO_TEST_SUITE_END()
