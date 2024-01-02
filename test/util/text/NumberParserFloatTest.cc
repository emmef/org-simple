//
// Created by michel on 28-12-21.
//

#include "org-simple/util/text/StreamPredicate.h"
#include <org-simple/util/text/StringStream.h>
#include <org-simple/util/text/NumberParser.h>
#include "test-helper.h"
#include "boost-unit-tests.h"
#include <boost/math/special_functions/relative_difference.hpp>
#include <cstdio>

namespace {
template <typename Value>
using Stream = org::simple::util::text::StringInputStream<char>;
using Result = org::simple::util::text::NumberParser::Result;
using NumberParser = org::simple::util::text::NumberParser;

template <typename Value = double> struct Scenario {
  static_assert(std::is_floating_point<Value>());
  std::string input;
  Result expectedResult;
  Value expectedValue;

  // A scenario with a number without prefix or anything
  Scenario(const char * string, Result result, Value value) : input(string), expectedResult(result), expectedValue(value) {

  }

  Scenario(long double number, Result result, Value value) : Scenario("", result, value) {
    static constexpr size_t FMT_LEN = 10;
    static constexpr int MANTISSA_DIGITS = std::numeric_limits<Value>::max_digits10;
    static constexpr unsigned EXP_10_MAX = static_cast<unsigned>(std::numeric_limits<double>::max_exponent10);
    static constexpr int EXP_10_BITS = sizeof(int)*8 - std::countl_zero(EXP_10_MAX);
    static constexpr size_t EXP_DIGITS = sizeof(int)*8 - std::countl_zero(EXP_10_MAX);
    static constexpr size_t SIGNS_EXP_DECIMAL = 4;
    static constexpr size_t LEN = MANTISSA_DIGITS + EXP_DIGITS + SIGNS_EXP_DECIMAL + 10;
    char format[FMT_LEN+1];
    char buffer[LEN + 1];

    std::snprintf(format, FMT_LEN, "%%.%ulle", MANTISSA_DIGITS);
    std::snprintf(buffer, LEN, format, number);
    input = buffer;
  }

  static void test(const Scenario &scenario) {
    Stream<char> stream(scenario.input);
    Value actualValue;
    Result actualResult = NumberParser::readRealValueFromStream<char>(stream, actualValue);

    BOOST_CHECK_EQUAL(scenario.expectedResult, actualResult);
    if (actualResult == Result::Ok) {
      if (boost::math::relative_difference(scenario.expectedValue, actualValue) > std::numeric_limits<Value>::epsilon()) {
        BOOST_CHECK_EQUAL(scenario.expectedValue, actualValue);
      }
    }
  }

  void writeTo(std::ostream &out) const;
};

template <typename Value>
static void testScenario(const Scenario<Value> &scenario) {
  return Scenario<Value>::test(scenario);
}

} // namespace

namespace std {
static ostream &operator<<(ostream &out, Result result) {
  out << NumberParser ::resultToString(result);
  return out;
}

template <typename Value>
static ostream &operator<<(ostream &out, const Scenario<Value> &scenario) {
  scenario.writeTo(out);
  return out;
}

} // namespace std

template <typename Value>
void Scenario<Value>::writeTo(std::ostream &out) const {
  out << "IntegralValueReader<float_" << (sizeof(Value) * 8);
  out << "_t>(\""<< input << "\") -> " << expectedResult;

  if (expectedResult == Result::Ok) {
      out << "(value = " << expectedValue << ")";
  }
}

template <typename Value>
static std::vector<Scenario<Value>> &generateFLoatTestSamples() {
  static std::vector<Scenario<Value>> results;
  typedef long double testValueType;
  static constexpr testValueType max =
      static_cast<testValueType>(std::numeric_limits<Value>::max());
  static constexpr testValueType min =
      static_cast<testValueType>(std::numeric_limits<Value>::lowest());
  std::vector<testValueType> testValues;

  if (std::is_same<bool, Value>()) {
    testValues.push_back(0);
    testValues.push_back(1);
    testValues.push_back(2);
  } else {
    testValues.push_back(0);
    testValues.push_back(1);
    testValues.push_back(max / 2);
    testValues.push_back(max - 1);
    testValues.push_back(max);
    testValues.push_back(max + 1);
    testValues.push_back(max * 2);
    testValues.push_back(min - 1);
    if constexpr (std::is_signed<Value>()) {
      testValues.push_back(std::numeric_limits<Value>::min());
    }
    size_t count = testValues.size();
    for (size_t i = 1; i < count; i++) {
      testValues.push_back(-testValues.at(i));
    }
  }
//
  for (auto testValue : testValues) {
    Result result = testValue >= min && testValue <= max ? Result::Ok : Result::OutOfRange;
    results.push_back({testValue, result, static_cast<Value>(testValue)});
  }
  return results;
}

template <typename Value>
static std::vector<Scenario<Value>> &generateValidTestcases() {
  static std::vector<Scenario<Value>> results;

  results.push_back({"0", Result::Ok, 0});
  results.push_back({".0", Result::Ok, 0});
  results.push_back({"0.", Result::Ok, 0});
  results.push_back({"0.0", Result::Ok, 0});
  results.push_back({"+0", Result::Ok, 0});
  results.push_back({"+.0", Result::Ok, 0});
  results.push_back({"+0.", Result::Ok, 0});
  results.push_back({"+0.0", Result::Ok, 0});
  results.push_back({"-0", Result::Ok, 0});
  results.push_back({"-.0", Result::Ok, 0});
  results.push_back({"-0.", Result::Ok, 0});
  results.push_back({"-0.0", Result::Ok, 0});

  results.push_back({"1", Result::Ok, 1});
  results.push_back({".1", Result::Ok, 0.1});
  results.push_back({"1.", Result::Ok, 1});
  results.push_back({"1.0", Result::Ok, 1});
  results.push_back({"0.1", Result::Ok, 0.1});

  results.push_back({"+1", Result::Ok, 1});
  results.push_back({"+.1", Result::Ok, 0.1});
  results.push_back({"+1.", Result::Ok, 1});
  results.push_back({"+1.0", Result::Ok, 1});
  results.push_back({"+0.1", Result::Ok, 0.1});

  results.push_back({"-1", Result::Ok, -1});
  results.push_back({"-.1", Result::Ok, -0.1});
  results.push_back({"-1.", Result::Ok, -1});
  results.push_back({"-1.0", Result::Ok, -1});
  results.push_back({"-0.1", Result::Ok, -0.1});

  return results;
}


BOOST_AUTO_TEST_SUITE(test_org_simple_util_config_NumberParserFloatTest)

BOOST_DATA_TEST_CASE(testUnsignedCharScenarios,
                     generateFLoatTestSamples<double>()) {
  testScenario(sample);
}

//BOOST_DATA_TEST_CASE(testSignedCharScenarios,
//                     generateUnsignedSamples<double>()) {
//  testScenario(sample);
//}
//
//BOOST_DATA_TEST_CASE(testUnsignedShortScenarios,
//                     generateUnsignedSamples<long double>()) {
//  testScenario(sample);
//}


BOOST_DATA_TEST_CASE(testSimpleValisUsecases, generateValidTestcases<double>()) {
  testScenario(sample);
}
BOOST_AUTO_TEST_CASE(testValidDouble) {

}

BOOST_AUTO_TEST_SUITE_END()
