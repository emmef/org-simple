//
// Created by michel on 01-01-22.
//

#include <org-simple/util/text/TokenizedStream.h>
#include "boost-unit-tests.h"
#include <functional>
#include <org-simple/util/text/ReplayStream.h>
#include <org-simple/util/text/StringStream.h>

using namespace org::simple::util::text;

static_assert(hasInputStreamSignature<TokenizedInputStream<char>, char>);

namespace {

struct Sub : public TokenizedInputStream<char> {
  InputStream<char> &empty = InputStream<char>::empty();
  bool ex = false;
  bool get(char &result) override {
    char c;
    if (empty.get(c)) {
      result = c;
      return true;
    }
    ex = true;
    return false;
  }

  bool isExhausted() const override { return ex; }
  /**
   * Resets the exhausted-state, which is useful if this token stream is based
   * upon yet another token stream.
   */
  void resetExhausted() override { ex = false; }
};


struct Scenario {
  const std::string input;
  const std::vector<std::string> expected;

  Scenario(const std::string &in, std::initializer_list<std::string> list)
      : input(in), expected(list) {}

  void write(std::ostream &out) const {
    out << "'" << input << "' =>";
    for (auto tok : expected) {
      out << "   '" << tok << "'";
    }
  }
};

static std::ostream& operator << (std::ostream &out, const Scenario &scenario) {
  scenario.write(out);
  return out;
}

std::vector<Scenario> generateScenarios() {
  std::vector<Scenario> scenarios;

  scenarios.push_back({ "Hello, world!", {"Hello", "world!"}});
  scenarios.push_back({ "Hello , world!", {"Hello", "world!"}});
  scenarios.push_back({ " ,Hello, world!", {"", "Hello", "world!"}});
  scenarios.push_back({ " , Hello, world!", {"", "Hello", "world!"}});
  scenarios.push_back({ "Hello,, world!", {"Hello", "", "world!"}});
  scenarios.push_back({ "Hello, , world!", {"Hello", "", "world!"}});
  scenarios.push_back({ "Hello , , world!", {"Hello", "", "world!"}});
  scenarios.push_back({ "Hello ,, world!", {"Hello", "", "world!"}});
  scenarios.push_back({ "Hello, world!,", {"Hello", "world!", ""}});
  return scenarios;
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_TokenizedStream)

BOOST_AUTO_TEST_CASE(testDummyStateLogic) {
  Sub sub;

  BOOST_CHECK_EQUAL(false, sub.isExhausted());
  char c;
  BOOST_CHECK_EQUAL(false, sub.get(c));
  BOOST_CHECK_EQUAL(true, sub.isExhausted());
  sub.resetExhausted();
  BOOST_CHECK_EQUAL(false, sub.isExhausted());
  BOOST_CHECK_EQUAL(false, sub.get(c));
  BOOST_CHECK_EQUAL(true, sub.isExhausted());
}

BOOST_DATA_TEST_CASE(testTokenizedScenarios, generateScenarios()) {
  StringInputStream<char> input = sample.input;
  PredicateTokenStream<char, StringInputStream<char>> stream(
      input,                                  //
      [](const char &c) { return c == ','; }, //
      [](const char &c) { return c == ' '; });

  std::vector<std::string> actual;
  while (!stream.isExhausted()) {
    std::string token;
    char c;
    while (stream.get(c)) {
      token += c;
    }
    actual.push_back(token);
  }
  BOOST_CHECK_EQUAL(sample.expected.size(), actual.size());
  size_t count = std::min(sample.expected.size(), actual.size());
  for (size_t i = 0; i < count; i++) {
    BOOST_CHECK_EQUAL(sample.expected[i], actual[i]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
