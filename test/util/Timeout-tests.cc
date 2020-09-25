//
// Created by michel on 24-09-20.
//

#include "test-helper.h"
#include <org-simple/util/Timeout.h>

using Fake = org::simple::util::TimeoutFakedClock<std::chrono::steady_clock>;


BOOST_AUTO_TEST_SUITE(org_simple_util_Timeout)

BOOST_AUTO_TEST_CASE(testTimeoutFakeClockIntegerArgument) {
  Fake timeout10(Fake::type_duration(10));
  Fake timeout13(Fake::type_duration(13));

  BOOST_CHECK_MESSAGE(timeout10.duration().count() == 10,
                      "Duration as given at construction");

  BOOST_CHECK_MESSAGE(timeout13.duration().count() == 13,
                      "Duration as given at construction");
}

BOOST_AUTO_TEST_CASE(testTimeoutTestMechanism) {
  Fake timeout(std::chrono::milliseconds(10));

  BOOST_CHECK_MESSAGE(timeout.duration().count() == 10,
                      "Duration as given at construction");

  Fake::type_time_point start = timeout.now();
  Fake::type_time_point deadline = start + timeout.duration();

  timeout.start();
  bool timed_out = false;
  while (!timeout.timed_out()) {
    if (timeout.now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
      timed_out = true;
    }
    timeout.set_now(timeout.now() + Fake::type_duration(1));
  }
  BOOST_CHECK(true);
}


BOOST_AUTO_TEST_CASE(testTimeoutUsingClock) {
  using tuc = org::simple::util::TimeoutUsingClock<std::chrono::steady_clock>;
  tuc timeout(std::chrono::milliseconds(100));

  timeout.start();
  tuc::type_time_point start = timeout.started();
  tuc::type_time_point deadline = start + timeout.duration();

  bool timed_out = false;
  while (!timeout.timed_out()) {
    if (tuc::clock::now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
      timed_out = true;
    }
  }
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleep) {
  using tuc = org::simple::util::TimeoutSlicedSleep<std::chrono::steady_clock>;
  tuc timeout(std::chrono::milliseconds(500), 10);

  tuc::type_time_point start = tuc::clock::now();
  tuc::type_time_point deadline = start + timeout.duration();

  timeout.start();
  bool timed_out = false;
  int slices = 0;
  while (!timeout.timed_out()) {
    if (tuc::clock::now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
      timed_out = true;
    }
    slices++;
  }
  if (slices > 11) {
    std::cout << "Used " << slices << " slices." << std::endl;
    BOOST_FAIL("Too many slices used");
  }
}

BOOST_AUTO_TEST_CASE(testTimeOutImmediate) {
  org::simple::util::Timeout &to = org::simple::util::TimeoutImmediately::instance();

  BOOST_CHECK_EQUAL(true, to.timed_out());
  to.start();
  BOOST_CHECK_EQUAL(true, to.timed_out());
}

BOOST_AUTO_TEST_CASE(testTimeOutNever) {
  org::simple::util::Timeout &to = org::simple::util::TimeoutNever::instance();

  BOOST_CHECK_EQUAL(false, to.timed_out());
  to.start();
  BOOST_CHECK_EQUAL(false, to.timed_out());
}

BOOST_AUTO_TEST_CASE(testTimeoutStart) {
  struct TimeOutImp : public org::simple::util::Timeout {
    void start() noexcept override {}
    [[nodiscard]] virtual bool timed_out() = 0;

  };
}

BOOST_AUTO_TEST_SUITE_END()
