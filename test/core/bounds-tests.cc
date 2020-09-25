//
// Created by michel on 21-09-20.
//

#include "test-helper.h"
#include <cstddef>
#include <org-simple/core/Size.h>

namespace {

constexpr unsigned short SIZE_BITS = 10;
constexpr size_t SIZE_LIMIT = size_t(1) << SIZE_BITS;
using FixedRange = org::simple::core::Size<1, size_t, SIZE_BITS>;

using Functions = org::simple::test::FunctionTestCases;

using TestCase = org::simple::test::AbstractValueTestCase;

struct WithinTests {
  static TestCase *createWithin(bool expected, size_t value, size_t min,
                                size_t max) {
    return Functions::create("is_within", org::simple::core::is_within,
                             expected, value, min, max);
  }
  static TestCase *createWithinExcl(bool expected, size_t value, size_t min,
                                    size_t max) {
    return Functions::create("is_within_excl",
                             org::simple::core::is_within_excl, expected, value,
                             min, max);
  }
};

std::vector<org::simple::test::AbstractValueTestCase *> *generateTestCases() {
  constexpr size_t MAX_LIMIT = FixedRange::Size::max;

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
    bool isValidIndex = i >= 0 && i < MAX_LIMIT;

    testCases->push_back(WithinTests::createWithin(isWithin, i, 0, MAX_LIMIT));
    testCases->push_back(
        WithinTests::createWithinExcl(isWithinExcl, i, 0, MAX_LIMIT));

    testCases->push_back(Functions::create("FixedRange::is_valid",
                                           FixedRange::Size::is_valid<size_t>,
                                           isValidSize, i));

    testCases->push_back(Functions::create(
        "FixedRange::is_valid_index", FixedRange::Size::is_valid_index<size_t>,
        isValidIndex, i));
    if (isValidIndex) {
      testCases->push_back(
          Functions::create("FixedRange::get_valid_index",
                            FixedRange::get_valid_index<size_t>, i, i));
    } else {
      testCases->push_back(
          Functions::create("FixedRange::get_valid_index",
                            FixedRange::get_valid_index<size_t>, i));
    }
    if (isValidSize) {
      testCases->push_back(
          Functions::create("FixedRange::get_valid_size",
                            FixedRange::Size::get_valid<size_t>, i, i));
    } else {
      testCases->push_back(
          Functions::create("FixedRange::get_valid_size",
                            FixedRange::Size::get_valid<size_t>, i));
    }
  }

  for (Pair &i : productValues) {
    size_t product = i.v1 * i.v2;
    bool isValidSizeProduct = product > 0 && product <= MAX_LIMIT;

    testCases->push_back(Functions::create("FixedRange::is_valid_product",
                                           FixedRange::is_valid_product,
                                           isValidSizeProduct, i.v1, i.v2));
    if (isValidSizeProduct) {
      testCases->push_back(Functions::create("FixedRange::get_valid_product",
                                             FixedRange::get_valid_product,
                                             product, i.v1, i.v2));
    } else {
      testCases->push_back(Functions::create("FixedRange::get_valid_product",
                                             FixedRange::get_valid_product,
                                             i.v1, i.v2));
    }
  }

  for (Pair &i : sumValues) {
    size_t sum = i.v1 * i.v2;
    bool isValidSizeSum = sum > 0 && sum <= MAX_LIMIT;

    testCases->push_back(Functions::create("FixedRange::is_valid_sum",
                                           FixedRange::is_valid_sum,
                                           isValidSizeSum, i.v1, i.v2));
    if (isValidSizeSum) {
      testCases->push_back(Functions::create("FixedRange::get_valid_sum",
                                             FixedRange::get_valid_sum, sum,
                                             i.v1, i.v2));
    } else {
      testCases->push_back(Functions::create(
          "FixedRange::get_valid_sum", FixedRange::get_valid_sum, i.v1, i.v2));
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

BOOST_AUTO_TEST_SUITE(org_simple_bounds)

BOOST_AUTO_TEST_CASE(testConstructorExactMaxSize) {
  FixedRange size(SIZE_LIMIT);
  BOOST_CHECK_EQUAL(size_t(size), SIZE_LIMIT);
}

BOOST_AUTO_TEST_CASE(testConstructorValidSize) {
  BOOST_CHECK_EQUAL(FixedRange(3u).get(), 3u);
}

BOOST_AUTO_TEST_CASE(testConstructorTooLargeSize) {
  BOOST_CHECK_THROW(FixedRange(SIZE_LIMIT + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testConstructorZeroSize) {
  BOOST_CHECK_THROW(FixedRange(0u), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testAdditionValid) {
  size_t v1 = 5;
  size_t v2 = 128;
  size_t sum = v1 + v2;
  FixedRange size(v1);

  BOOST_CHECK_EQUAL(size_t(size + v2), sum);
}

BOOST_AUTO_TEST_CASE(testAdditionTooLarge) {
  size_t v1 = 900;
  size_t v2 = 128;
  FixedRange size(v1);
  FixedRange result(1u);

  BOOST_CHECK_THROW(result = size + v2, std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testProductValid) {
  size_t v1 = 5;
  size_t v2 = 128;
  size_t product = v1 * v2;
  FixedRange size(v1);

  BOOST_CHECK_EQUAL(size_t(size * v2), product);
}

BOOST_AUTO_TEST_CASE(testProductTooLarge) {
  size_t v1 = 900;
  size_t v2 = 128;
  FixedRange size(v1);
  FixedRange result(1u);

  BOOST_CHECK_THROW(result = size * v2, std::invalid_argument);
}

TestGenerator TEST_GENERATOR;

BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.getTestCases()) { sample->test(); }

BOOST_AUTO_TEST_SUITE_END()
