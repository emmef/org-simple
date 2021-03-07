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
static constexpr size_t SIZE = 10;
typedef ArrayInline<value, SIZE> Array10;
typedef ArraySlice<value> Slice;

class TestA {};

class TestB : public TestA {};

template <typename Array> void fillWithIndex(Array &array) {
  for (size_t i = 0; i < array.capacity(); i++) {
    array[i] = static_cast<typename Array::data_type>(i);
  }
}

template <typename Array> void fillWithZero(Array &array) {
  for (size_t i = 0; i < array.capacity(); i++) {
    array[i] = static_cast<typename Array::data_type>(0);
  }
}

template <typename Array> void fillWithOne(Array &array) {
  for (size_t i = 0; i < array.capacity(); i++) {
    array[i] = static_cast<typename Array::data_type>(1);
  }
}

template <typename Array> void fillWithHundred(Array &array) {
  for (size_t i = 0; i < array.capacity(); i++) {
    array[i] = static_cast<typename Array::data_type>(100);
  }
}
struct A {};

template <class Array>
void testAlignmentValues() {
  typedef BaseArrayTest<Array> Test;
  if (Test::FIXED_CAPACITY == 0) {
    return;
  }
  typedef typename Test::data_type data_type;
  static constexpr size_t DATA_SIZE = sizeof(data_type) * Test::FIXED_CAPACITY;
  static constexpr size_t SIZE = Test::ALIGNAS * (1 + (DATA_SIZE - 1) / Test::ALIGNAS);

  BOOST_CHECK_GE(sizeof(Array), sizeof(data_type));
  BOOST_CHECK_GE(sizeof(Array), DATA_SIZE);
  BOOST_CHECK_GE(sizeof(Array), Test::ALIGNAS);
  BOOST_CHECK_EQUAL(sizeof(Array), SIZE);

  BOOST_CHECK_GE(Test::ALIGNAS, alignof(data_type));
  BOOST_CHECK_LE(sizeof(data_type), Test::ALIGNAS);
  BOOST_CHECK_EQUAL(sizeof(Array) % Test::ALIGNAS, 0);
  BOOST_CHECK_EQUAL(sizeof(Array) % alignof(data_type), 0);
}

template <class Array>
void testAlignmentValuesDataStruct() {
  typedef BaseArrayTest<Array> Test;
  if (Test::FIXED_CAPACITY == 0) {
    return;
  }
  typedef typename Test::data_type data_type;
  static constexpr size_t DATA_SIZE = sizeof(data_type) * Test::FIXED_CAPACITY;
  static constexpr size_t SIZE = Test::ALIGNAS * (1 + (DATA_SIZE - 1) / Test::ALIGNAS);

  BOOST_CHECK_GE(sizeof(typename Array::DataStruct), sizeof(data_type));
  BOOST_CHECK_GE(sizeof(typename Array::DataStruct), DATA_SIZE);
  BOOST_CHECK_GE(sizeof(typename Array::DataStruct), Test::ALIGNAS);
  BOOST_CHECK_EQUAL(sizeof(typename Array::DataStruct), SIZE);

  BOOST_CHECK_GE(Test::ALIGNAS, alignof(data_type));
  BOOST_CHECK_LE(sizeof(data_type), Test::ALIGNAS);
  BOOST_CHECK_EQUAL(sizeof(typename Array::DataStruct) % Test::ALIGNAS, 0);
  BOOST_CHECK_EQUAL(sizeof(typename Array::DataStruct) % alignof(data_type), 0);
}

template <size_t SZ, size_t ALIGN>
void testAlignmentValuesHeap() {
  ArrayHeap<int32_t, ALIGN> heap(SZ);

  typedef BaseArrayTest<ArrayHeap<int32_t, ALIGN>> Test;
  typedef typename Test::data_type data_type;

  BOOST_CHECK_EQUAL(0, Test::FIXED_CAPACITY);
  BOOST_CHECK_EQUAL(org::simple::core::Alignment<int32_t>::get_valid(ALIGN), Test::ALIGNAS);
  BOOST_CHECK_GE(Test::ALIGNAS, alignof(data_type));

  auto ptr = heap.begin();
  size_t offs = (const char *)ptr - (const char *)nullptr;

  BOOST_CHECK_EQUAL(0, offs % Test::ALIGNAS);
}

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_BaseArray)

BOOST_AUTO_TEST_CASE(testArray10IsBaseArray) {
  BOOST_CHECK((BaseArrayTest<Array10>::value));
}

BOOST_AUTO_TEST_CASE(testArray10Size10) {
  Array10 array10;
  BOOST_CHECK_EQUAL(10u, array10.capacity());
}

BOOST_AUTO_TEST_CASE(testArray10COnstSize10) {
  BOOST_CHECK_EQUAL(10u, Array10::FIXED_CAPACITY);
}

BOOST_AUTO_TEST_CASE(testCompatibility) {
  typedef ArraySlice<TestA *> ASlice;
  typedef ArraySlice<TestB *> BSlice;
  typedef BaseArrayTest<ASlice> ATest;
  typedef BaseArrayTest<BSlice> BTest;

  BOOST_CHECK_EQUAL(true, ATest::compatible<ATest::data_type>());
  BOOST_CHECK_EQUAL(true, BTest::compatible<BTest::data_type>());
  BOOST_CHECK_EQUAL(true, BTest::compatible<ATest::data_type>());
  BOOST_CHECK_EQUAL(false, ATest::compatible<BTest::data_type>());
}

BOOST_AUTO_TEST_CASE(testArray10SetAndGetValues) {
  Array10 array;
  for (size_t i = 0; i < array.capacity(); i++) {
    array[i] = i;
  }
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{i}, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testFillWithIndexes) {
  Array10 array;
  fillWithIndex<Array10>(array);
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{i}, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testFillWithOne) {
  Array10 array;
  fillWithZero<Array10>(array);
  fillWithOne<Array10>(array);
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{1}, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testFillWithZero) {
  Array10 array;
  fillWithOne<Array10>(array);
  fillWithZero<Array10>(array);
  for (size_t i = 0; i < array.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{0}, array[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopy) {
  Array10 source;
  fillWithIndex<Array10>(source);
  Array10 destination;
  fillWithZero<Array10>(destination);
  BOOST_CHECK(destination.copy(0, source));

  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{i}, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyOffsetPlusSizeTooLarge) {
  Array10 source;
  Array10 destination;
  BOOST_CHECK(!destination.copy(1, source));
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At0) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>>(source);
  BOOST_CHECK(destination.copy(0, source));
  size_t i = 0;
  for (; i < source.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{i}, destination[i]);
  }
  for (; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{0}, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestStaticCopyArray5At0) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>>(source);
  destination.copy<0>(source);
  size_t i = 0;
  for (; i < source.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{i}, destination[i]);
  }
  for (; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{0}, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At5) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>>(source);
  BOOST_CHECK(destination.copy(5, source));
  size_t i = 0;
  for (; i < 5; i++) {
    BOOST_CHECK_EQUAL(size_t{0}, destination[i]);
  }
  for (size_t value = 0; i < destination.capacity(); i++, value++) {
    BOOST_CHECK_EQUAL(value, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestStaticCopyArray5At5) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>>(source);
  destination.copy<5>(source);
  size_t i = 0;
  for (; i < 5; i++) {
    BOOST_CHECK_EQUAL(size_t{0}, destination[i]);
  }
  for (size_t value = 0; i < destination.capacity(); i++, value++) {
    BOOST_CHECK_EQUAL(value, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At6Fails) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  ArrayInline<double, 5> source;
  fillWithIndex<ArrayInline<double, 5>>(source);
  BOOST_CHECK(!destination.copy(6, source));
}

BOOST_AUTO_TEST_CASE(testArray10CopyRangeFailStartAboveEndMustFail) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  BOOST_CHECK(!destination.copy_range(0, source, 4, 3));
}

BOOST_AUTO_TEST_CASE(testArray10CopyRangeFailEndIsSizeMustFail) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  BOOST_CHECK(!destination.copy_range(0, source, 4, source.capacity()));
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at0) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  BOOST_CHECK(destination.copy_range(0, source, 4, 6));
  double expected[] = {4, 5, 6, 0, 0, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyStaticRange4to6at0) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  destination.copy_range<0, 4, 6>(source);
  double expected[] = {4, 5, 6, 0, 0, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at3) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  BOOST_CHECK(destination.copy_range(3, source, 4, 6));
  double expected[] = {0, 0, 0, 4, 5, 6, 0, 0, 0, 0};
  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyStaticRange4to6at3) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  destination.copy_range<3, 4, 6>(source);
  double expected[] = {0, 0, 0, 4, 5, 6, 0, 0, 0, 0};
  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at7) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  BOOST_CHECK(destination.copy_range(7, source, 4, 6));
  double expected[] = {0, 0, 0, 0, 0, 0, 0, 4, 5, 6};
  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10CopyStaticRange4to6at7) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  destination.copy_range<7, 4, 6>(source);
  double expected[] = {0, 0, 0, 0, 0, 0, 0, 4, 5, 6};
  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(expected[i], destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArrayRangeCopy4To6) {
  Array10 source;
  fillWithIndex<Array10>(source);
  auto slice = source.range_copy(4, 6);
  size_t size = slice.capacity();
  BOOST_CHECK_EQUAL(3, size);
  for (size_t i = 0, x = 4; i < size; i++, x++) {
    BOOST_CHECK_EQUAL(x, slice[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArrayStaticRangeCopy4To6) {
  Array10 source;
  fillWithIndex<Array10>(source);
  auto slice = source.range_copy<4, 6>();
  size_t size = slice.capacity();
  BOOST_CHECK_EQUAL(3, size);
  for (size_t i = 0, x = 4; i < size; i++, x++) {
    BOOST_CHECK_EQUAL(x, slice[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSliceIsBaseArray) {
  BOOST_CHECK((BaseArrayTest<Slice>::value));
}

template <typename T> static constexpr size_t effective_alignment(size_t A) {
  return org::simple::core::Alignment<T>::is_valid(A) ? A : 0;
}

BOOST_AUTO_TEST_CASE(testTestArrayBase) {
  using Fix = ArrayInline<int, 4, 7>;
  BOOST_CHECK_EQUAL(true, BaseArrayTest<Fix>::value);
  BOOST_CHECK_EQUAL(true, BaseArrayTest<Fix::Super>::value);
  BOOST_CHECK_EQUAL(false, BaseArrayTest<A>::value);
  BOOST_CHECK_EQUAL(sizeof(int) * 4, sizeof(Fix));
}


BOOST_AUTO_TEST_CASE(testArrayInline_1_0) {
  testAlignmentValues<ArrayInline<int32_t, 1, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_0) {
  testAlignmentValues<ArrayInline<int32_t, 4, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_0) {
  testAlignmentValues<ArrayInline<int32_t, 7, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_0) {
  testAlignmentValues<ArrayInline<int32_t, 8, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_0) {
  testAlignmentValues<ArrayInline<int32_t, 9, 0>>();
}


BOOST_AUTO_TEST_CASE(testArrayInline_1_1) {
  testAlignmentValues<ArrayInline<int32_t, 1, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_1) {
  testAlignmentValues<ArrayInline<int32_t, 4, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_1) {
  testAlignmentValues<ArrayInline<int32_t, 7, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_1) {
  testAlignmentValues<ArrayInline<int32_t, 8, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_1) {
  testAlignmentValues<ArrayInline<int32_t, 9, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_1_4) {
  testAlignmentValues<ArrayInline<int32_t, 1, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_4) {
  testAlignmentValues<ArrayInline<int32_t, 4, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_4) {
  testAlignmentValues<ArrayInline<int32_t, 7, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_4) {
  testAlignmentValues<ArrayInline<int32_t, 8, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_4) {
  testAlignmentValues<ArrayInline<int32_t, 9, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_1_32) {
  testAlignmentValues<ArrayInline<int32_t, 1, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_32) {
  testAlignmentValues<ArrayInline<int32_t, 4, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_32) {
  testAlignmentValues<ArrayInline<int32_t, 7, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_32) {
  testAlignmentValues<ArrayInline<int32_t, 8, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_32) {
  testAlignmentValues<ArrayInline<int32_t, 9, 32>>();
}


//////////////////////


BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_0) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 1, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_0) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 4, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_0) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 7, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_0) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 8, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_0) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 9, 0>>();
}


BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_1) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 1, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_1) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 4, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_1) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 7, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_1) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 8, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_1) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 9, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_4) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 1, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_4) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 4, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_4) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 7, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_4) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 8, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_4) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 9, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_32) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 1, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_32) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 4, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_32) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 7, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_32) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 8, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_32) {
  testAlignmentValuesDataStruct<ArrayConstAlloc<int32_t, 9, 32>>();
}


//////////////////////


BOOST_AUTO_TEST_CASE(testArrayHeap_1_0) {
  testAlignmentValuesHeap<1, 0>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_4_0) {
  testAlignmentValuesHeap<4, 0>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_7_0) {
  testAlignmentValuesHeap<7, 0>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_8_0) {
  testAlignmentValuesHeap<8, 0>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_9_0) {
  testAlignmentValuesHeap<9, 0>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_1_1) {
  testAlignmentValuesHeap<1, 1>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_4_1) {
  testAlignmentValuesHeap<4, 1>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_7_1) {
  testAlignmentValuesHeap<7, 1>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_8_1) {
  testAlignmentValuesHeap<8, 1>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_9_1) {
  testAlignmentValuesHeap<9, 1>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_1_4) {
  testAlignmentValuesHeap<1, 4>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_4_4) {
  testAlignmentValuesHeap<4, 4>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_7_4) {
  testAlignmentValuesHeap<7, 4>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_8_4) {
  testAlignmentValuesHeap<8, 4>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_9_4) {
  testAlignmentValuesHeap<9, 4>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_1_32) {
  testAlignmentValuesHeap<1, 32>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_4_32) {
  testAlignmentValuesHeap<4, 32>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_7_32) {
  testAlignmentValuesHeap<7, 32>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_8_32) {
  testAlignmentValuesHeap<8, 32>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_9_32) {
  testAlignmentValuesHeap<9, 32>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_1_128) {
  testAlignmentValuesHeap<1, 128>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_4_128) {
  testAlignmentValuesHeap<4, 128>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_7_128) {
  testAlignmentValuesHeap<7, 128>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_8_128) {
  testAlignmentValuesHeap<8, 128>();
}

BOOST_AUTO_TEST_CASE(testArrayHeap_9_128) {
  testAlignmentValuesHeap<9, 128>();
}



BOOST_AUTO_TEST_SUITE_END()
