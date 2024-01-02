//
// Created by michel on 31-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/text/QuoteState.h>
#include <vector>

typedef char character;
using QuoteState = org::simple::text::QuoteState<character>;
using Filter = org::simple::text::QuoteStateFilter<character>;
using Result = org::simple::text::InputFilterResult;
using Stream = org::simple::text::InputStream<character>;
template <typename S>
using FilteredStream = org::simple::text::QuoteFilteredStream<character , S>;
template <typename S>
using QuoteTokenizedStream = org::simple::text::QuoteStateTokenizedStream<character, S>;

namespace {

struct ScenarioStep {
  character input;
  character output;
  bool inQuote;

  ScenarioStep(character i, character o, bool q)
      : input(i), output(o), inQuote(q) {}
  ScenarioStep(character i, bool q) : ScenarioStep(i, i, q) {}
  ScenarioStep(character i) : ScenarioStep(i, false) {}
};

struct Scenario {
  std::vector<ScenarioStep> steps;
  bool inQuote = false;
  std::string input;
  std::string output;
  bool endInQuoteState;

  struct MyStream : public Stream {
    const std::vector<ScenarioStep> &steps;
    const ScenarioStep *step = nullptr;
    size_t pos = 0;

    MyStream(const std::vector<ScenarioStep> &listOfSteps)
        : steps(listOfSteps) {}

    bool get(character &result) override {
      if (hasNext()) {
        step = &steps.at(pos);
        result = step->input;
        pos++;
        return true;
      }
      return false;
    }
    bool hasNext() const { return pos < steps.size(); }

    const ScenarioStep &getStep() const { return *step; }
  };

  Scenario(bool endQuotedState = false) : endInQuoteState(endQuotedState) {}

  Scenario &add(character i, character o, bool q) {
    steps.push_back({i, o, q});
    inQuote = q;
    input += i;
    if (o != '\0') {
      output += o;
    }
    return *this;
  }

  Scenario &add(character i, character o) {
    return add(i, o, inQuote);
  }

  Scenario &add(character i, bool q) {
    return add(i, i, q);
  }

  Scenario &add(character i) {
    return add(i, i, inQuote);
  }

  Scenario &add(const char *text, bool quoted) {
    inQuote = quoted;
    const char *ptr = text;
    while (*ptr) {
      character c = *ptr++;
      add(c, c, inQuote);
    }
    return *this;
  }

  Scenario &add(const char *text) {
    return add(text, inQuote);
  }

  void test() const {
    Filter filter("\"");
    MyStream input(steps);
    FilteredStream<MyStream> stream(filter, input);
    std::string actualOutput;
    character c;
    while (stream.get(c)) {
      actualOutput += c;
      ScenarioStep step = input.getStep();
      BOOST_CHECK_EQUAL(step.output, c);
      BOOST_CHECK_EQUAL(step.inQuote, filter->inQuote());
    }
    BOOST_CHECK_EQUAL(endInQuoteState, filter->inQuote());
    BOOST_CHECK_EQUAL(output, actualOutput);
  }
};

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_QuoteStateFilter)

BOOST_AUTO_TEST_CASE(testNormalUnquotedTextPassesThrough) {
  Scenario scenario;
  scenario.add("Hello world!");
  scenario.test();
}

BOOST_AUTO_TEST_CASE(testQuotesAreRemovedButStateIsCorrect) {
  Scenario scenario;
  scenario.add("Hello ", false);
  scenario.add('"', '\0', true); // quote not actually checked; swallowed
  scenario.add("World!");
  scenario.add('"', '\0', false); // quote not actually checked; swallowed
  scenario.add('.', '.');
  scenario.add('.', '.');
  scenario.test();
}

BOOST_AUTO_TEST_CASE(testEscapedQuoteRemainsWithoutEscapeOutside) {
  Scenario scenario;
  scenario.add("Hello ", false);
  scenario.add('\\', '\0'); // quote not actually checked; swallowed
  scenario.add('"', '"');
  scenario.add("World!");
  scenario.test();
}

BOOST_AUTO_TEST_CASE(testEscapedQuoteRemainsWithoutEscapeInside) {
  Scenario scenario;
  scenario.add("Hello ", false);
  scenario.add('"', '\0', true); // quote not actually checked; swallowed
  scenario.add('5', '5');
  scenario.add('\\', '\0');
  scenario.add("\" total.");
  scenario.add('"', '\0', false); // quote not actually checked; swallowed
  scenario.test();
}

BOOST_AUTO_TEST_CASE(testQuoteNotClosed) {
  Scenario scenario(true);
  scenario.add("Hello ", false);
  scenario.add('"', '\0',true); // quote not actually checked; swallowed
  scenario.add("World!");
  scenario.add('.', '.');
  scenario.add('.', '.');
  scenario.test();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace
