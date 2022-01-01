//
// Created by michel on 28-12-21.
//

#include "boost-unit-tests.h"
#include "test-helper.h"
#include <cstdint>
#include <org-simple/util/config/IntegralNumberReader.h>
#include <org-simple/util/text/StreamPredicate.h>
#include <org-simple/util/text/StringStream.h>

namespace {

using Stream = org::simple::util::text::StringInputStream<char>;
using TokenizedStream = org::simple::util::text::TokenizedInputStream<char>;
using ReaderResult = org::simple::util::config::ReaderResult;
using Basics = org::simple::util::text::NumberParser;
using Result = org::simple::util::text::NumberParser::Result;
using Value = std::uint16_t;

struct Reader
    : public org::simple::util::config::IntegralNumberReader<char, Value> {
public:
  Value actualValue;
  void writeValue(const Value &value) { actualValue = value; }
};

struct Scenario {
  typedef std::uint32_t inputValueType;
  std::string input;
  Result expectedParserResult;
  ReaderResult expectedReaderResult;
  Value expectedValue;

  static Result calculateExpectedResult(inputValueType v) {
    static inputValueType max = std::numeric_limits<Value>::max();
    static inputValueType min = std::numeric_limits<Value>::min();
    if (v > max || v < min) {
      return Result::OutOfRange;
    }
    return Result::Ok;
  }

  // A scenario with a number without prefix or anything
  Scenario(const char *value, Result parseResult,
           Value expected)
      : input(value), expectedParserResult(parseResult),
        expectedReaderResult(org::simple::util::config::toReaderResult(expectedParserResult)), expectedValue(expected) {}
  Scenario(inputValueType value, bool leadingZeroes = false)
      : Scenario(org::simple::test::printNumber(value, leadingZeroes ? std::numeric_limits<Value>::digits10 : 0), calculateExpectedResult(value), static_cast<Value>(value)) {}
  Scenario(inputValueType value, const char *before, const char *after)
      : Scenario(org::simple::test::printNumber(value, 0, before, after), calculateExpectedResult(value), static_cast<Value>(value)) {}
  Scenario(inputValueType value, Result expected, const char *before,const char *after)
      : Scenario(org::simple::test::printNumber(value, 0, before, after), expected, static_cast<Value>(value)) {}

  static void test(const Scenario &scenario) {
    Stream streamSource(scenario.input);
    class TS : public TokenizedStream {
      Stream &input;
    public:
      TS(Stream &s) : input(s) {}

      bool get(char &c) { return input.get(c); }
      bool isExhausted() const override { return false; }
      /**
   * Resets the exhausted-state, which is useful if this token stream is based
   * upon yet another token stream.
       */
      void resetExhausted() override {}
      void rewind() { input.rewind(); }
    } stream(streamSource);
    Reader reader;
    Value actualValue;
    Result actualParseResult =
        Basics::readIntegralValueFromStream<char>(stream, actualValue);

    BOOST_CHECK_EQUAL(scenario.expectedParserResult, actualParseResult);
    if (actualParseResult == Result::Ok) {
      BOOST_CHECK_EQUAL(scenario.expectedValue, actualValue);
    }

    stream.rewind();
    ReaderResult actualReaderResult = reader.read(stream, "key");
    BOOST_CHECK_EQUAL(scenario.expectedReaderResult, actualReaderResult);
    BOOST_CHECK_EQUAL(
        actualReaderResult,
        org::simple::util::config::toReaderResult(actualParseResult));
    if (actualReaderResult == ReaderResult::Ok) {
      BOOST_CHECK_EQUAL(scenario.expectedValue, reader.actualValue);
    }
  }

  void writeTo(std::ostream &out) const;
};



} // namespace

namespace std {
static ostream &operator<<(ostream &out, ReaderResult result) {
  out << org::simple::util::config::readerResultToString(result);
  return out;
}
static ostream &operator<<(ostream &out, Result result) {
  out << org::simple::util::text::NumberParser::resultToString(result);
  return out;
}
static std::ostream &operator<<(std::ostream &out, const Scenario &scenario) {
  scenario.writeTo(out);
  return out;
}

} // namespace std

void Scenario::writeTo(std::ostream &out) const {
  out << "IntegralValueReader<uint16_t>(\"" << input << "\") -> (ReaderResult=" << expectedReaderResult
      << "; ParserResult=" << expectedParserResult;

  if (expectedParserResult == Result::Ok) {
    out << "; value = " << uint16_t(expectedValue);
  }
  out << ")";
}

static std::vector<Scenario> &generateSamples() {
  static std::vector<Scenario> results;
  typedef typename Scenario::inputValueType testValueType;
  static constexpr testValueType max =
      static_cast<testValueType>(std::numeric_limits<Value>::max());
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

BOOST_AUTO_TEST_SUITE(test_org_simple_util_config_IntegralNumberReaderTest)

BOOST_DATA_TEST_CASE(testUnsignedCharScenarios_IntegralValueReader,
                     generateSamples()) {
  Scenario::test(sample);
}

BOOST_AUTO_TEST_SUITE_END()
