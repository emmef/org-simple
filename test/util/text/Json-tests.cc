//
// Created by michel on 27-01-22.
//

#include "boost-unit-tests.h"
#include <org-simple/text/Json.h>
#include <org-simple/text/TextFilePosition.h>
#include <org-simple/text/UnixNewLine.h>
#include <string>

namespace {

using JsonContext = org::simple::text::JsonContext;
using JsonException = org::simple::text::JsonException;
using JsonStringBuilder = org::simple::text::JsonStringBuilder;
using Builder = org::simple::text::DefaultJsonStringBuilder;

enum class Action {
  PushIndex,
  PopIndex,
  PushName,
  PopName,
  SetString,
  SetNumber,
  SetBoolean,
  SetNull
};

static const char *actionToString(const Action &action) {
  switch (action) {
  case Action::PushIndex:
    return "PushIndex";
  case Action::PopIndex:
    return "PopIndex";
  case Action::PushName:
    return "PushName";
  case Action::PopName:
    return "PopName";
  case Action::SetString:
    return "SetString";
  case Action::SetNumber:
    return "SetNumber";
  case Action::SetBoolean:
    return "SetBoolean";
  case Action::SetNull:
    return "SetNull";
  default:
    return "INVALID";
  }
}

std::ostream &operator<<(std::ostream &out, const Action &action) {
  out << actionToString(action);
  return out;
}

class Step {
  Action action;
  const char *string = nullptr;
  bool boolean = false;
  int index = -1;

  Step(Action action_, const char *string_)
      : action(action_), string(string_) {}
  Step(bool boolean_) : action(Action::SetBoolean), boolean(boolean_) {}
  Step(int index_) : action(Action::PushIndex), index(index_) {}
  Step(Action action_) : action(action_) {}

public:
  void writeTo(std::ostream &out) const {
    out << "{" << action;
    switch (action) {
    case Action::PushIndex:
      out << " " << index;
      break;
    case Action::PushName:
    case Action::SetString:
      out << " \"" << string << '"';
      break;
    case Action::SetNumber:
      out << ' ' << string;
      break;
    case Action::SetBoolean:
      out << (boolean ? " true" : " false");
      break;
    default:
      break;
    }
    out << "}";
  }

  bool operator==(const Step &other) const {
    if (this == &other) {
      return true;
    }
    if (action != other.action) {
      return false;
    }
    switch (action) {
    case Action::PushIndex:
      return index == other.index;
    case Action::PushName:
    case Action::SetString:
    case Action::SetNumber:
      if (string == nullptr) {
        return other.string == nullptr;
      }
      return other.string != nullptr &&
             strncmp(string, other.string, 1024) == 0;
    case Action::SetBoolean:
      return boolean == other.boolean;
    case Action::PopName:
    case Action::PopIndex:
    case Action::SetNull:
      return true;
    default:
      return false;
    }
  }

  static Step pushIndex(int i) { return {i}; }
  static Step popIndex() { return {Action::PopIndex}; }
  static Step pushName(const char *name) { return {Action::PushName, name}; }
  static Step popName() { return {Action::PopName}; }
  static Step setString(const char *name) { return {Action::SetString, name}; }
  static Step setNumber(const char *name) { return {Action::SetNumber, name}; }
  static Step setNull() { return {Action::SetNull}; }
  static Step setBoolean(bool boolean) { return {boolean}; }
};

std::ostream &operator<<(std::ostream &out, const Step &step) {
  step.writeTo(out);
  return out;
}

struct Scenario {
  std::string json;
  std::vector<Step> steps;
  static thread_local int step;

  static void reset() { step = 0; }

  const Step &next() const { return steps.at(step++); }

  void writeTo(std::ostream &out) const {
    out << std::endl << "JSON" << std::endl;
    out << json << std::endl;
    out << "STEPS:" << std::endl;
    int size = steps.size();
    for (int i = 0; i < size; i++) {
      const Step s = steps.at(i);
      out << "  " << s;
    }
    out << std::endl << "END";
  }

  Scenario(std::string json_, std::initializer_list<Step> steps_)
      : json(json_), steps(steps_) {}
};

thread_local int Scenario::step;

std::ostream &operator<<(std::ostream &out, const Scenario &scenario) {
  scenario.writeTo(out);
  return out;
}

class Context : public JsonContext {
  class BailOutException : public std::exception {};
  const Scenario &scenario;
  Builder name;
  Builder string;

  class Stream : public org::simple::text::InputStream<char> {
    class StringStream : org::simple::text::InputStream<char> {
      const std::string &input;
      size_t at = 0;

    public:
      StringStream(const std::string &string) : input(string) {}
      bool get(char &c) override {
        if (at < input.length()) {
          c = input[at++];
          return true;
        }
        return false;
      }
    } stringStream;

    org::simple::text::UnixNewLineStream<char, StringStream> nlStream;
    org::simple::text::TextFilePositionData<char> position;

  public:
    Stream(const std::string &string)
        : stringStream(string), nlStream(stringStream) {}
    bool get(char &c) override {
      if (nlStream.get(c)) {
        position.probe(c);
        return true;
      }
      return false;
    }

    const auto &getPosition() const { return position; }
  };

public:
  Context(const Scenario &scenario1, size_t nameLength = 16,
          size_t stringLength = 16)
      : scenario(scenario1), name(nameLength, 0), string(stringLength, 0) {}

  JsonStringBuilder &nameBuilder() final { return name; }
  JsonStringBuilder &stringBuilder() final { return string; }

  void pushIndex(int index) override {
    Step expected = scenario.next();
    Step actual = Step::pushIndex(index);
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void popIndex() override {
    Step expected = scenario.next();
    Step actual = Step::popIndex();
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void pushName(const char *name) override {
    Step expected = scenario.next();
    Step actual = Step::pushName(name);
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void popName() override {
    Step expected = scenario.next();
    Step actual = Step::popName();
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void setString(const char *string) override {
    Step expected = scenario.next();
    Step actual = Step::setString(string);
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void setNumber(const char *string) override {
    Step expected = scenario.next();
    Step actual = Step::setNumber(string);
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void setBoolean(bool value) override {
    Step expected = scenario.next();
    Step actual = Step::setBoolean(value);
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }
  void setNull() override {
    Step expected = scenario.next();
    Step actual = Step::setNull();
    BOOST_CHECK_EQUAL(expected, actual);
    if (expected != actual) {
      throw BailOutException();
    }
  }

  std::string addPosition(const Stream &stream, const std::exception &e) {
    const auto &pos = stream.getPosition();
    std::string m = std::to_string(pos.getLine());
    m += ":";
    m += std::to_string(pos.getColumn());
    m += ": ";
    m += e.what();
    return m;
  }

  void test() {
    Scenario::reset();
    Stream stream(scenario.json);

    try {
      JsonContext ::readJson(*this, stream);
      int expectedStepsExecuted = scenario.steps.size();
      int actualStepsExecuted = Scenario::step;
      BOOST_CHECK_EQUAL(expectedStepsExecuted, actualStepsExecuted);
    } catch (const BailOutException &e) {
      BOOST_FAIL(addPosition(stream, e));
      // Stopping test here.
    } catch (const JsonException &e) {
      BOOST_FAIL(addPosition(stream, e));
    }
  }
};

std::vector<Scenario> getInitialScenarios() {
  std::vector<Scenario> scenarios;

  scenarios.push_back({"{}", {}});

  scenarios.push_back(
      {"{ \"name\" : true }",
       {Step::pushName("name"), Step::setBoolean(true), Step::popName()}});

  scenarios.push_back(
      {"{ \"name\" : null }",
       {Step::pushName("name"), Step::setNull(), Step::popName()}});

  scenarios.push_back(
      {"{ \"name\" : false }",
       {Step::pushName("name"), Step::setBoolean(false), Step::popName()}});

  scenarios.push_back(
      {"{ \"name\" : 13.84 }",
       {Step::pushName("name"), Step::setNumber("13.84"), Step::popName()}});

  scenarios.push_back({
      R"({
      "name" : "string value"
      })",
      {Step::pushName("name"), Step::setString("string value"),
       Step::popName()}});

  scenarios.push_back(
      {R"({ "name1" : true, "name2" : 13 })",
       {Step::pushName("name1"), Step::setBoolean(true), Step::popName(),
        Step::pushName("name2"), Step::setNumber("13"), Step::popName()}});

  scenarios.push_back(
      {R"({ "name1" : true, "name2" : { "name2.1" : 13}} })",
       {Step::pushName("name1"), Step::setBoolean(true), Step::popName(),
        Step::pushName("name2"), Step::pushName("name2.1"),
        Step::setNumber("13"), Step::popName(), Step::popName()}});

  scenarios.push_back(
      {R"({ "name" : [13.84, 16.3,18.9] })",
       {Step::pushName("name"), Step::pushIndex(0), Step::setNumber("13.84"),
        Step::popIndex(), Step::pushIndex(1), Step::setNumber("16.3"),
        Step::popIndex(), Step::pushIndex(2), Step::setNumber("18.9"),
        Step::popIndex(), Step::popName()}});

  scenarios.push_back(
      {R"({ "name" : [13.84, { "name2" : 18.9 }] })",
       {Step::pushName("name"), Step::pushIndex(0), Step::setNumber("13.84"),
        Step::popIndex(), Step::pushIndex(1), Step::pushName("name2"),
        Step::setNumber("18.9"), Step::popName(), Step::popIndex(),
        Step::popName()}});

  scenarios.push_back(
      {R"({ "name" : [13.84, [16.3,18.9]] })",
       {Step::pushName("name"), Step::pushIndex(0), Step::setNumber("13.84"),
        Step::popIndex(), Step::pushIndex(1), Step::pushIndex(0),
        Step::setNumber("16.3"), Step::popIndex(), Step::pushIndex(1),
        Step::setNumber("18.9"), Step::popIndex(), Step::popIndex(),
        Step::popName()}});

  return scenarios;
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_JsonContext)

BOOST_DATA_TEST_CASE(testInitialScenarios, getInitialScenarios()) {
  Context context(sample);

  context.test();
}

BOOST_AUTO_TEST_SUITE_END()
