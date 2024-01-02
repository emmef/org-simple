//
// Created by michel on 12-07-21.
//

#include "test-helper.h"
#include <mutex>
#include <org-simple/FakeClock.h>
#include <org-simple/SignalManager.h>
#include <thread>
#include <vector>

using SignalType = org::simple::SignalType;
using Signal = org::simple::Signal;
using wrapped_type = org::simple::Signal::value_type;
using SignalManager = org::simple::SignalManager;
using SignalResult = org::simple::SignalResult;
using FakeClock = org::simple::FakeClock;

class AbstractScenario {
  Signal first_;
  Signal second_;

public:
  AbstractScenario(Signal first, Signal second)
      : first_(first), second_(second) {}

  [[nodiscard]] virtual const char *description() const = 0;
  virtual ~AbstractScenario() = default;

  [[nodiscard]] Signal first() const { return first_; }
  [[nodiscard]] Signal second() const { return second_; }
};

namespace std {
template <typename C, typename V>
std::basic_ostream<C> &
operator<<(std::basic_ostream<C> &out,
           const org::simple::AbstractSignal<V> &signal) {
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

class null_out_buf : public std::streambuf {
public:
  std::streamsize xsputn(const char *, std::streamsize n) final { return n; }
  int overflow(int) final { return 1; }
};

class null_out_stream : public std::ostream {
public:
  null_out_stream() : std::ostream(&buf) {}

private:
  null_out_buf buf;
};

class SetFirstThenSecond : public AbstractScenario {
public:
  SetFirstThenSecond(Signal first, Signal second)
      : AbstractScenario(first, second) {}

  [[nodiscard]] const char *description() const final {
    return "Set first then last";
  }

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

class SetFirstWaitSecondThreadSetsSecond : public AbstractScenario {

  class Executor : public AbstractScenario {
    std::atomic<bool> signal_thread_started_ = false;
    std::atomic<bool> wait_thread_started_ = false;
    std::atomic<bool> signal_thread_stopped_ = false;
    std::atomic<bool> wait_thread_stopped_ = false;
    std::atomic<bool> inside_wait_ = false;
    std::mutex mutex_;
    Signal result_;
    bool had_signal_;

    org::simple::AbstractSignalManager<unsigned char, true> manager_;

    static constexpr long TIMEOUT = 2;

    bool while_with_max_iterations(const std::function<bool()> &true_func,
                                   const char *msg) {
      auto now = std::chrono::system_clock::now();
      auto duration = std::chrono::seconds(1);
      for (int i = 0;
           i < 1000000 && (std::chrono::system_clock::now() - now < duration);
           i++) {
        for (int j = 0; j < 1000000; j++) {
          if (!true_func()) {
            return true;
          }
        }
      }
      const char *m = msg ? msg : "";
      std::cerr << "ERROR: while_max did not finish in max iterations: " << m
                << std::endl;
      return false;
    }

    class StopStartGuard {
      [[maybe_unused]] std::atomic<bool> &started_;
      [[maybe_unused]] std::atomic<bool> &stopped_;

    public:
      StopStartGuard(std::atomic<bool> &started, std::atomic<bool> &stopped)
          : started_(started), stopped_(stopped) {
        started_ = true;
      }
      ~StopStartGuard() { stopped_ = true; }
    };

    bool started() {
      return signal_thread_started_.load() && wait_thread_started_.load();
    }

    bool stopped() {
      return signal_thread_stopped_.load() && wait_thread_stopped_.load();
    }

    static void set_inside_wait(void *data) {
      if (data) {
        static_cast<Executor *>(data)->inside_wait_ = true;
      }
    }

    void signal_thread_worker() {
      StopStartGuard guard(signal_thread_started_, signal_thread_stopped_);
      while_with_max_iterations([this]() { return !inside_wait_; },
                                "Await wait thread to be inside initial wait");
      manager_.set_signal(second());
      inside_wait_ = false;
      std::this_thread::yield();
      while_with_max_iterations(
          [this]() { return !(inside_wait_ || wait_thread_stopped_); },
          "Await wait thread to be inside wait after signal set");
      std::this_thread::yield();
      FakeClock ::set_now(FakeClock::now() + FakeClock::duration(TIMEOUT + 1));
    }

    void waiting_thread_worker() {
      StopStartGuard guard(wait_thread_started_, wait_thread_stopped_);
      Signal result = first();
      bool has_signal = manager_.busy_wait_until(
          result, FakeClock::now() + FakeClock::duration(TIMEOUT));
      {
        std::unique_lock<std::mutex> lock(mutex_);
        result_ = result;
        had_signal_ = has_signal;
      }
    }

    static void signal_thread(Executor *instance) {
      static_cast<Executor *>(instance)->signal_thread_worker();
    }

    static void waiting_thread(Executor *instance) {
      static_cast<Executor *>(instance)->waiting_thread_worker();
    }

    void verifyResults() {
      bool had_signal;
      Signal result;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        had_signal = had_signal_;
        result = result_;
      }
      bool expected_signal = second().is_signal();
      BOOST_CHECK_EQUAL(expected_signal, had_signal);
      if (expected_signal) {
        BOOST_CHECK_EQUAL(second(), result);
      } else {
        BOOST_CHECK_EQUAL(first(), result_);
      }
    }

  public:
    static void execute(Signal sig1, Signal sig2) {
      auto *instance = new Executor(sig1, sig2);
      instance->manager_.set_callback(set_inside_wait, instance);
      FakeClock::set_count(0);
      std::thread wt(waiting_thread, instance);
      std::thread st(signal_thread, instance);
      if (!instance->while_with_max_iterations(
              [instance]() { return !instance->started(); },
              "Waiting for threads to start")) {
        std::cerr << "Test threads failed to start " << *instance << std::endl;
      } else if (instance->while_with_max_iterations(
                     [instance]() { return !instance->stopped(); },
                     "Waiting for threads to stop")) {
        wt.join();
        st.join();
        instance->verifyResults();
        delete instance;
      } else {
        wt.detach();
        st.detach();
        std::cerr << "Threads took too long: detached!" << std::endl;
      }
    }

    Executor(Signal first, Signal second)
        : AbstractScenario(first, second), had_signal_(false) {}
    [[nodiscard]] const char *description() const final {
      return "Set first in result, await signal, set second in other thread";
    }
  };

public:
  void execute() const { Executor::execute(first(), second()); }

  SetFirstWaitSecondThreadSetsSecond(Signal first, Signal second)
      : AbstractScenario(first, second) {}

  [[nodiscard]] const char *description() const final {
    return "Wait for first; second thread sets second.";
  }
};

std::vector<wrapped_type> &generateTestValues() {
  static std::vector<wrapped_type> values;

  values.emplace_back(0);
  values.emplace_back(1u);
  values.emplace_back(2u);
  values.emplace_back(Signal::maxValue / 4);
  values.emplace_back(Signal::maxValue / 3);
  values.emplace_back(Signal::maxValue / 2);
  values.emplace_back(Signal::maxValue - 1);
  values.emplace_back(Signal::maxValue);

  return values;
}

std::vector<wrapped_type> &getTestValues() {
  static std::vector<wrapped_type> values = generateTestValues();
  return values;
}

std::vector<Signal> &generateTestSignals() {
  static std::vector<Signal> signals;

  for (wrapped_type value : getTestValues()) {
    if (value == 0) {
      signals.emplace_back(Signal());
    } else {
      signals.emplace_back(Signal::system(value, false));
      signals.emplace_back(Signal::program(value, false));
      signals.emplace_back(Signal::user(value, false));
      signals.emplace_back(Signal::system(value, true));
      signals.emplace_back(Signal::program(value, true));
      signals.emplace_back(Signal::user(value, true));
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
      scenarios.emplace_back(SetFirstThenSecond(first, second));
    }
  }

  return scenarios;
}

static const std::vector<SetFirstThenSecond> &getScenarios() {
  static const std::vector<SetFirstThenSecond> scenarios = generateScnenarios();

  return scenarios;
}

static const std::vector<SetFirstWaitSecondThreadSetsSecond> &
generateScnenarios2() {
  static std::vector<SetFirstWaitSecondThreadSetsSecond> scenarios;

  for (Signal first : getTestSignals()) {
    for (Signal second : getTestSignals()) {
      scenarios.emplace_back(SetFirstWaitSecondThreadSetsSecond(first, second));
    }
  }

  return scenarios;
}

static const std::vector<SetFirstWaitSecondThreadSetsSecond> &getScenarios2() {
  static const std::vector<SetFirstWaitSecondThreadSetsSecond> scenarios =
      generateScnenarios2();

  return scenarios;
}

BOOST_AUTO_TEST_SUITE(org_simple_util_SignalManager)

BOOST_AUTO_TEST_CASE(testInstanceNoSignalAtStart) {
  SignalManager manager;
  Signal sig = manager.get_signal();

  BOOST_CHECK(sig.type() == org::simple::SignalType::NONE);
  BOOST_CHECK(sig.value() == 0);
}

BOOST_DATA_TEST_CASE(testAssignTwice, getScenarios()) {
  sample.setFirstThenSecond();
}

BOOST_DATA_TEST_CASE(tesWaitForSignal, getScenarios2()) { sample.execute(); }

BOOST_DATA_TEST_CASE(testSetThenWait, getScenarios2()) {
  SignalManager man;
  Signal empty;
  bool expectedSignal = sample.second().is_signal();
  Signal expected = expectedSignal ? sample.second() : empty;
  Signal result;
  man.set_signal(expected);
  BOOST_CHECK_EQUAL(expectedSignal, man.busy_wait_spin(result, 1000));
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()
