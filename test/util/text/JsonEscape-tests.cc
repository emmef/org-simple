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
  out << "JsonEscapeState{";
  switch (state.type) {
  case 0:
    out << '-';
    break;
  case 1:
    out << '\\';
    break;
  case 2:
    out << "\\u";
    break;
  }
  if (state.count) {
    out << "; " << state.count;
  }
  if (state.value != 0 || state.type == 2) {
    uint32_t value = state.value;
    for (int i = 0, j = 7; i < 8; i++, j--) {
      hexNumber[j] = digits[value % 16];
      value /= 16;
    }
    hexNumber[8] = '\0';
    out << "; 0x" << hexNumber;
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
  out << "Step{append('";
  if (step.input < ' ' || step.input >= 126) {
    out << "\\x" << digits[step.input / 16] << digits[step.input % 16];
  } else {
    out << step.input;
  }
  out << "') -> " << (step.appendResult ? "true" : "false");
  out << "; " << step.escapeStateAfterAppend << "}";
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
  expected = "Day";
  expected += toUtf8(0x10437);
  results.push_back({{
                         {'D', true, {}},
                         {'a', true, {}},
                         {'y', true, {}},
                         {'\\', true, ESCAPE_CHAR},
                         {'u', true, {2, 0, 0}},
                         {'d', true, {2, 1, 0x000d}},
                         {'8', true, {2, 2, 0x00d8}},
                         {'0', true, {2, 3, 0x0d80}},
                         {'1', true, {2, 4, 0x0001}},
                         {'\\', true, {2, 5, 0x0001}},
                         {'u', true, {2, 6, 0x0001}},
                         {'d', true, {2, 7, 0x0000001d}},
                         {'c', true, {2, 8, 0x000001dc}},
                         {'3', true, {2, 9, 0x00001dc3}},
                         {'7', true, {}},  // 0x0001dc37 - dc00 = 0x00010037
                     },
                     expected});
  return results;
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_AddJsonStringCharacter)

BOOST_DATA_TEST_CASE(testSimpleScenarios, simpleScenarios()) {
  sample.execute();
}

BOOST_AUTO_TEST_SUITE_END()
