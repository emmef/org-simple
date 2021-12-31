//
// Created by michel on 31-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/util/Predicate.h>
#include <org-simple/util/text/QuoteState.h>
#include <org-simple/util/text/StringStream.h>
#include <vector>

typedef char character;
using Filter = org::simple::util::text::QuoteStateFilter<character>;
using Predicate = org::simple::util::Predicate<character>;
using Result = org::simple::util::text::InputFilterResult;
using Stream = org::simple::util::text::InputStream<character>;
using StringStream = org::simple::util::text::StringInputStream<character>;
using FilteredStream = org::simple::util::text::QuoteFilteredStream<character>;
using QuoteTokenizedStream =
    org::simple::util::text::QuoteStateTokenizedStream<character>;

namespace {

struct Scenario {
  std::string input;
  std::vector<std::string> expected;
  bool usePredicate = false;

  Scenario(const std::string &string, bool skipWhitespace)
      : input(string), usePredicate(skipWhitespace) {}

  Scenario(const std::string &string, bool skipWhitespace,
           const std::initializer_list<std::string> &list)
      : Scenario(string, skipWhitespace) {
    for (const auto exp : list) {
      this->operator<<(exp);
    }
  }

  Scenario &operator<<(const std::string &expect) {
    expected.push_back(expect);
    return *this;
  }
  struct SeparatorPredicate : public Predicate {
    bool test(const character &c) const override {
      return c == ' ';
    }
  } predicate;

  void test() const {
    Filter filter("\"");
    StringStream string(input);
    FilteredStream stream(filter, string);
    QuoteTokenizedStream tokStream(stream, usePredicate ? &predicate : nullptr);
    std::string builder;
    std::vector<std::string> actual;
    do {
      builder.clear();
      character c;
      while (tokStream.get(c)) {
        builder += c;
      }
      if (builder.length() > 0) {
        actual.push_back(builder);
      }
    }
    while (!tokStream.isExhausted());
    BOOST_CHECK_EQUAL(expected.size(), actual.size());
    size_t minLength = std::min(expected.size(), actual.size());
    for (size_t i = 0; i < minLength; i++) {
      BOOST_CHECK_EQUAL(expected.at(i), actual.at(i));
    }
  }
};

std::vector<Scenario> generateScenarios() {
  std::vector<Scenario> results;

  results.push_back({"Hello", false, {"Hello"}});
  results.push_back({" Hello", false, {" Hello"}});
  results.push_back({"Hello ", false, {"Hello "}});
  results.push_back({" Hello ", false, {" Hello "}});

  results.push_back({"Hello world!", false, {"Hello world!"}});
  results.push_back({" Hello world!", false, {" Hello world!"}});
  results.push_back({"Hello world! ", false, {"Hello world! "}});
  results.push_back({" Hello world! ", false, {" Hello world! "}});

  results.push_back({"Hello world!", true, {"Hello", "world!"}});

  results.push_back({"Hello world! ", true, {"Hello", "world!"}});

  results.push_back(
      {"Hello \"dear\" viewers!", false, {"Hello ", "dear", " viewers!"}});

  results.push_back({"Hello \"de\\\"ar\" viewers!",
                     false,
                     {"Hello ", "de\"ar", " viewers!"}});

  results.push_back({"Hello \"my fellow\" viewers!",
                     false,
                     {"Hello ", "my fellow", " viewers!"}});

  results.push_back(
      {"Hello \"dear\" viewers!", true, {"Hello", "dear", "viewers!"}});

  results.push_back(
      {"Hello \"de\\\"ar\" viewers!", true, {"Hello", "de\"ar", "viewers!"}});

  results.push_back({"Hello \"my fellow\" viewers!",
                     true,
                     {"Hello", "my fellow", "viewers!"}});

  results.push_back({"Hello today, \"my fellow\" viewers!",
                     true,
                     {"Hello", "today,", "my fellow", "viewers!"}});

  return results;
}

static std::ostream &operator<<(std::ostream &out, const Scenario &scenario) {
  out << "SCENARIO (skip-whitespace=" << (scenario.usePredicate ? "yes" : "no");
  out << ") '" << scenario.input << "' =>";
  for (auto exp : scenario.expected) {
    out << "   '" << exp << "'";
  }
  return out;
}

} // anonymous namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_QuoteStateTokenizedStream)

BOOST_DATA_TEST_CASE(testScenarios, generateScenarios()) { sample.test(); }

BOOST_AUTO_TEST_SUITE_END()
