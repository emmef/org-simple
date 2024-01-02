//
// Created by michel on 22-10-20.
//

#include "test-helper.h"
#include <org-simple/NumArray.h>

using namespace org::simple;

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

BOOST_AUTO_TEST_CASE(testAssign) {
  NumaSmall array1 {0,1,2,3};
  NumaSmall array2 {7,8, 9, 10};
  NumaSmall array3 {0,1,2,3};

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_NE(array1[i], array2[i]);
  }
  array3 = array2;
  array1 = array2;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array1[i], array2[i]);
  }
  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array1[i], array2[i]);
  }
  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array2[i], array3[i]);
  }
}

BOOST_AUTO_TEST_CASE(testInitParentheses) {
  NumaSmall array1 {0,1,2,3};
  NumaSmall array2 (array1);

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array1[i], array2[i]);
  }
}

BOOST_AUTO_TEST_CASE(testInitAssign) {
  NumaSmall array1 {0,1,2,3};
  NumaSmall array2 = array1;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array1[i], array2[i]);
  }
}

BOOST_AUTO_TEST_CASE(testAddUnarySame) {
  NumaSmall array {0,1,2,3};
  array += array;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(double(i) + double(i), array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testAddUnaryDiff) {
  NumaSmall array1{0,1,2,3};
  NumaSmall array2{5,6,7,8};
  array1 += array2;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(double(i) + double(i + 5), array1[i]);
  }
}

BOOST_AUTO_TEST_CASE(testAddBinarySameAsAddUnarySame) {
  NumaSmall array {0,1,2,3};
  auto sum = array + array;
  array += array;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(array[i], sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testAddBinarySameAsAddUnaryDiff) {
  NumaSmall array1{0,1,2,3};
  NumaSmall array2{5,6,7,8};
  auto sum = array1 + array2;
  array1 += array2;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array1[i], sum[i]);
  }
}


BOOST_AUTO_TEST_CASE(testSubUnarySame) {
  NumaSmall array {0,1,2,3};
  NumaSmall array2(array);
  array -= array2;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(0.0, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSubUnaryDiff) {
  NumaSmall array1{0,1,2,3};
  NumaSmall array2{5,6,7,8};
  array1 -= array2;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(double(i) - double(i + 5), array1[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSubBinarySameAsAddUnarySame) {
  NumaSmall array {0,1,2,3};
  NumaSmall array2(array);
  auto sum = array - array;
  array -= array2;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(array[i], sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSubBinarySameAsAddUnaryDiff) {
  NumaSmall array1{0,1,2,3};
  NumaSmall array2{5,6,7,8};
  auto sum = array1 - array2;
  array1 -= array2;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array1[i], sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testAddBinaryAndMultiply) {
  NumaSmall array1 {0,1,2,3};
  NumaSmall array2 = array1;

  auto sum = array1 + 2 * array2;

  array2 *= 2;
  array2 += array1;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(array2[i], sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testAddBinaryAndMultiplyAdded) {
  NumaSmall array {0,1,2,3};
  NumaSmall array2 = { 7,8,9,10 };
  auto sum = array + 2 * (array + array2);

  NumaSmall temp1(array);
  temp1 += array2;
  temp1 *= 2;
  temp1 += array;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(temp1[i], sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSubtractBinaryAndMultiply) {
  NumaSmall array1 {5,7,9,11};
  NumaSmall array2 {0,1,2,3};
  auto sum = array1 - 2 * array2;

  for (size_t i = 0; i < sum.capacity(); i++) {
    BOOST_CHECK_EQUAL(2 * i + 5, array1[i]);
  }
  for (size_t i = 0; i < sum.capacity(); i++) {
    BOOST_CHECK_EQUAL(i, array2[i]);
  }
  for (size_t i = 0; i < sum.capacity(); i++) {
    BOOST_CHECK_EQUAL(2 * i + 5 - 2 * i, sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSubtractBinaryAndMultiplySubtracted) {
  NumaSmall array1 {1,2,3,4};
  NumaSmall array2 {5,6,7,8};
  NumaSmall array3 = { 9, 10, 11, 12 };
  auto sum = array1 - 2 * (array2 - array3);

  for (size_t i = 0; i < sum.capacity(); i++) {
    BOOST_CHECK_EQUAL(i + 9, sum[i]);
  }
}

BOOST_AUTO_TEST_CASE(testMultiplyUnary) {
  NumaSmall array {1,2,3, 4};
  array *= 2;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL((i + 1) * 2, array[i]);
  }
}


BOOST_AUTO_TEST_CASE(testMultiplyBinary) {
  NumaSmall array {1,2,3, 4};
  auto product = array * 2;
  array *= 2;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(array[i], product[i]);
  }
}

BOOST_AUTO_TEST_CASE(testDivideUnary) {
  NumaSmall array {1,2,3, 4};
  array /= 2;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(double(i + 1) / 2.0, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testDivideBinary) {
  NumaSmall array {1,2,3, 4};
  auto product = array / 2;
  array /= 2;

  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(array[i], product[i]);
  }
}

BOOST_AUTO_TEST_CASE(testDivideBinaryAdd) {
  NumaSmall array1{1,2,3, 4};
  NumaSmall array2 = { 5,6,7,8};
  auto product = array1 + array2 / 2;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(i + 1 + double(i + 5) / 2, product[i]);
  }
}


BOOST_AUTO_TEST_CASE(testDivideBinaryMul) {
  NumaSmall array1{1,2,3, 4};
  NumaSmall array2 = { 5,6,7,8};
  auto product = array1 + array2 * 0.5;

  for (size_t i = 0; i < array1.capacity(); i++) {
    BOOST_CHECK_EQUAL(i + 1 + 0.5 * (i + 5), product[i]);
  }
}

BOOST_AUTO_TEST_CASE(testDotDiff) {
  NumaSmall array1{1,2,3, 4};
  NumaSmall array2 = { 5,6,7,8};
  double product = array1.dot(array2);

  BOOST_CHECK_EQUAL(1*5 + 2*6 + 3*7 + 4*8, product);
}

BOOST_AUTO_TEST_CASE(testDotSameRealSquaredAbsolute) {
  NumaSmall array1{1,2,3, 4};
  double product = array1.dot(array1);
  double sqNorm = array1.squared_absolute();

  BOOST_CHECK_EQUAL(product, sqNorm);
}

BOOST_AUTO_TEST_SUITE_END()
