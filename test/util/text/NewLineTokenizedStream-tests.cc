//
// Created by michel on 31-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/util/Predicate.h>
#include <org-simple/util/text/StringStream.h>
#include <org-simple/util/text/UnixNewLine.h>
#include <vector>
#include <algorithm>

typedef char character;
using Stream = org::simple::util::text::InputStream<character>;
using StringStream = org::simple::util::text::StringInputStream<character>;
using TokenizedStream =
    org::simple::util::text::NewlineTokenizedStream<character, StringStream>;

namespace {

struct Scenario {
  std::string input;
  std::vector<std::string> expected;

  Scenario(const std::string &string,
           const std::initializer_list<std::string> &list)
      : input(string) {
    for (const auto &exp : list) {
      this->operator<<(exp);
    }
  }

  Scenario &operator<<(const std::string &expect) {
    expected.push_back(expect);
    return *this;
  }

  void test() const {
    StringStream string(input);
    TokenizedStream tokStream(string);
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
    } while (!tokStream.isExhausted());
    BOOST_CHECK_EQUAL(expected.size(), actual.size());
    size_t minLength = std::min(expected.size(), actual.size());
    for (size_t i = 0; i < minLength; i++) {
      BOOST_CHECK_EQUAL(expected.at(i), actual.at(i));
    }
  }
};

std::string trim(const std::string &input) {
  ssize_t start = -1;
  ssize_t end = input.length();
  ssize_t lastNW = -1;

  for (ssize_t i = 0; i < end; i++) {
    character c = input.at(i);
    if (start < 0) {
      if (c == '\n') {
        continue;
      }
      start = i;
    }
    else if (c != '\n'){
      lastNW = i;
    }
  }
  if (start < 0 || lastNW < start) {
    return "";
  }
  return input.substr(start, lastNW + 1 - start);
}

std::vector<Scenario> generateScenarios() {
  std::vector<std::string> phrases1;
  phrases1.push_back("Hello");
  phrases1.push_back("Hello world!");
  phrases1.push_back("Hello \"dear\" 9\\\" world!");

  std::vector<std::string> phrases2;
  for (auto phrase : phrases1) {
    std::string newPhrase = phrase;
    phrases2.push_back(newPhrase);
    newPhrase += " ";
    phrases2.push_back(newPhrase);
    newPhrase = " ";
    newPhrase += phrase;
    phrases2.push_back(newPhrase);
    newPhrase += " ";
    phrases2.push_back(newPhrase);
  }

  std::vector<std::string> nls;
  nls.push_back("\n");
  nls.push_back("\n\n");

  std::vector<std::string> nlPhrases;
  // scenarios to check that leading and trailing newlines make no difference.
  for (auto phrase : phrases2) {
    for (auto nl : nls) {
      std::string newPhrase = phrase;
      nlPhrases.push_back(newPhrase);
      newPhrase += nl;
      nlPhrases.push_back(newPhrase);
      newPhrase = nl;
      newPhrase += phrase;
      nlPhrases.push_back(newPhrase);
      newPhrase += nl;
      nlPhrases.push_back(newPhrase);
    }
  }

  std::vector<Scenario> results;

  // Scenarios to check no whitepsace or other stuff is eaten.
  for (auto phrase : phrases2) {
    results.push_back({phrase, {phrase}});
  }

  // scenarios to check that leading and trailing newlines make no difference.
  for (auto phrase : nlPhrases) {
    results.push_back({phrase, { trim(phrase) }});
  }

  // multiple lines
  std::string p;
  std::string pp;
  for (auto phrase : nlPhrases) {
    if (pp.length() > 0) {
      std::string newPhrase = pp;
      newPhrase += "\n";
      newPhrase += p;
      newPhrase += "\n";
      newPhrase += phrase;
      results.push_back({newPhrase, { trim(pp), trim(p), trim(phrase) }});
    }
    pp = p;
    p = phrase;
  }

  return results;
}

std::string escapeNl(const std::string &input) {
  std::string result;
//  character buf[2];
//  buf[1] = '\0';
  for (const auto c : input) {
    if (c != '\n') {
//      buf[0] = c;
      result += c;//.append(buf);
    }
    else {
      result += "\\n";
    }
  }
  return result;
}

static std::ostream &operator<<(std::ostream &out, const Scenario &scenario) {
  out << "SCENARIO '" << escapeNl(scenario.input) << "' =>";
  for (auto exp : scenario.expected) {
    out << "   '" << escapeNl(exp) << "'";
  }
  return out;
}

} // anonymous namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_NewLineTokenizedStream)

BOOST_DATA_TEST_CASE(testScenarios, generateScenarios()) { sample.test(); }

BOOST_AUTO_TEST_SUITE_END()
