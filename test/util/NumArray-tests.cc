//
// Created by michel on 22-10-20.
//

#include "test-helper.h"
#include <org-simple/util/NumArray.h>

using namespace org::simple::util;
using namespace org::simple::core;

namespace {  
constexpr size_t SIZE = 10;
typedef NumArray<double, SIZE> Numa;
typedef ArrayInline<double, SIZE> Array;

}

BOOST_AUTO_TEST_SUITE(org_simple_util_NumArray)



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

BOOST_AUTO_TEST_CASE(testNumArrayInitList) {
  Numa array {0,1,2,3,4,5,6,7,8,9};
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(i, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArrayInitListPartial) {
  Numa array {0,1,2,3,4,5,6};
  size_t i = 0;
  for (; i <= 6; i++) {
    BOOST_CHECK_EQUAL(i, array[i]);
  }
  for (; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(0, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArraySpliceConst) {
  Numa source {0,1,2,3,4,5,6,7,8,9};
  auto array = source.splice<3, 5>();
  BOOST_CHECK_EQUAL(3, array.size());
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(i + 3, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testNumArraySpliceVar) {
  Numa source {0,1,2,3,4,5,6,7,8,9};
  auto array = source.splice(3, 5);
  BOOST_CHECK_EQUAL(3, array.size());
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(i + 3, array[i]);
  }
}


BOOST_AUTO_TEST_CASE(testConstSize) {
  BOOST_CHECK_EQUAL(SIZE, Numa::constSize());
}

BOOST_AUTO_TEST_CASE(testArrayConstSize) {
  BOOST_CHECK_EQUAL(SIZE, Array::constSize());
}

BOOST_AUTO_TEST_CASE(testSize) {
  Numa array;
  BOOST_CHECK_EQUAL(SIZE, array.size());
}

BOOST_AUTO_TEST_SUITE_END()
