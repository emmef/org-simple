//
// Created by michel on 24-09-20.
//

#include "test-helper.h"
#include <org-simple/util/Timeout.h>

using namespace org::simple::util;

BOOST_AUTO_TEST_SUITE(org_simple_util_Timeout)


BOOST_AUTO_TEST_CASE(testTimeoutFakeClockIntegerArgument) {
  TimeoutFakedClock timeout10(10);
  TimeoutFakedClock timeout13(13);

  BOOST_CHECK_MESSAGE(timeout10.duration() == 10,
                      "Duration as given at construction");

  BOOST_CHECK_MESSAGE(timeout13.duration() == 13,
                      "Duration as given at construction");
}

BOOST_AUTO_TEST_CASE(testTimeoutFakeClockDurationArgument) {
  TimeoutFakedClock timeout10milli(std::chrono::milliseconds(10));
  TimeoutFakedClock timeout10micro(std::chrono::microseconds(10));
  TimeoutFakedClock timeout13micro(std::chrono::microseconds(13));

  BOOST_CHECK_MESSAGE(timeout10milli.duration() == 10,
                      "Duration as given at construction");
  BOOST_CHECK_MESSAGE(timeout10micro.duration() == 10,
                      "Duration as given at construction");

  BOOST_CHECK_MESSAGE(timeout13micro.duration() == 13,
                      "Duration as given at construction");
}

BOOST_AUTO_TEST_CASE(testTimeoutTestMechanism) {
  using tuc = TimeoutFakedClock;
  tuc timeout(std::chrono::milliseconds(10));

  BOOST_CHECK_MESSAGE(timeout.duration() == 10,
                      "Duration as given at construction");

  tuc::type_time_point start = timeout.now();
  tuc::type_time_point deadline = start + timeout.duration();

  timeout.start();
  bool timed_out = false;
  while (!timeout.timed_out()) {
    if (timeout.now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
    }
    timeout.set_now(timeout.now() + 1);
  }
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(testTimeoutUsingClock) {
  using tuc = TimeoutUsingClock<std::chrono::steady_clock>;
  tuc timeout(std::chrono::milliseconds(10));

  tuc::type_time_point start = tuc::clock::now();
  tuc::type_time_point deadline = start + timeout.duration();

  timeout.start();
  bool timed_out = false;
  while (!timeout.timed_out()) {
    if (tuc::clock::now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
    }
  }
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleep) {
  using tuc = TimeoutSlicedSleep<std::chrono::steady_clock>;
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
    }
    slices++;
  }
  if (slices > 11) {
    std::cout << "Used " << slices << " slices." << std::endl;
    BOOST_FAIL("Too many slices used");
  }
}

BOOST_AUTO_TEST_SUITE_END()
