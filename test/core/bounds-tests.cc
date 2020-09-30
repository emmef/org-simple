//
// Created by michel on 21-09-20.
//

#include "test-helper.h"
#include <cstddef>
#include <org-simple/core/Size.h>
#include <valarray>

namespace {

using Functions = org::simple::test::FunctionTestCases;
using TestCase = org::simple::test::AbstractValueTestCase;

template <typename T> std::vector<T> generateTestValues(T min, T max) {
  static constexpr T lowest = std::numeric_limits<T>::lowest();
  static constexpr T highest = std::numeric_limits<T>::max();

  std::vector<T> values;
  if constexpr (!std::is_floating_point_v<T>) {
    switch (min) {
    case lowest:
      break;
    case lowest + T(1):
      values.push_back(lowest);
      // FALL-THROUGH
    default:
      values.push_back(min - 1);
    }
  }
  values.push_back(min);
  values.push_back(min + 1);
  values.push_back(max - 1);
  values.push_back(max);
  if constexpr (!std::is_floating_point_v<T>) {
    switch (max) {
    case highest:
      break;
    case highest - T(1):
      values.push_back(highest);
      // FALL-THROUGH
    default:
      values.push_back(max + 1);
    }
  }
  return values;
}

template <bool> struct IntermediateTypeSelectorBase;

template <> struct IntermediateTypeSelectorBase<true> {
  typedef long double intermediate;
};

template <> struct IntermediateTypeSelectorBase<false> {
  typedef long long signed intermediate;
};

template <typename Value, typename Bounds> struct IntermediateTypeSelector {
  static constexpr bool is_floating_point =
      std::is_floating_point_v<Value> || std::is_floating_point_v<Bounds>;

  typedef typename IntermediateTypeSelectorBase<is_floating_point>::intermediate
      type;

  static_assert(std::numeric_limits<type>::lowest() <=
                std::numeric_limits<Value>::lowest() &&
                std::numeric_limits<type>::lowest() <=
                std::numeric_limits<Bounds>::lowest() &&
                std::numeric_limits<type>::max() >=
                std::numeric_limits<Value>::max() &&
                std::numeric_limits<type>::max() >=
                std::numeric_limits<Bounds>::max());
};

struct WithinTests {

  template <typename Value, typename Bounds>
  static TestCase *createWithin(Value value, Bounds min, Bounds max) {
    typedef typename IntermediateTypeSelector<Value, Bounds>::type intermediate;
    bool expected = intermediate(value) >= intermediate(min) &&
                    intermediate(value) <= intermediate(max);
    return Functions::create("is_within", org::simple::core::is_within,
                             expected, value, min, max);
  }

  template <typename Value, typename Bounds>
  static TestCase *createWithinExcl(Value value, Bounds min, Bounds max) {
    typedef typename IntermediateTypeSelector<Value, Bounds>::type intermediate;
    bool expected = intermediate(value) > intermediate(min) &&
                    intermediate(value) < intermediate(max);
    return Functions::create("is_within_excl",
                             org::simple::core::is_within_excl, expected, value,
                             min, max);
  }
};

std::vector<org::simple::test::AbstractValueTestCase *> *generateTestCases() {
  auto testCases =
      new std::vector<org::simple::test::AbstractValueTestCase *>();

  unsigned short U_LO = std::numeric_limits<unsigned short>::lowest();
  unsigned short U_HI = std::numeric_limits<unsigned short>::max();
  unsigned short U_MIN = 2;
  unsigned short U_MAX = 10;

  signed short I_LO = std::numeric_limits<signed short>::lowest();
  signed short I_HI = std::numeric_limits<signed short>::max();
  signed short I_MIN1 = -10;
  signed short I_MAX1 = -5;
  signed short I_MIN2 = -5;
  signed short I_MAX2 = 5;
  signed short I_MIN3 = 5;
  signed short I_MAX3 = 10;

  float F_LO = std::numeric_limits<signed short>::lowest();
  float F_HI = std::numeric_limits<signed short>::max();
  float F_MIN1 = -10;
  float F_MAX1 = -5;
  float F_MIN2 = -5;
  float F_MAX2 = 5;
  float F_MIN3 = 5;
  float F_MAX3 = 10;

  std::vector<unsigned short> unsigned_limits = generateTestValues(U_LO, U_HI);
  std::vector<unsigned short> unsigned_arbitrary =
      generateTestValues<unsigned short>(U_MIN, U_MAX);

  for (unsigned short t : unsigned_limits) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));
  }

  for (unsigned short t : unsigned_arbitrary) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));
  }

  std::vector<signed short> signed_limits = generateTestValues(I_LO, I_HI);
  std::vector<signed short> signed_arbitrary_negative =
      generateTestValues<signed short>(I_MIN1, I_MAX1);
  std::vector<signed short> signed_arbitrary_around =
      generateTestValues<signed short>(I_MIN2, I_MAX2);
  std::vector<signed short> signed_arbitrary_positive =
      generateTestValues<signed short>(I_MIN3, I_MAX3);

  for (signed short t : signed_limits) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  for (signed short t : signed_arbitrary_negative) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  for (signed short t : signed_arbitrary_around) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  for (signed short t : signed_arbitrary_positive) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  std::vector<float> float_limits = generateTestValues(F_LO, F_HI);
  std::vector<float> float_arbitrary_negative =
      generateTestValues<float>(I_MIN1, I_MAX1);
  std::vector<float> float_arbitrary_around =
      generateTestValues<float>(I_MIN2, I_MAX2);
  std::vector<float> float_arbitrary_positive =
      generateTestValues<float>(I_MIN3, I_MAX3);


  for (float t : float_limits) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  for (float t : float_arbitrary_negative) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  for (float t : float_arbitrary_around) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  for (float t : float_arbitrary_positive) {
    testCases->push_back(WithinTests::createWithin(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithin(t, U_MIN, U_MAX));
    testCases->push_back(WithinTests::createWithinExcl(t, U_LO, U_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, U_MIN, U_MAX));

    testCases->push_back(WithinTests::createWithin(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, I_LO, I_HI));
    testCases->push_back(WithinTests::createWithin(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN1, I_MAX1));
    testCases->push_back(WithinTests::createWithin(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN2, I_MAX2));
    testCases->push_back(WithinTests::createWithin(t, I_MIN3, I_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, I_MIN3, I_MAX3));

    testCases->push_back(WithinTests::createWithin(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithinExcl(t, F_LO, F_HI));
    testCases->push_back(WithinTests::createWithin(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN1, F_MAX1));
    testCases->push_back(WithinTests::createWithin(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN2, F_MAX2));
    testCases->push_back(WithinTests::createWithin(t, F_MIN3, F_MAX3));
    testCases->push_back(WithinTests::createWithinExcl(t, F_MIN3, F_MAX3));
  }

  return testCases;
}

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

BOOST_AUTO_TEST_SUITE(org_simple_core_bounds)
TestGenerator TEST_GENERATOR;

BOOST_DATA_TEST_CASE(sample, TEST_GENERATOR.getTestCases()) { sample->test(); }

BOOST_AUTO_TEST_SUITE_END()
