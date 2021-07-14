//
// Created by michel on 12-07-21.
//

#include "test-helper.h"
#include <org-simple/util/SignalManager.h>
#include <org-simple/util/FakeClock.h>
#include <vector>

using SignalType = org::simple::util::SignalType;
using Signal = org::simple::util::Signal;
using ext_type = org::simple::util::Signal::external_type;
using SignalManager = org::simple::util::SignalManager;
using SignalResult = org::simple::util::SignalResult;
using FakeClock = org::simple::util::FakeClock;

class AbstractScenario {
  Signal first_;
  Signal second_;

public:
  AbstractScenario(Signal first, Signal second)
      : first_(first), second_(second) {}

  virtual const char *description() const = 0;
  virtual ~AbstractScenario() = default;

  Signal first() const { return first_; }
  Signal second() const { return second_; }
};

class SetFirstThenSecond : public AbstractScenario {
public:
  SetFirstThenSecond(Signal first, Signal second)
      : AbstractScenario(first, second) {}

  const char *description() const final { return "Set first then last"; }

  void setFirstThenSecond() const {
    SignalManager manager;

    BOOST_CHECK(manager.set_signal(first()) == SignalResult::SUCCESS);
    BOOST_CHECK_EQUAL(first(), manager.get_signal());

    if (first().terminates()) {
      BOOST_CHECK(manager.set_signal(second()) == SignalResult::NOT_ALLOWED);
      BOOST_CHECK_EQUAL(first(), manager.get_signal());
    } else {
      BOOST_CHECK(manager.set_signal(second()) == SignalResult::SUCCESS);
      BOOST_CHECK_EQUAL(second(), manager.get_signal());
    }
  }
};

namespace std {
template <typename C, typename V>
std::basic_ostream<C> &
operator<<(std::basic_ostream<C> &out,
           const org::simple::util::AbstractSignal<V> &signal) {
  out << signal.type_name() << "{";
  if (signal.type() != SignalType::NONE) {
    out << signal.value();
    if (signal.terminates()) {
      out << ", terminates";
    }
  }
  out << "}";
  return out;
}

template <typename C>
std::basic_ostream<C> &operator<<(std::basic_ostream<C> &out,
                                  const AbstractScenario &scenario) {
  out << "Scenario{\"" << scenario.description()
      << "\"; first=" << scenario.first() << "; second=" << scenario.second()
      << "}";
  return out;
}
} // namespace std

std::vector<ext_type> &generateTestValues() {
  static std::vector<ext_type> values;

  values.push_back(0);
  values.push_back(1u);
  values.push_back(2u);
  values.push_back(Signal::MAX_VALUE / 4);
  values.push_back(Signal::MAX_VALUE / 3);
  values.push_back(Signal::MAX_VALUE / 2);
  values.push_back(Signal::MAX_VALUE - 1);
  values.push_back(Signal::MAX_VALUE);

  return values;
}

std::vector<ext_type> &getTestValues() {
  static std::vector<ext_type> values = generateTestValues();
  return values;
}

std::vector<Signal> &generateTestSignals() {
  static std::vector<Signal> signals;

  for (ext_type value : getTestValues()) {
    if (value == 0) {
      signals.push_back({});
    } else {
      signals.push_back(Signal::system(value, false));
      signals.push_back(Signal::program(value, false));
      signals.push_back(Signal::user(value, false));
      signals.push_back(Signal::system(value, true));
      signals.push_back(Signal::program(value, true));
      signals.push_back(Signal::user(value, true));
    }
  }

  return signals;
}

std::vector<Signal> &getTestSignals() {
  static std::vector<Signal> signals = generateTestSignals();
  return signals;
}

static const std::vector<SetFirstThenSecond> &generateScnenarios() {
  static std::vector<SetFirstThenSecond> scenarios;

  for (Signal first : getTestSignals()) {
    for (Signal second : getTestSignals()) {
      scenarios.push_back({first, second});
    }
  }

  std::cout << "Test scenarios: " << scenarios.size() << std::endl;

  return scenarios;
}

static const std::vector<SetFirstThenSecond> &getScenarios() {
  static const std::vector<SetFirstThenSecond> scenarios = generateScnenarios();

  return scenarios;
}

BOOST_AUTO_TEST_SUITE(org_simple_util_SignalManager)

BOOST_AUTO_TEST_CASE(testInstanceNoSignalAtStart) {
  SignalManager manager;
  Signal sig = manager.get_signal();

  BOOST_CHECK(sig.type() == org::simple::util::SignalType::NONE);
  BOOST_CHECK(sig.value() == 0);
}

BOOST_DATA_TEST_CASE(testAssignTwice, getScenarios()) {
  sample.setFirstThenSecond();
}

BOOST_AUTO_TEST_SUITE_END()
