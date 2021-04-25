//
// Created by michel on 26-09-20.
//

#include "test-helper.h"
#include <org-simple/core/Size.h>

using namespace org::simple::core;

using Metric = SizeMetricBase<size_t>;
using Functions = org::simple::test::FunctionTestCases;
using TestCase = org::simple::test::AbstractValueTestCase;

namespace {

struct WithinTests {
  static TestCase *createWithin(bool expected, size_t value, size_t min,
                                size_t max) {
    return Functions::create("is_within", is_within, expected, value, min, max);
  }
  static TestCase *createWithinExcl(bool expected, size_t value, size_t min,
                                    size_t max) {
    return Functions::create("is_within_excl", is_within_excl, expected, value,
                             min, max);
  }
};

std::vector<org::simple::test::AbstractValueTestCase *> *generateTestCases() {
  constexpr size_t MAX_LIMIT = Metric::limit;
  constexpr size_t MAX_INDEX = MAX_LIMIT - 1;
  constexpr size_t MAX_MASK = Bits<size_t>::bit_mask_not_exceeding(MAX_INDEX);

  std::vector<size_t> singleValues;
  singleValues.push_back(0);
  singleValues.push_back(1);
  singleValues.push_back(2);
  singleValues.push_back(3);
  singleValues.push_back(MAX_LIMIT - 1);
  singleValues.push_back(MAX_LIMIT);
  singleValues.push_back(MAX_LIMIT + 1);

  struct Pair {
    size_t v1;
    size_t v2;
  };

  std::vector<Pair> productValues;
  for (size_t i = 0; i <= MAX_LIMIT + 1; i++) {
    for (size_t j = i; j <= MAX_LIMIT + 1; j++) {
      size_t product = j * i;
      if (product < 4 || (product > MAX_LIMIT - 2 && product < MAX_LIMIT + 2)) {
        productValues.push_back({i, j});
        productValues.push_back({j, i});
      }
    }
  }

  std::vector<Pair> sumValues;
  for (size_t i = 0; i <= MAX_LIMIT + 1; i++) {
    for (size_t j = i; j <= MAX_LIMIT + 1; j++) {
      size_t sum = j + i;
      if (sum < 4 || (sum > MAX_LIMIT - 2 && sum < MAX_LIMIT + 2)) {
        productValues.push_back({i, j});
        productValues.push_back({j, i});
      }
    }
  }

  auto testCases =
      new std::vector<org::simple::test::AbstractValueTestCase *>();

  for (size_t i : singleValues) {
    bool isWithin = i >= 0 && i <= MAX_LIMIT;
    bool isWithinExcl = i > 0 && i < MAX_LIMIT;
    bool isValidSize = i > 0 && i <= MAX_LIMIT;
    bool isValidIndex = i >= 0 && i <= MAX_INDEX;
    bool isValidMask = i <= MAX_MASK && Bits<size_t>::fill(i) == i;

    testCases->push_back(
        WithinTests::createWithinExcl(isWithinExcl, i, 0, MAX_LIMIT));

    testCases->push_back(WithinTests::createWithin(isWithin, i, 0, MAX_LIMIT));

    testCases->push_back(Functions::create(
        "Metric::IsValid::value", Metric::IsValid::value, isValidSize, i));
    testCases->push_back(Functions::create(
        "Metric::IsValid::index", Metric::IsValid::index, isValidIndex, i));
    testCases->push_back(Functions::create(
        "Metric::IsValid::mask", Metric::IsValid::mask, isValidMask, i));

    if (isValidSize) {
      testCases->push_back(Functions::create(
          "Metric::Valid::value", Metric::Valid::value<size_t>, i, i));
    } else {
      testCases->push_back(Functions::create("Size::check::valid_value",
                                             Metric::Valid::value<size_t>, i));
    }
    if (isValidIndex) {
      testCases->push_back(Functions::create(
          "Metric::check::valid_index", Metric::Valid::index<size_t>, i, i));
    } else {
      testCases->push_back(Functions::create("Size::check::valid_index",
                                             Metric::Valid::index<size_t>, i));
    }
    if (isValidMask) {
      testCases->push_back(Functions::create(
          "Metric::check::valid_mask", Metric::Valid::mask<size_t>, i, i));
    } else {
      testCases->push_back(Functions::create("Size::check::valid_mask",
                                             Metric::Valid::mask<size_t>, i));
    }
  }

  for (Pair &i : productValues) {
    size_t product = i.v1 * i.v2;
    bool isValidSizeProduct = product > 0 && product <= MAX_LIMIT;

    testCases->push_back(Functions::create("Metric::check::product",
                                           Metric::IsValid::product,
                                           isValidSizeProduct, i.v1, i.v2));
    testCases->push_back(Functions::create("Metric::check::product",
                                           Metric::IsValid::product,
                                           isValidSizeProduct, i.v2, i.v1));
    if (isValidSizeProduct) {
      testCases->push_back(Functions::create("Metric::check::valid_product",
                                             Metric::Valid::product<size_t>,
                                             product, i.v1, i.v2));
      testCases->push_back(Functions::create("Metric::check::valid_product",
                                             Metric::Valid::product<size_t>,
                                             product, i.v2, i.v1));
    } else {
      testCases->push_back(Functions::create("Metric::check::valid_product",
                                             Metric::Valid::product<size_t>,
                                             i.v1, i.v2));
      testCases->push_back(Functions::create("Metric::check::valid_product",
                                             Metric::Valid::product<size_t>,
                                             i.v2, i.v1));
    }
  }

  for (Pair &i : sumValues) {
    size_t sum = i.v1 * i.v2;
    bool isValidSizeSum = sum > 0 && sum <= MAX_LIMIT;

    testCases->push_back(Functions::create("Metric::check::sum",
                                           Metric::IsValid::sum, isValidSizeSum,
                                           i.v1, i.v2));
    testCases->push_back(Functions::create("Metric::check::sum",
                                           Metric::IsValid::sum, isValidSizeSum,
                                           i.v2, i.v1));
    if (isValidSizeSum) {
      testCases->push_back(Functions::create(
          "Metric::check::valid_product", Metric::Valid::sum, sum, i.v1, i.v2));
      testCases->push_back(Functions::create(
          "Metric::check::valid_product", Metric::Valid::sum, sum, i.v2, i.v1));
    } else {
      testCases->push_back(Functions::create("Metric::check::valid_product",
                                             Metric::Valid::sum<size_t>, i.v1,
                                             i.v2));
      testCases->push_back(Functions::create("Metric::check::valid_product",
                                             Metric::Valid::sum<size_t>, i.v2,
                                             i.v1));
    }
  }

  return testCases;
} // namespace

class TestGenerator {

  std::vector<org::simple::test::AbstractValueTestCase *> *testCases;

public:
  TestGenerator() { testCases = generateTestCases(); }

  ~TestGenerator() {
    if (testCases) {
      for (auto testCase : *testCases) {
        delete testCase;
      }
      delete testCases;
      testCases = nullptr;
    }
  }

  [[nodiscard]] std::vector<org::simple::test::AbstractValueTestCase *>
  getTestCases() const {
    return *testCases;
  }
};
} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_core_SizeMetric)

BOOST_AUTO_TEST_CASE(testSizeMetricBaseValues) {
  typedef SizeMetricBase<size_t> Size;

  BOOST_CHECK_EQUAL(Size::limit, ~size_t(0));
  BOOST_CHECK_EQUAL(get_max_index_value(Size::limit), ~size_t(0) - 1);
  BOOST_CHECK_EQUAL(get_max_mask_from_size(Size::limit), ~size_t(0) >> 1);
}

BOOST_AUTO_TEST_CASE(testSizeMetricValues) {
  typedef SizeMetricBase<size_t> Size;

  BOOST_CHECK_EQUAL(Size::limit, ~size_t(0));
  BOOST_CHECK_EQUAL(get_max_index_value(Size::limit), ~size_t(0) - 1);
  BOOST_CHECK_EQUAL(get_max_mask_from_size(Size::limit), ~size_t(0) >> 1);
}

BOOST_AUTO_TEST_CASE(testSizeMetricElementValues) {
  typedef SizeMetricBase<size_t> Size;
  static constexpr size_t element_size = 13;
  typedef Size::Elements<element_size> Elements;
  BOOST_CHECK_EQUAL(Elements::limit, ~size_t(0) / element_size);
  static constexpr size_t group_size = 13;
  typedef Elements::Elements<group_size> Groups;
  BOOST_CHECK_EQUAL(Groups::limit, ~size_t(0) / element_size / group_size);
}

BOOST_AUTO_TEST_CASE(testBitLimitedSizeMetric16) {
  typedef SizeMetricBase<size_t, get_size_for_size_bits<size_t, 16>()> Size;
  BOOST_CHECK_EQUAL(get_size_for_size_bits<size_t>(16), 0x10000);
  BOOST_CHECK_EQUAL(Size::limit, 0x10000);
  BOOST_CHECK_EQUAL(get_max_index_value(Size::limit), 0xffff);
  BOOST_CHECK_EQUAL(get_max_mask_from_size(Size::limit), 0xffff);
}

BOOST_AUTO_TEST_CASE(testBitLimitedSizeMetricAll) {
  typedef SizeMetricBase<size_t, get_size_for_size_bits<size_t, sizeof(size_t) * 8>()> Size;
  BOOST_CHECK_EQUAL(Size::limit, ~size_t(0));
  BOOST_CHECK_EQUAL(get_max_index_value(Size::limit), Size::limit - 1);
  BOOST_CHECK_EQUAL(get_max_mask_from_size(Size::limit), Size::limit >> 1);
}

BOOST_AUTO_TEST_CASE(testLimitedSizeMetric) {
  typedef SizeMetricBase<size_t, 48000> Size;
  size_t max = 48000;
  size_t mask = Bits<size_t>::bit_mask_not_exceeding(48000);
  size_t index = max - 1;

  BOOST_CHECK_EQUAL(Size::limit, max);
  BOOST_CHECK_EQUAL(get_max_index_value(Size::limit), index);
  BOOST_CHECK_EQUAL(get_max_mask_from_size(Size::limit), mask);
}

BOOST_AUTO_TEST_CASE(testPlus) {
  typedef SizeMetricBase<size_t, 48000> Size;
  auto result = Size::times<15u, Size::plus<13u, 16u>()>();
  BOOST_CHECK_EQUAL(result, 15 * (13u + 16u));
}

BOOST_AUTO_TEST_CASE(sizeTestcaseToBeMovedToSeparateSizeTestCasesModule) {
  static constexpr size_t SIZE1 = 13;
  static constexpr size_t SIZE2 = 17;
  static constexpr size_t SUM = SIZE1 + SIZE2;
  static constexpr size_t PRODUCT = SIZE1 * SIZE2;

  typedef SizeMetricBase<size_t> Size;
  BOOST_CHECK_EQUAL(PRODUCT, Size::product(SIZE1, SIZE2));
  BOOST_CHECK_EQUAL(SUM, Size::sum(SIZE1, SIZE2));

  typedef SizeMetricBase<size_t, 100> Limited;
  BOOST_CHECK_EQUAL(SUM, Limited::sum(SIZE1, SIZE2));
  BOOST_CHECK_THROW(Limited::product(SIZE1, SIZE2), std::invalid_argument);
}

TestGenerator TEST_GENERATOR;

BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.getTestCases()) {
  sample->test(); }

BOOST_AUTO_TEST_SUITE_END()
