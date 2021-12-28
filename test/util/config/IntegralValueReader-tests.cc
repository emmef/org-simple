//
// Created by michel on 28-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/util/config/ConfigReaders.h>
#include <org-simple/util/text/StringStream.h>

namespace {
template <typename Value>
using IntegralNumberReader =
    org::simple::util::config::IntegralNumberReader<char, Value>;
using Stream = org::simple::util::text::StringInputStream<char>;
using Result = org::simple::util::config::ReaderResult;

template <typename Value>
static const char *printNumber(Value v, char *buffer, size_t cap, int leadingZeroes) {
  if constexpr (std::is_same_v<bool, Value>) {
    buffer[0] = v != 0 ? "1" : "0";
    buffer[1] = 0;
    return buffer + 1;
  } else {
    char format[8];
    int pos = 0;
    format[pos++] = '%';
    if (leadingZeroes) {
      leadingZeroes += 3;
      format[pos++] = '0';
      if (leadingZeroes > 10) {
        format[pos++] = '0' + (leadingZeroes / 10);
      }
      format[pos++] = '0' + (leadingZeroes % 10);
    }
    if constexpr (sizeof(Value) == sizeof(long)) {
      format[pos++] = 'l';
    }
    if constexpr (sizeof(Value) > sizeof(long)) {
      format[pos++] = 'l';
    }
    if constexpr (std::is_signed<Value>()) {
      format[pos++] = 'i';
    } else {
      format[pos++] = 'u';
    }
    format[pos] = 0;
    int l = snprintf(buffer, cap, format, v);
    return buffer + (l > 0 ? l : 0);
  }
}

template <typename Value>
static const char *printNumber(Value v, int leadingZeroes, const char *before = "",
                               const char *after = "") {
  static constexpr int LEN = 50;
  static char numBuffer[50 + 1];
  int pos = 0;
  const char *p = before;
  while (pos < LEN && *p) {
    numBuffer[pos++] = *p++;
  }
  pos = printNumber(v, numBuffer + pos, LEN - pos, leadingZeroes) - numBuffer;
  p = after;
  while (pos < LEN && *p) {
    numBuffer[pos++] = *p++;
  }
  numBuffer[pos] = 0;
  return numBuffer;
}

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
    return org::simple::util::config::ReaderResult::Invalid;
  }
  if (v <min) {
    return org::simple::util::config::ReaderResult::Invalid;
  }
  return org::simple::util::config::ReaderResult::Ok;
}

template <typename Value>
static Result
calculateExpectedResult(typename ScenarioNumberType<Value>::type) {
  return org::simple::util::config::ReaderResult::Ok;
}

class SeparatorPredicate : public org::simple::util::Predicate<char> {
public:
  bool test(const char &c) const override { return c == ','; }
};

const SeparatorPredicate separatorPredicate;

template <typename Value> class Reader : public IntegralNumberReader<Value> {
protected:
  virtual void writeValue(const Value &value) {
    actualValue = value;
    actuallySet = true;
  }

public:
  Value actualValue = 0;
  bool actuallySet = false;

  Reader() : IntegralNumberReader<Value>(&separatorPredicate) {}
};

template <typename Value> struct Scenario {
  typedef typename ScenarioNumberType<Value>::type inputValueType;
  std::string input;
  Result expectedResult;
  Value expectedValue;

  // A scenario with a number without prefix or anything
  Scenario(inputValueType value)
      : input(printNumber(value, 0)),
        expectedResult(calculateExpectedResult<Value>(value)),
        expectedValue(value) {}
  Scenario(inputValueType value, bool )
      : input(printNumber(value, std::numeric_limits<Value>::digits10)),
        expectedResult(calculateExpectedResult<Value>(value)),
        expectedValue(value) {}
  // A scenario with a number with prefix and such that should not affect
  // outcome
  Scenario(inputValueType value, const char *before, const char *after)
      : input(printNumber(value, 0, before, after)),
        expectedResult(calculateExpectedResult<Value>(value)),
        expectedValue(static_cast<Value>(
            expectedResult == org::simple::util::config::ReaderResult::Ok
                ? value
                : 0)) {}
  Scenario(inputValueType value, Result expected, const char *before,
           const char *after)
      : input(printNumber(value, 0, before, after)), expectedResult(expected),
        expectedValue(static_cast<Value>(
            expectedResult == org::simple::util::config::ReaderResult::Ok
                ? value
                : 0)) {}

  static void test(const Scenario &scenario) {
    Reader<Value> reader;
    reader.actualValue = 0;
    Stream stream(scenario.input);
    Result actualResult = reader.read(stream, "key");

    BOOST_CHECK_EQUAL(scenario.expectedResult, actualResult);
    bool expectedToBeSet =
        scenario.expectedResult == org::simple::util::config::ReaderResult::Ok;
    BOOST_CHECK_EQUAL(expectedToBeSet, reader.actuallySet);
    if (expectedToBeSet) {
      BOOST_CHECK_EQUAL(scenario.expectedValue, reader.actualValue);
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
static ostream &operator<<(ostream &out,
                           org::simple::util::config::ReaderResult result) {
  switch (result) {
  case org::simple::util::config::ReaderResult::Ok:
    out << "Ok";
    break;
  case org::simple::util::config::ReaderResult::NotFound:
    out << "NotFound";
    break;
  case org::simple::util::config::ReaderResult::Invalid:
    out << "Invalid";
    break;
  }
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
    }
    else {
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
    results.push_back({testValue, "", ","});
    results.push_back({testValue, " ", ","});
    results.push_back({testValue, true});
  }
  return results;
}

BOOST_AUTO_TEST_SUITE(test_org_simple_util_config_IntegralValueReader)

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

//BOOST_AUTO_TEST_CASE(testMultipleReads) {
//  Reader<int> reader;
//  Stream stream("13, -54, 154");
//
//  Result actualResult = reader.read(stream, "key");
//
//  BOOST_CHECK_EQUAL(scenario.expectedResult, actualResult);
//  bool expectedToBeSet =
//      scenario.expectedResult == org::simple::util::config::ReaderResult::Ok;
//  BOOST_CHECK_EQUAL(expectedToBeSet, reader.actuallySet);
//  if (expectedToBeSet) {
//    BOOST_CHECK_EQUAL(scenario.expectedValue, reader.actualValue);
//  }
//}
//
BOOST_AUTO_TEST_SUITE_END()
