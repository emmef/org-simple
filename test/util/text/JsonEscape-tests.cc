//
// Created by michel on 27-01-22.
//
#include "boost-unit-tests.h"
#include <charconv>
#include <org-simple/util/text/Json.h>
#include <string>

using EscapeState = org::simple::util::text::JsonEscapeState;

static constexpr EscapeState ESCAPE_CHAR = {1, 0, 0};
static constexpr EscapeState ESCAPE_NONE = {0, 0, 0};

static constexpr bool noEscapeState(const EscapeState &state) {
  return state.type == 0 && state.count == 0 && state.value == 0;
}

static constexpr bool singleEscapeState(const EscapeState &state) {
  return state.type == 1 && state.count == 0 && state.value == 0;
}

namespace org::simple::util::text {

std::ostream &operator<<(std::ostream &out, const EscapeState &state) {
  static constexpr const char digits[] = "0123456789abcdef";
  static constexpr size_t LEN = 8;
  char hexNumber[LEN + 1];
  out << "{";
  switch (state.type) {
  case 0:
    break;
  case 1:
    out << '1';
    break;
  case 2:
    out << "2";
    break;
  }
  if (state.count) {
    out << ", " << state.count;
  }
  if (state.value != 0 || state.type == 2) {
    uint32_t value = state.value;
    for (int i = 0, j = 7; i < 8; i++, j--) {
      hexNumber[j] = digits[value % 16];
      value /= 16;
    }
    hexNumber[8] = '\0';
    out << ", 0x" << hexNumber;
  }
  out << "}";
  return out;
}

} // namespace org::simple::util::text

namespace {

class String;

struct Scenario {
  struct Step {
    // input
    const char input;
    // result from append
    const bool appendResult;
    // expected state after;
    const EscapeState escapeStateAfterAppend;
  };
  std::vector<Step> steps;
  std::string expectedResult;

  void execute(String &string) const;

  void execute() const;
};

std::ostream &operator<<(std::ostream &out, const Scenario::Step &step) {
  static constexpr const char digits[] = "0123456789abcdef";
  out << "{'";
  if (step.input < ' ' || step.input >= 126) {
    out << "\\x" << digits[step.input / 16] << digits[step.input % 16];
  } else {
    out << step.input;
  }
  out << "', " << (step.appendResult ? "true" : "false");
  out << ", " << step.escapeStateAfterAppend << "}";
  return out;
}

std::ostream &operator<<(std::ostream &out, const Scenario &scenario) {
  static constexpr const char digits[] = "0123456789abcdef";
  out << "Scenario{[";
  bool first = true;
  for (const Scenario::Step &step : scenario.steps) {
    if (first) {
      first = false;
      out << std::endl << '\t' << '\t';
    } else {
      out << "," << std::endl << '\t' << '\t';
    }
    out << step;
  }
  out << "] -> \"";
  for (const char c : scenario.expectedResult) {
    if (c < ' ' || c >= 126) {
      out << "\\x" << digits[c / 16] << digits[c % 16];
    } else {
      out << c;
    }
  }
  out << '"' << std::endl << "\t}";
  return out;
}

class String {
  std::string value;
  EscapeState state = {};
  size_t maxLen = 20;

public:
  const EscapeState &getState() const { return state; }
  const char *getActual() const { return value.c_str(); }
  void start() { state = {}; }
  bool append(const char &c) {
    return org::simple::util::text::addJsonStringCharacter(
        c, state, [this](const char &c) {
          if (value.length() < maxLen) {
            value += c;
            return true;
          }
          return false;
        });
  }
  bool append(const char *string) {
    bool appended = true;
    const char *p = string;
    while (appended && *p) {
      appended = append(*p++);
    }
    return appended;
  }

  void executeStep(const Scenario::Step &expected) {
    bool actualAppendResult = append(expected.input);
    BOOST_CHECK_EQUAL(expected.appendResult, actualAppendResult);
    BOOST_CHECK_EQUAL(expected.escapeStateAfterAppend, state);
  }
};

void Scenario::execute(String &string) const {
  for (const Step &step : steps) {
    string.executeStep(step);
  }

  BOOST_CHECK_EQUAL(expectedResult.c_str(), string.getActual());
}

void Scenario::execute() const {
  String string;

  execute(string);
}

std::string toUtf8(char32_t codePoint) {
  char utf8[5];
  char *end =
      org::simple::util::text::Utf8Encoding ::unsafeEncode(codePoint, utf8);
  *end = 0;

  return utf8;
}

std::vector<Scenario> simpleScenarios() {
  std::vector<Scenario> results;

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hello"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'\\', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\\lo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'/', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel/lo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'"', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\"lo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'b', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\blo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'f', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\flo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'n', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\nlo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'r', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\rlo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'t', true, {}},
                      {'l', true, {}},
                      {'o', true, {}}},
                     "Hel\tlo"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'l', true, {}},
                      {'o', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'t', true, {}}},
                     "Hello\t"});

  results.push_back({{{'H', true, {}},
                      {'e', true, {}},
                      {'l', true, {}},
                      {'l', true, {}},
                      {'o', true, {}},
                      {'\\', true, ESCAPE_CHAR}},
                     "Hello"});
  return results;
}

Scenario generateForCodePoint(char32_t cp) {
  static constexpr const char digit[] = "0123456789abcdef";
  const char *prefix = "pre";
  const char *suffix = "suf";
  char32_t u = cp - 0x10000;
  char32_t leading = 0xd800 | (u >> 10);
  char32_t trailing = 0xdc00 | (u & 0x03ff);

  std::string expected;
  std::vector<Scenario::Step> steps;

  for (const char *p = prefix; *p != 0; p++) {
    expected += *p;
    steps.push_back({*p, true, {}});
  }

  char32_t value = 0;
  steps.push_back({'\\', true, ESCAPE_CHAR});
  steps.push_back({'u', true, {2, 0, 0}});
  uint16_t c = 1;
  for (int j = 3; j >= 0; j--, c++) {
    char32_t ld = (leading >> (j * 4)) & 0x000f;
    char32_t lv = (leading >> ((3 - j) * 4)) & 0x000f;
    value <<= 4;
    value |= ld;
    if (j != 0) {
      steps.push_back({digit[ld], true, {2, c, value}});
    } else {
      value &= 0x03ff;
      steps.push_back({digit[ld], true, {2, c, value}});
    }
  }

  steps.push_back({'\\', true, {2, c++, value}});
  steps.push_back({'u', true, {2, c++, value}});
  for (int j = 3; j >= 0; j--, c++) {
    char32_t td = (trailing >> (j * 4)) & 0x000f;
    char32_t tv = (trailing >> ((3 - j) * 4)) & 0x000f;
    value <<= 4;
    value |= td;
    if (j != 0) {
      steps.push_back({digit[td], true, {2, c, value}});
    } else {
      steps.push_back({digit[td], true, {}});
    }
  }

  expected += toUtf8(cp);
  for (const char *p = suffix; *p != 0; p++) {
    expected += *p;
    steps.push_back({*p, true, {}});
  }

  Scenario s = {steps, expected};
  return s;
}

std::vector<Scenario> successfulBasicMultilingualPlaneScenarios() {
  std::vector<Scenario> results;

  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'0', true, {2, 1, 0}},
                      {'0', true, {2, 2, 0}},
                      {'0', true, {2, 3, 0}},
                      {'a', true, {}}},
                     "Day\n"});

  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'0', true, {2, 1, 0}},
                      {'0', true, {2, 2, 0}},
                      {'2', true, {2, 3, 0x0002}},
                      {'a', true, {}}},
                     "Day\x2a"});

  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'0', true, {2, 1, 0}},
                      {'0', true, {2, 2, 0}},
                      {'a', true, {2, 3, 0x00a}},
                      {'3', true, {}}},
                     "Day\xc2\xa3"});

  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'0', true, {2, 1, 0}},
                      {'9', true, {2, 2, 0x0009}},
                      {'3', true, {2, 3, 0x0093}},
                      {'9', true, {}}},
                     "Day\xe0\xa4\xb9"});

  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'d', true, {2, 1, 0x000d}},
                      {'5', true, {2, 2, 0x00d5}},
                      {'5', true, {2, 3, 0x0d55}},
                      {'c', true, {}}},
                     "Day\xed\x95\x9c"});

  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'d', true, {2, 1, 0x000d}},
                      {'7', true, {2, 2, 0x00d7}},
                      {'f', true, {2, 3, 0x0d7f}},
                      {'f', true, {}}},
                     expected});

  expected = "Day";
  expected += toUtf8(0xe000);
  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'e', true, {2, 1, 0x000e}},
                      {'0', true, {2, 2, 0x00e0}},
                      {'0', true, {2, 3, 0x0e00}},
                      {'0', true, {}}},
                     expected});

  expected = "Day";
  expected += toUtf8(0xe001);
  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'e', true, {2, 1, 0x000e}},
                      {'0', true, {2, 2, 0x00e0}},
                      {'0', true, {2, 3, 0x0e00}},
                      {'1', true, {}}},
                     expected});

  expected = "Day";
  expected += toUtf8(0xffff);
  results.push_back({{{'D', true, {}},
                      {'a', true, {}},
                      {'y', true, {}},
                      {'\\', true, ESCAPE_CHAR},
                      {'u', true, {2, 0, 0}},
                      {'f', true, {2, 1, 0x000f}},
                      {'f', true, {2, 2, 0x00ff}},
                      {'f', true, {2, 3, 0x0fff}},
                      {'f', true, {}}},
                     expected});
  return results;
}

std::vector<Scenario> successfulSupplementaryPlaneScenarios() {
  std::vector<Scenario> results;

  results.push_back(generateForCodePoint(0x010000));
  results.push_back(generateForCodePoint(0x010437));
  results.push_back(generateForCodePoint(0x10ffff));

  results.push_back(generateForCodePoint(0x055555));
  results.push_back(generateForCodePoint(0x0aaaaa));
  results.push_back(generateForCodePoint(0x0a55aa));
  results.push_back(generateForCodePoint(0x05aa55));
  results.push_back(generateForCodePoint(0x05a5a5));
  results.push_back(generateForCodePoint(0x0a5a5a));

  results.push_back(generateForCodePoint(0x105555));
  results.push_back(generateForCodePoint(0x10aaaa));
  results.push_back(generateForCodePoint(0x1055aa));
  results.push_back(generateForCodePoint(0x10aa55));
  results.push_back(generateForCodePoint(0x10a5a5));
  results.push_back(generateForCodePoint(0x105a5a));

  results.push_back(generateForCodePoint(0x033333));
  results.push_back(generateForCodePoint(0x066666));
  results.push_back(generateForCodePoint(0x0ccccc));
  results.push_back(generateForCodePoint(0x036c36));
  results.push_back(generateForCodePoint(0x06c36c));
  results.push_back(generateForCodePoint(0x0c36c3));
  results.push_back(generateForCodePoint(0x03c63c));
  results.push_back(generateForCodePoint(0x0c63c6));
  results.push_back(generateForCodePoint(0x063c63));

  results.push_back(generateForCodePoint(0x103333));
  results.push_back(generateForCodePoint(0x106666));
  results.push_back(generateForCodePoint(0x10cccc));
  results.push_back(generateForCodePoint(0x106c36));
  results.push_back(generateForCodePoint(0x10c36c));
  results.push_back(generateForCodePoint(0x1036c3));
  results.push_back(generateForCodePoint(0x10c63c));
  results.push_back(generateForCodePoint(0x1063c6));
  results.push_back(generateForCodePoint(0x103c63));

  return results;
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_AddJsonStringCharacter)

BOOST_DATA_TEST_CASE(testSimpleScenarios, simpleScenarios()) {
  sample.execute();
}

BOOST_DATA_TEST_CASE(testSuccessfulUnicodeScenarios,
                     successfulBasicMultilingualPlaneScenarios()) {
  sample.execute();
}

BOOST_DATA_TEST_CASE(testSuccessFulSupplemntaryPLanes,
                     successfulSupplementaryPlaneScenarios()) {
  sample.execute();
}

BOOST_AUTO_TEST_CASE(testInvalidDigit1) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'g', true, {2, 1, 0x000d}},
                        {'b', true, {2, 2, 0x00db}},
                        {'d', true, {2, 3, 0x0dbd}},
                        {'5', true, {2, 4, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidDigit2) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'d', true, {2, 1, 0x000d}},
                        {'g', true, {2, 2, 0x00db}},
                        {'d', true, {2, 3, 0x0dbd}},
                        {'5', true, {2, 4, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidDigit3) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'d', true, {2, 1, 0x000d}},
                        {'b', true, {2, 2, 0x00db}},
                        {'g', true, {2, 3, 0x0dbd}},
                        {'5', true, {2, 4, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidDigit4) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'d', true, {2, 1, 0x000d}},
                        {'b', true, {2, 2, 0x00db}},
                        {'d', true, {2, 3, 0x0dbd}},
                        {'g', true, {2, 4, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testNoTrailingSurrogateFirst) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'d', true, {2, 1, 0x000d}},
                        {'b', true, {2, 2, 0x00db}},
                        {'d', true, {2, 3, 0x0dbd}},
                        {'5', true, {2, 4, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testNoTrailingSurrogateSecond) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'d', true, {2, 1, 0x000d}},
                        {'b', true, {2, 2, 0x00db}},
                        {'d', true, {2, 3, 0x0dbd}},
                        {'5', true, {2, 4, 0x03d5}},
                        {'\\', true, {2, 5, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testStartWithTrailingSurrogate) {
  std::string expected;
  expected = "Day";
  expected += toUtf8(0xd7ff);
  Scenario scenario = {{{'D', true, {}},
                        {'a', true, {}},
                        {'y', true, {}},
                        {'\\', true, ESCAPE_CHAR},
                        {'u', true, {2, 0, 0}},
                        {'d', true, {2, 1, 0x000d}},
                        {'c', true, {2, 2, 0x00dc}},
                        {'d', true, {2, 3, 0x0dcd}},
                        {'5', true, {2, 4, 0x03d5}},
                        {'N', true, {}}},
                       expected};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidTrailingSurrogateLow) {
  Scenario scenario = {{{'p', true, {}},
                        {'r', true, {}},
                        {'e', true, {}},
                        {'\\', true, {1}},
                        {'u', true, {2, 0x00000000}},
                        {'d', true, {2, 1, 0x0000000d}},
                        {'8', true, {2, 2, 0x000000d8}},
                        {'0', true, {2, 3, 0x00000d80}},
                        {'0', true, {2, 4, 0x00000000}},
                        {'\\', true, {2, 5, 0x00000000}},
                        {'u', true, {2, 6, 0x00000000}},
                        {'d', true, {2, 7, 0x0000000d}},
                        {'8', true, {2, 8, 0x000000d8}},
                        {'0', true, {2, 9, 0x00000d80}},
                        {'0', true, {}},
                        {'s', true, {}},
                        {'u', true, {}},
                        {'f', true, {}}},
                       "pre___suf"};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidTrailingSurrogateHigh) {
  Scenario scenario = {{{'p', true, {}},
                        {'r', true, {}},
                        {'e', true, {}},
                        {'\\', true, {1}},
                        {'u', true, {2, 0x00000000}},
                        {'d', true, {2, 1, 0x0000000d}},
                        {'8', true, {2, 2, 0x000000d8}},
                        {'0', true, {2, 3, 0x00000d80}},
                        {'0', true, {2, 4, 0x00000000}},
                        {'\\', true, {2, 5, 0x00000000}},
                        {'u', true, {2, 6, 0x00000000}},
                        {'e', true, {2, 7, 0x0000000e}},
                        {'8', true, {2, 8, 0x000000e8}},
                        {'0', true, {2, 9, 0x00000e80}},
                        {'0', true, {}},
                        {'s', true, {}},
                        {'u', true, {}},
                        {'f', true, {}}},
                       "pre___suf"};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidDigit5) {
  Scenario scenario = {{{'p', true, {}},
                        {'r', true, {}},
                        {'e', true, {}},
                        {'\\', true, {1}},
                        {'u', true, {2, 0x00000000}},
                        {'d', true, {2, 1, 0x0000000d}},
                        {'8', true, {2, 2, 0x000000d8}},
                        {'0', true, {2, 3, 0x00000d80}},
                        {'0', true, {2, 4, 0x00000000}},
                        {'\\', true, {2, 5, 0x00000000}},
                        {'u', true, {2, 6, 0x00000000}},
                        {'g', true, {2, 7, 0x0000000d}},
                        {'c', true, {2, 8, 0x000000dc}},
                        {'0', true, {2, 9, 0x00000dc0}},
                        {'0', true, {}},
                        {'s', true, {}},
                        {'u', true, {}},
                        {'f', true, {}}},
                       "pre___suf"};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}

BOOST_AUTO_TEST_CASE(testInvalidDigit6) {
  Scenario scenario = {{{'p', true, {}},
                        {'r', true, {}},
                        {'e', true, {}},
                        {'\\', true, {1}},
                        {'u', true, {2, 0x00000000}},
                        {'d', true, {2, 1, 0x0000000d}},
                        {'8', true, {2, 2, 0x000000d8}},
                        {'0', true, {2, 3, 0x00000d80}},
                        {'0', true, {2, 4, 0x00000000}},
                        {'\\', true, {2, 5, 0x00000000}},
                        {'u', true, {2, 6, 0x00000000}},
                        {'d', true, {2, 7, 0x0000000d}},
                        {'g', true, {2, 8, 0x000000dc}},
                        {'0', true, {2, 9, 0x00000dc0}},
                        {'0', true, {}},
                        {'s', true, {}},
                        {'u', true, {}},
                        {'f', true, {}}},
                       "pre___suf"};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}


BOOST_AUTO_TEST_CASE(testInvalidDigit7) {
  Scenario scenario = {{{'p', true, {}},
                        {'r', true, {}},
                        {'e', true, {}},
                        {'\\', true, {1}},
                        {'u', true, {2, 0x00000000}},
                        {'d', true, {2, 1, 0x0000000d}},
                        {'8', true, {2, 2, 0x000000d8}},
                        {'0', true, {2, 3, 0x00000d80}},
                        {'0', true, {2, 4, 0x00000000}},
                        {'\\', true, {2, 5, 0x00000000}},
                        {'u', true, {2, 6, 0x00000000}},
                        {'d', true, {2, 7, 0x0000000d}},
                        {'c', true, {2, 8, 0x000000dc}},
                        {'g', true, {2, 9, 0x00000dc0}},
                        {'0', true, {}},
                        {'s', true, {}},
                        {'u', true, {}},
                        {'f', true, {}}},
                       "pre___suf"};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}


BOOST_AUTO_TEST_CASE(testInvalidDigit8) {
  Scenario scenario = {{{'p', true, {}},
                        {'r', true, {}},
                        {'e', true, {}},
                        {'\\', true, {1}},
                        {'u', true, {2, 0x00000000}},
                        {'d', true, {2, 1, 0x0000000d}},
                        {'8', true, {2, 2, 0x000000d8}},
                        {'0', true, {2, 3, 0x00000d80}},
                        {'0', true, {2, 4, 0x00000000}},
                        {'\\', true, {2, 5, 0x00000000}},
                        {'u', true, {2, 6, 0x00000000}},
                        {'d', true, {2, 7, 0x0000000d}},
                        {'c', true, {2, 8, 0x000000dc}},
                        {'0', true, {2, 9, 0x00000dc0}},
                        {'g', true, {}},
                        {'s', true, {}},
                        {'u', true, {}},
                        {'f', true, {}}},
                       "pre___suf"};
  BOOST_CHECK_THROW(scenario.execute(),
                    org::simple::util::text::JsonUnicodeEscapeException);
}


BOOST_AUTO_TEST_SUITE_END()
