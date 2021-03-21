//
// Created by michel on 22-10-20.
//

#include "test-helper.h"
#include <org-simple/util/NumArray.h>

using namespace org::simple::util;
using namespace org::simple::core;

namespace {

constexpr size_t SIZE = 8;
constexpr size_t SMALLER = 4;
constexpr size_t BIGGER = 16;
typedef NumArray<double, SIZE> Numa;
typedef NumArray<double, SMALLER> NumaSmall;
typedef NumArray<double, BIGGER> NumaBig;
typedef Array<double, SIZE> Array10;

}

BOOST_AUTO_TEST_SUITE(org_simple_util_NumArray)

//BOOST_AUTO_TEST_CASE(testNumArrayIsBaseArrayConstSize) {
//  bool is = concepts::BaseArrayConstSizeImpl<double, Numa10>;
//  BOOST_CHECK_MESSAGE(is, "concepts::BaseArrayConstSizeImpl<double, Numa10>");
//}

BOOST_AUTO_TEST_CASE(testSetGet) {
  Numa array;
  static constexpr size_t index = 4;
  double old = array[index];
  double newValue = fabs(old) > 1e-6 ? old / 2 : old + 1.0;
  array[index] = newValue;
  BOOST_CHECK_EQUAL(newValue, array[index]);
}

BOOST_AUTO_TEST_CASE(testNumArraySizeOverhead) {
  Numa array;
  BOOST_CHECK_EQUAL(sizeof(array), sizeof(double) * SIZE);
}

BOOST_AUTO_TEST_CASE(testNumArrayInitListExactSize) {
  Numa array {0,1,2,3,4,5,6,7,8,9};
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(i, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArrayInitListPartial) {
  Numa array {0,1,2,3,4,5,6};
  size_t i = 0;
  for (; i <= 6; i++) {
    BOOST_CHECK_EQUAL(i, array[i]);
  }
  for (; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(0, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArrayInitListLarger) {
  Numa array {0,1,2,3,4,5,6,7,8,9,10,11,12};
  size_t i = 0;
  for (; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(i, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArrayrangeConst) {
  NumaBig source {0,1,2,3,4,5,6,7,8,9};
  auto array = source.range_ref<3, 5>();
  BOOST_CHECK_EQUAL(3, array.capacity());
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(i + 3, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArrayrangeVar) {
  Numa source {0,1,2,3,4,5,6,7,8,9};
  auto array = source.range_copy(3, 5);
  BOOST_CHECK_EQUAL(3, array.capacity());
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(i + 3, array[i]);
  }
}


BOOST_AUTO_TEST_CASE(testConstSize) {
  BOOST_CHECK_EQUAL(SIZE, Numa::FIXED_CAPACITY);
}

BOOST_AUTO_TEST_CASE(testArrayConstSize) {
  BOOST_CHECK_EQUAL(SIZE, Array10::FIXED_CAPACITY);
}

BOOST_AUTO_TEST_CASE(testSize) {
  Numa array;
  BOOST_CHECK_EQUAL(SIZE, array.capacity());
}

BOOST_AUTO_TEST_CASE(testAdd) {
  NumaSmall array {0,1,2,3};
  auto sum = array + array;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(i * 2, sum[i]);
  }
}


BOOST_AUTO_TEST_SUITE_END()
