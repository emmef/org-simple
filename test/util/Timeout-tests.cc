//
// Created by michel on 24-09-20.
//

#include "test-helper.h"
#include <org-simple/util/FakeClock.h>
#include <org-simple/util/Timeout.h>

typedef org::simple::util::FakeClock FakeClock;
typedef typename FakeClock::duration duration;
typedef typename FakeClock::time_point time_point;
typedef org::simple::util::TimeoutWithDeadline<FakeClock> TimeoutWithDeadline;
typedef org::simple::util::TimeoutSlicedSleep<FakeClock> TimeoutSlicedSleep;

static int64_t sleep_time_with_noise(int64_t sleep_time) {
  double factor = 0.1 * sleep_time / RAND_MAX;
  return sleep_time  + factor * rand();
}

BOOST_AUTO_TEST_SUITE(org_simple_util_Timeout)

BOOST_AUTO_TEST_CASE(testTimeoutWithDeadlineDeadlineValue) {
  int64_t duration_count = abs(rand());
  duration timeout_duration = duration(duration_count);
  FakeClock::set_now(duration(rand()));
  auto start_count = FakeClock ::get_count();
  TimeoutWithDeadline timeout(timeout_duration);

  BOOST_CHECK_EQUAL(start_count + duration_count,
                    timeout.deadline().time_since_epoch().count());
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleepDeadlineValue) {
  int64_t duration_count = abs(rand());
  duration timeout_duration = duration(duration_count);
  FakeClock::set_now(duration(rand()));
  auto start_count = FakeClock ::get_count();
  TimeoutSlicedSleep timeout(timeout_duration, 3);

  BOOST_CHECK_EQUAL(start_count + duration_count,
                    timeout.deadline().time_since_epoch().count());
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleepSliceValueZeroYieldsDurationCount) {
  int64_t duration_count = 10;
  duration timeout_duration = duration(duration_count);
  TimeoutSlicedSleep timeout(timeout_duration, 0);

  BOOST_CHECK_EQUAL(duration_count, timeout.slice().count());
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleepSliceValueOneYieldsDurationCount) {
  int64_t duration_count = 10;
  duration timeout_duration = duration(duration_count);
  TimeoutSlicedSleep timeout(timeout_duration, 1);

  BOOST_CHECK_EQUAL(duration_count, timeout.slice().count());
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleepSliceValueTwoYieldsHalfDurationCount) {
  int64_t duration_count = 10;
  duration timeout_duration = duration(duration_count);
  TimeoutSlicedSleep timeout(timeout_duration, 2);

  BOOST_CHECK_EQUAL(duration_count / 2, timeout.slice().count());
}

BOOST_AUTO_TEST_CASE(testTimeoutSlicedSleepSliceValueTooBigYieldsOne) {
  int64_t duration_count = 10;
  duration timeout_duration = duration(duration_count);
  TimeoutSlicedSleep timeout(timeout_duration, duration_count * 2);

  BOOST_CHECK_EQUAL(1, timeout.slice().count());
}

BOOST_AUTO_TEST_CASE(testTimeoutWithDeadline) {
  int64_t duration_count = 10;
  duration timeout_duration = duration(duration_count);
  FakeClock::set_now(duration(rand()));
  TimeoutWithDeadline timeout(timeout_duration);
  time_point deadline = timeout.deadline();

  bool timed_out = false;
  while (!timeout.timed_out()) {
    if (FakeClock::now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
      timed_out = true;
    }
    FakeClock::add_get_count(1);
  }
  BOOST_CHECK(true);
}


BOOST_AUTO_TEST_CASE(testTimeOutSlicedSleep) {
  int64_t duration_count = 1000000;
  unsigned slices = 10;
  duration timeout_duration = duration(duration_count);
  FakeClock::set_now(duration(rand()));
  TimeoutSlicedSleep timeout(timeout_duration, slices);
  int64_t sleepCount = timeout.slice().count();
  time_point deadline = timeout.deadline();

  bool timed_out = false;
  unsigned actual_slices = 0;
  while (!timeout.timed_out()) {
    FakeClock::add_get_count(sleep_time_with_noise(sleepCount));
    if (FakeClock::now() > deadline) {
      if (timed_out) {
        BOOST_CHECK_MESSAGE(false, "Check for timed_out failed even after "
                                   "deadline was already passed");
        return;
      }
      timed_out = true;
    }
    actual_slices++;
    FakeClock::add_get_count(1);
  }
  if (actual_slices > slices + 1) {
    BOOST_FAIL("Too many slices used");
  }
  else if (actual_slices < slices - 1) {
    BOOST_FAIL("Too few slices used");
  }
}

BOOST_AUTO_TEST_CASE(testTimeOutNever) {
  org::simple::util::Timeout &to =
      org::simple::util::TimeoutNever::instance();

  BOOST_CHECK_EQUAL(false, to.timed_out());
}

BOOST_AUTO_TEST_CASE(testTimeOutImmediately) {
  org::simple::util::Timeout &to =
      org::simple::util::TimeoutImmediately::instance();

  BOOST_CHECK_EQUAL(true, to.timed_out());
}

BOOST_AUTO_TEST_SUITE_END()
