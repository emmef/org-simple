//
// Created by michel on 24-09-20.
//

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <org-simple/core/Index.h>

using Exclusive = org::simple::Index;
using Inclusive = org::simple::Index::Inclusive;

static constexpr size_t SIZE = 10;
static constexpr size_t ZERO = 0;
static constexpr size_t ONE = 1;

BOOST_AUTO_TEST_SUITE(org_simple_core_Index)

BOOST_AUTO_TEST_CASE(testIndexCheckedZeroSizeZeroFails) {
  size_t out;
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
  size_t out;
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
  size_t out;
  BOOST_CHECK_THROW(out = Inclusive::checked(SIZE + ONE, SIZE), std::out_of_range);
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


BOOST_AUTO_TEST_SUITE_END()
