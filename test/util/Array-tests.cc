//
// Created by michel on 22-10-20.
//

#include "test-helper.h"
#include <iostream>
#include <org-simple/Array.h>
#include <typeinfo>

using namespace org::simple;

namespace {

typedef double value;
static constexpr size_t SIZE = 10;
typedef Array<value, SIZE> Array10;
typedef ArrayDataRef<value> Slice;

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
  typedef concept_base_array<Array> Test;
  if (Test::FIXED_CAPACITY == 0) {
    return;
  }
  typedef typename Test::data_type data_type;
  static constexpr size_t DATA_SIZE = sizeof(data_type) * Test::FIXED_CAPACITY;
  static constexpr size_t ALIGNMENT = std::bit_ceil(std::max(static_cast<size_t>(1u), Test::ALIGNAS));
  static constexpr size_t SIZE = ALIGNMENT * (1 + (DATA_SIZE - 1) / ALIGNMENT);

  BOOST_CHECK_GE(sizeof(Array), sizeof(data_type));
  BOOST_CHECK_GE(sizeof(Array), DATA_SIZE);
  BOOST_CHECK_GE(sizeof(Array), Test::ALIGNAS);
  BOOST_CHECK_EQUAL(sizeof(Array), SIZE);

  BOOST_CHECK_GE(alignof(data_type) * Test::ALIGNAS, alignof(data_type));
  BOOST_CHECK_LE(sizeof(data_type), alignof(data_type) * Test::ALIGNAS);
  BOOST_CHECK_EQUAL(Test::ALIGNAS ? sizeof(Array) % Test::ALIGNAS : Test::ALIGNAS, 0);
  BOOST_CHECK_EQUAL(sizeof(Array) % alignof(data_type), 0);
}

template <class Array>
void testAlignmentValuesDataStruct() {
  typedef concept_base_array<Array> Test;
  if (Test::FIXED_CAPACITY == 0) {
    return;
  }
  typedef typename Test::data_type data_type;
  static constexpr size_t DATA_SIZE = sizeof(data_type) * (Test::FIXED_CAPACITY ? std::bit_ceil(Test::FIXED_CAPACITY) : 1);
  static constexpr size_t ALIGNMENT = std::max(static_cast<size_t>(1u), Test::ALIGNAS);
  static constexpr size_t SIZE = ALIGNMENT * (1 + (DATA_SIZE - 1) / ALIGNMENT);

  BOOST_CHECK_GE(sizeof(typename Array::DataStruct), sizeof(data_type));
  BOOST_CHECK_GE(sizeof(typename Array::DataStruct), DATA_SIZE);
  BOOST_CHECK_GE(sizeof(typename Array::DataStruct), Test::ALIGNAS);
  BOOST_CHECK_EQUAL(sizeof(typename Array::DataStruct), SIZE);

  BOOST_CHECK_GE(alignof(data_type) * Test::ALIGNAS, alignof(data_type));
  BOOST_CHECK_LE(sizeof(data_type), alignof(data_type) * Test::ALIGNAS);
  BOOST_CHECK_EQUAL(Test::ALIGNAS ? sizeof(typename Array::DataStruct) % Test::ALIGNAS : Test::ALIGNAS, 0);
  BOOST_CHECK_EQUAL(sizeof(typename Array::DataStruct) % alignof(data_type), 0);
}

template <size_t SZ, size_t ALIGN>
void testAlignmentValuesHeap() {
  ArrayAllocated<int32_t, ALIGN> heap(SZ);

  typedef concept_base_array<ArrayAllocated<int32_t, ALIGN>> Test;
  typedef typename Test::data_type data_type;

  BOOST_CHECK_EQUAL(0, Test::FIXED_CAPACITY);

  auto ptr = heap.begin();
  size_t offs = reinterpret_cast<std::uintptr_t>(ptr);

  BOOST_CHECK_EQUAL(0, Test::ALIGNAS ? offs % Test::ALIGNAS: Test::ALIGNAS);
}

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_BaseArray)

BOOST_AUTO_TEST_CASE(testArray10IsBaseArray) {
  BOOST_CHECK((concept_base_array<Array10>::value));
}

BOOST_AUTO_TEST_CASE(testArray10Size10) {
  Array10 array10;
  BOOST_CHECK_EQUAL(10u, array10.capacity());
}

BOOST_AUTO_TEST_CASE(testArray10COnstSize10) {
  BOOST_CHECK_EQUAL(10u, Array10::FIXED_CAPACITY);
}

BOOST_AUTO_TEST_CASE(testCompatibility) {
  typedef ArrayDataRef<TestA *> ASlice;
  typedef ArrayDataRef<TestB *> BSlice;
  typedef concept_base_array<ASlice> ATest;
  typedef concept_base_array<BSlice> BTest;

  BOOST_CHECK_EQUAL(true, ATest::is_type_compatible<ATest::data_type>);
  BOOST_CHECK_EQUAL(true, BTest::is_type_compatible<BTest::data_type>);
  BOOST_CHECK_EQUAL(true, BTest::is_type_compatible<ATest::data_type>);
  BOOST_CHECK_EQUAL(false, ATest::is_type_compatible<BTest::data_type>);
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
  BOOST_CHECK_NO_THROW(destination.copy_to(0, source));

  for (size_t i = 0; i < destination.capacity(); i++) {
    BOOST_CHECK_EQUAL(size_t{i}, destination[i]);
  }
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyOffsetPlusSizeTooLarge) {
  Array10 source;
  Array10 destination;
  BOOST_CHECK_THROW(destination.copy_to(1, source), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testArray10TestCopyArray5At0) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array<double, 5> source;
  fillWithIndex<Array<double, 5>>(source);
  BOOST_CHECK_NO_THROW(destination.copy_to(0, source));
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
  Array<double, 5> source;
  fillWithIndex<Array<double, 5>>(source);
  destination.copy_to<0>(source);
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
  Array<double, 5> source;
  fillWithIndex<Array<double, 5>>(source);
  BOOST_CHECK_NO_THROW(destination.copy_to(5, source));
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
  Array<double, 5> source;
  fillWithIndex<Array<double, 5>>(source);
  destination.copy_to<5>(source);
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
  Array<double, 5> source;
  fillWithIndex<Array<double, 5>>(source);
  BOOST_CHECK_THROW(destination.copy_to(6, source), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testArray10CopyRangeFailStartAboveEndMustFail) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  BOOST_CHECK_THROW(destination.copy_range_to(0, source, 4, 3), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testArray10CopyRangeFailEndIsSizeMustFail) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  BOOST_CHECK_THROW(destination.copy_range_to(0, source, 4, source.capacity()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testArray10CopyRange4to6at0) {
  Array10 destination;
  fillWithZero<Array10>(destination);
  Array10 source;
  fillWithIndex<Array10>(source);
  BOOST_CHECK_NO_THROW(destination.copy_range_to(0, source, 4, 6));
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
  destination.copy_range_to<0, 4, 6>(source);
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
  BOOST_CHECK_NO_THROW(destination.copy_range_to(3, source, 4, 6));
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
  destination.copy_range_to<3, 4, 6>(source);
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
  BOOST_CHECK_NO_THROW(destination.copy_range_to(7, source, 4, 6));
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
  destination.copy_range_to<7, 4, 6>(source);
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
  auto slice = source.range_ref<4, 6>();
  size_t size = slice.capacity();
  BOOST_CHECK_EQUAL(3, size);
  for (size_t i = 0, x = 4; i < size; i++, x++) {
    BOOST_CHECK_EQUAL(x, slice[i]);
  }
}

BOOST_AUTO_TEST_CASE(testSliceIsBaseArray) {
  BOOST_CHECK((concept_base_array<Slice>::value));
}

template <typename T> static constexpr size_t effective_alignment(size_t A) {
  return A == 0 || !std::has_single_bit(A) ? 0 : A;
}

BOOST_AUTO_TEST_CASE(testTestArrayBase) {
  using Fix = Array<int, 4, 8>; // was 7 as alignment, which is invalid
  BOOST_CHECK_EQUAL(true, concept_base_array<Fix>::value);
  BOOST_CHECK_EQUAL(true, concept_base_array<Fix::Super>::value);
  BOOST_CHECK_EQUAL(false, concept_base_array<A>::value);
  BOOST_CHECK_EQUAL(sizeof(int) * 8, sizeof(Fix));
}


BOOST_AUTO_TEST_CASE(testArrayInline_1_0) {
  testAlignmentValues<Array<int32_t, 1, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_0) {
  testAlignmentValues<Array<int32_t, 4, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_0) {
  testAlignmentValues<Array<int32_t, 7, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_0) {
  testAlignmentValues<Array<int32_t, 8, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_0) {
  testAlignmentValues<Array<int32_t, 9, 0>>();
}


BOOST_AUTO_TEST_CASE(testArrayInline_1_1) {
  testAlignmentValues<Array<int32_t, 1, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_1) {
  testAlignmentValues<Array<int32_t, 4, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_1) {
  testAlignmentValues<Array<int32_t, 7, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_1) {
  testAlignmentValues<Array<int32_t, 8, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_1) {
  testAlignmentValues<Array<int32_t, 9, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_1_4) {
  testAlignmentValues<Array<int32_t, 1, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_4) {
  testAlignmentValues<Array<int32_t, 4, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_4) {
  testAlignmentValues<Array<int32_t, 7, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_4) {
  testAlignmentValues<Array<int32_t, 8, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_4) {
  testAlignmentValues<Array<int32_t, 9, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_1_32) {
  testAlignmentValues<Array<int32_t, 1, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_4_32) {
  testAlignmentValues<Array<int32_t, 4, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_7_32) {
  testAlignmentValues<Array<int32_t, 7, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_8_32) {
  testAlignmentValues<Array<int32_t, 8, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayInline_9_32) {
  testAlignmentValues<Array<int32_t, 9, 32>>();
}


//////////////////////


BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_0) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 1, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_0) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 4, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_0) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 7, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_0) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 8, 0>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_0) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 9, 0>>();
}


BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_1) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 1, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_1) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 4, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_1) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 7, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_1) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 8, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_1) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 9, 1>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_4) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 1, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_4) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 4, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_4) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 7, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_4) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 8, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_4) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 9, 4>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_1_32) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 1, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_4_32) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 4, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_7_32) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 7, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_8_32) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 8, 32>>();
}

BOOST_AUTO_TEST_CASE(testArrayConstAlloc_9_32) {
  testAlignmentValuesDataStruct<ArrayAllocatedFixedSize<int32_t, 9, 32>>();
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
