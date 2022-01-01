//
// Created by michel on 28-12-21.
//

#include "org-simple/util/text/StreamPredicate.h"
#include <org-simple/util/text/StringStream.h>
#include <org-simple/util/text/NumberParser.h>
#include "test-helper.h"
#include "boost-unit-tests.h"

namespace {
template <typename Value>
using Stream = org::simple::util::text::StringInputStream<char>;
using Result = org::simple::util::text::NumberParser::Result;
using NumberParser = org::simple::util::text::NumberParser;

template <typename Value, bool isSigned = std::is_signed<Value>() ||
                                          sizeof(Value) < sizeof(long long)>
struct ScenarioNumberType;

template <typename Value> struct ScenarioNumberType<Value, false> {
  typedef unsigned long long type;
  static constexpr bool sign = std::is_signed<Value>();
  static constexpr bool largest = sizeof(Value) == sizeof(long long);
};

template <typename Value> struct ScenarioNumberType<Value, true> {
  typedef signed long long type;
  static constexpr bool sign = std::is_signed<Value>();
  static constexpr bool largest = sizeof(Value) == sizeof(long long);
};

template <typename Value>
requires(sizeof(Value) < sizeof(long long)) //
    static Result
    calculateExpectedResult(typename ScenarioNumberType<Value>::type v) {
  static typename ScenarioNumberType<Value>::type max =
      std::is_same<bool, Value>() ? 1 : std::numeric_limits<Value>::max();
  static typename ScenarioNumberType<Value>::type min =
      std::is_same<bool, Value>() ? 1 : std::numeric_limits<Value>::min();
  if (v > max) {
    return Result::OutOfRange;
  }
  if (v < min) {
    return std::is_signed<Value>()
               ? Result::OutOfRange
               : Result::UnexpectedCharacter;
  }
  return Result::Ok;
}

template <typename Value>
static Result
calculateExpectedResult(typename ScenarioNumberType<Value>::type) {
  return org::simple::util::text::NumberParser::Result::Ok;
}

class SeparatorPredicate : public org::simple::util::Predicate<char> {
public:
  bool test(const char &c) const override { return c == ','; }
};

template <typename Value> struct Scenario {
  typedef typename ScenarioNumberType<Value>::type inputValueType;
  std::string input;
  Result expectedResult;
  Value expectedValue;

  // A scenario with a number without prefix or anything
  Scenario(inputValueType value)
      : input(org::simple::test::printNumber(value, 0)),
        expectedResult(calculateExpectedResult<Value>(value)),
        expectedValue(value) {}
  Scenario(inputValueType value, bool)
      : input(org::simple::test::printNumber(value, std::numeric_limits<Value>::digits10)),
        expectedResult(calculateExpectedResult<Value>(value)),
        expectedValue(value) {}
  // A scenario with a number with prefix and such that should not affect
  // outcome
  Scenario(inputValueType value, const char *before, const char *after)
      : input(org::simple::test::printNumber(value, 0, before, after)),
        expectedResult(calculateExpectedResult<Value>(value)),
        expectedValue(static_cast<Value>(
            expectedResult == Result::Ok
                ? value
                : 0)) {}
  Scenario(inputValueType value, Result expected, const char *before,
           const char *after)
      : input(printNumber(value, 0, before, after)), expectedResult(expected),
        expectedValue(static_cast<Value>(
            expectedResult == Result::Ok
                ? value
                : 0)) {}

  static void test(const Scenario &scenario) {
    Stream<char> stream(scenario.input);
    Value actualValue;
    Result actualResult = NumberParser::readIntegralValueFromStream<char>(stream, actualValue);

    BOOST_CHECK_EQUAL(scenario.expectedResult, actualResult);
    if (actualResult == Result::Ok) {
      BOOST_CHECK_EQUAL(scenario.expectedValue, actualValue);
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
  out << "IntegralValueReader<";
  out << (std::is_signed<Value>() ? "int_" : "uint_");
  out << (sizeof(Value) * 8);
  out << "_t>(\"" << input << "\") -> " << expectedResult;

  if (expectedResult == Result::Ok) {
    if constexpr (sizeof(Value) == 1) {
      out << "(value = " << (int(expectedValue) & 0xff) << ")";
    } else {
      out << "(value = " << expectedValue << ")";
    }
  }
}

template <typename Value>
static std::vector<Scenario<Value>> &generateUnsignedSamples() {
  static std::vector<Scenario<Value>> results;
  typedef typename Scenario<Value>::inputValueType testValueType;
  static constexpr testValueType max =
      static_cast<testValueType>(std::numeric_limits<Value>::max());
  static constexpr testValueType min =
      static_cast<testValueType>(std::numeric_limits<Value>::min());
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
    if constexpr (!ScenarioNumberType<Value>::largest) {
      testValues.push_back(max + 1);
      testValues.push_back(max * 2);
      testValues.push_back(min - 1);
    }
    if constexpr (std::is_signed<Value>()) {
      testValues.push_back(std::numeric_limits<Value>::min());
    }
    size_t count = testValues.size();
    for (size_t i = 1; i < count; i++) {
      testValues.push_back(-testValues.at(i));
    }
  }

  for (auto testValue : testValues) {
    results.push_back({testValue});
    results.push_back({testValue, " ", ""});
    results.push_back({testValue, "", " "});
    results.push_back({testValue, " ", " "});
    //    results.push_back({testValue, "", ","});
    //    results.push_back({testValue, " ", ","});
    results.push_back({testValue, true});
  }
  return results;
}

void testThreeKnownNumbers(const char *string) {
  Stream<char> stringStream(string);
  org::simple::util::text::EchoStream<char, Stream<char>> echo(&stringStream);
  struct NoCommaPredicate : org::simple::util::Predicate<char> {
    bool test(const char &c) const { return c != ','; }
  } predicate;
  org::simple::util::text::PredicateStream<char, NoCommaPredicate, false>
      stream(&echo, predicate);
  Result actualResult;
  static constexpr Result Ok = Result::Ok;
  int actualValue;
  char x;

  actualResult = NumberParser::readIntegralValueFromStream<char>(stream, actualValue);
  BOOST_CHECK_EQUAL(Ok, actualResult);
  BOOST_CHECK_EQUAL(13, actualValue);
  x = echo.peek();
  do {
    if (!predicate.test(x)) {
      // separator!
      break;
    } else if (x == ' ') {
      echo.get(x);
    } else {
      echo.repeat();
      break;
    }
  } while (true);
  actualResult = NumberParser::readIntegralValueFromStream<char>(stream, actualValue);
  BOOST_CHECK_EQUAL(Ok, actualResult);
  BOOST_CHECK_EQUAL(-54, actualValue);
  x = echo.peek();
  do {
    if (!predicate.test(x)) {
      // separator!
      break;
    } else if (x == ' ') {
      echo.get(x);
    } else {
      echo.repeat();
      break;
    }
  } while (true);
  actualResult = NumberParser::readIntegralValueFromStream<char>(stream, actualValue);
  BOOST_CHECK_EQUAL(Ok, actualResult);
  BOOST_CHECK_EQUAL(154, actualValue);
}

BOOST_AUTO_TEST_SUITE(test_org_simple_util_config_IntegralNumberParserTest)

BOOST_DATA_TEST_CASE(testUnsignedCharScenarios,
                     generateUnsignedSamples<unsigned char>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testSignedCharScenarios,
                     generateUnsignedSamples<signed char>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testUnsignedShortScenarios,
                     generateUnsignedSamples<unsigned short>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testSignedShortScenarios,
                     generateUnsignedSamples<signed short>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testUnsignedScenarios,
                     generateUnsignedSamples<unsigned>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testSignedScenarios, generateUnsignedSamples<signed>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testUnsignedLongScenarios,
                     generateUnsignedSamples<unsigned long>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testSignedLongScenarios,
                     generateUnsignedSamples<signed long>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testUnsignedLongLongScenarios,
                     generateUnsignedSamples<unsigned long long>()) {
  testScenario(sample);
}

BOOST_DATA_TEST_CASE(testSignedLongLongScenarios,
                     generateUnsignedSamples<signed long long>()) {
  testScenario(sample);
}

BOOST_AUTO_TEST_CASE(testMultipleReadsSpaceSeparated) {
  testThreeKnownNumbers("13 -54 154");
}

BOOST_AUTO_TEST_CASE(testMultipleReadsCommaSeparated) {
  testThreeKnownNumbers("13,-54,154");
}

BOOST_AUTO_TEST_CASE(testMultipleReadsSpaceCommaSeparated) {
  testThreeKnownNumbers("13 ,-54 ,154");
}

BOOST_AUTO_TEST_CASE(testMultipleReadsCommaSpaceSeparated) {
  testThreeKnownNumbers("13, -54, 154");
}

BOOST_AUTO_TEST_CASE(testMultipleReadsSpaceCommaSpaceSeparated) {
  testThreeKnownNumbers(" 13 , -54 , 154");
}

BOOST_AUTO_TEST_SUITE_END()
