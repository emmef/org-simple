//
// Created by michel on 22-10-20.
//

#include "test-helper.h"
#include <iostream>
#include <org-simple/util/BaseArray.h>
#include <typeinfo>

using namespace org::simple::util;

namespace {

typedef double value;
constexpr size_t SIZE = 10;
typedef ArrayInline<value, SIZE> Array10;
typedef ArraySlice<value> Slice;

template <typename Array, typename Value>
void fillWithIndex(Array &array) {
  for (size_t i = 0; i < array.size(); i++) {
    array[i] = static_cast<Value>(i);
  }
}

template <typename Array, typename Value>
void fillWithZero(Array &array) {
  for (size_t i = 0; i < array.size(); i++) {
    array[i] = static_cast<Value>(0);
  }
}

template <typename Array, typename Value>
void fillWithOne(Array &array) {
  for (size_t i = 0; i < array.size(); i++) {
    array[i] = static_cast<Value>(1);
  }
}

template <typename Array, typename Value>
void fillWithHundred(Array &array) {
  for (size_t i = 0; i < array.size(); i++) {
    array[i] = static_cast<Value>(100);
  }
}

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_BaseArray)

BOOST_AUTO_TEST_CASE(testArray10IsBaseArray) {
  BOOST_CHECK((AboutBaseArray<Array10, double>::isBaseArray));
}

BOOST_AUTO_TEST_CASE(testArray10Size10) {
  Array10 array10;
  BOOST_CHECK_EQUAL(10u, array10.size());
}

BOOST_AUTO_TEST_CASE(testArray10COnstSize10) {
  BOOST_CHECK_EQUAL(10u, Array10::constSize());
}

BOOST_AUTO_TEST_CASE(testArray10HasConstSizeAndAccess) {
  BOOST_CHECK((AboutBaseArray<Array10, double>::isBaseArrayConstSize));
}

BOOST_AUTO_TEST_CASE(testArray10SetAndGetValues) {
  Array10 array;
  for (size_t i = 0; i < array.size(); i++) {
    array[i] = i;
  }
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { i }, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testFillWithIndexes) {
  Array10 array;
  fillWithIndex<Array10, double>(array);
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { i }, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testFillWithOne) {
  Array10 array;
  fillWithZero<Array10, double>(array);
  fillWithOne<Array10, double>(array);
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { 1 }, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testFillWithZero) {
  Array10 array;
  fillWithOne<Array10, double>(array);
  fillWithZero<Array10, double>(array);
  for (size_t i = 0; i < array.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { 0 }, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10SetAndGetValuesViaWrapper) {
  Array10 array10;
  fillWithIndex<Array10, double>(array10);
  BaseArrayWrapper<Array10, double> wrapper(array10);
  BOOST_CHECK_EQUAL(Array10::constSize(), wrapper.size());
  for (size_t i = 0; i < wrapper.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { i }, wrapper[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopy) {
  Array10 source;
  fillWithIndex<Array10, double>(source);
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  BOOST_CHECK(destination.copy(0,source));

  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { i }, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyOffsetPlusSizeTooLarge) {
  Array10 source;
  Array10 destination;
  BOOST_CHECK(!destination.copy(1,source));
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At0) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>, double>(source);
  BOOST_CHECK(destination.copy(0, source));
  size_t i = 0;
  for (; i < source.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { i }, destination[i]);
  }
  for (; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { 0 }, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestStaticCopyArray5At0) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>, double>(source);
  destination.copy<0>(source);
  size_t i = 0;
  for (; i < source.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { i }, destination[i]);
  }
  for (; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(size_t { 0 }, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At5) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>, double>(source);
  BOOST_CHECK(destination.copy(5, source));
  size_t i = 0;
  for (; i < 5; i++) {
    BOOST_CHECK_EQUAL(size_t { 0 }, destination[i]);
  }
  for (size_t value = 0; i < destination.size(); i++, value++) {
    BOOST_CHECK_EQUAL(value, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestStaticCopyArray5At5) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>, double>(source);
  destination.copy<5>(source);
  size_t i = 0;
  for (; i < 5; i++) {
    BOOST_CHECK_EQUAL(size_t { 0 }, destination[i]);
  }
  for (size_t value = 0; i < destination.size(); i++, value++) {
    BOOST_CHECK_EQUAL(value, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At6Fails) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>, double>(source);
  BOOST_CHECK(!destination.copy(6, source));
}

BOOST_AUTO_TEST_CASE(testArray10CopyRangeFailStartAboveEndMustFail) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  BOOST_CHECK(!destination.copy_range(0, source, 4, 3));
}

BOOST_AUTO_TEST_CASE(testArray10CopyRangeFailEndIsSizeMustFail) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  BOOST_CHECK(!destination.copy_range(0, source, 4, source.size()));
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at0) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  fillWithIndex<Array10, double>(source);
  BOOST_CHECK(destination.copy_range(0, source, 4, 6));
  double expected[] = {4,5,6,0,0,0,0,0,0,0};
  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyStaticRange4to6at0) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  fillWithIndex<Array10, double>(source);
  destination.copy_range<0,4,6>(source);
  double expected[] = {4,5,6,0,0,0,0,0,0,0};
  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at3) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  fillWithIndex<Array10, double>(source);
  BOOST_CHECK(destination.copy_range(3, source, 4, 6));
  double expected[] = {0,0,0,4,5,6,0,0,0,0};
  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyStaticRange4to6at3) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  fillWithIndex<Array10, double>(source);
  destination.copy_range<3, 4, 6>(source);
  double expected[] = {0,0,0,4,5,6,0,0,0,0};
  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at7) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  fillWithIndex<Array10, double>(source);
  BOOST_CHECK(destination.copy_range(7, source, 4, 6));
  double expected[] = {0,0,0,0,0,0,0,4,5,6};
  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyStaticRange4to6at7) {
  Array10 destination;
  fillWithZero<Array10, double>(destination);
  Array10 source;
  fillWithIndex<Array10, double>(source);
  destination.copy_range<7, 4, 6>(source);
  double expected[] = {0,0,0,0,0,0,0,4,5,6};
  for (size_t i = 0; i < destination.size(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArrayRangeCopy4To6) {
  Array10 source;
  fillWithIndex<Array10, double>(source);
  auto slice = source.range_copy(4, 6);
  size_t size = slice.size();
  BOOST_CHECK_EQUAL(3, size);
  for (size_t i = 0, x = 4; i < size; i++, x++) {
    BOOST_CHECK_EQUAL(x, slice[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArrayStaticRangeCopy4To6) {
  Array10 source;
  fillWithIndex<Array10, double>(source);
  auto slice = source.range_copy<4, 6>();
  size_t size = slice.size();
  BOOST_CHECK_EQUAL(3, size);
  for (size_t i = 0, x = 4; i < size; i++, x++) {
    BOOST_CHECK_EQUAL(x, slice[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSliceIsBaseArray) {
  BOOST_CHECK((AboutBaseArray<Slice, double>::isBaseArray));
}

BOOST_AUTO_TEST_SUITE_END()
