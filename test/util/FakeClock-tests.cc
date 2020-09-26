//
// Created by michel on 26-09-20.
//

#include "boost-unit-tests.h"
#include <org-simple/util/FakeClock.h>

typedef org::simple::util::FakeClock Clock;

typedef std::chrono::system_clock system_clock;
typedef std::chrono::system_clock::duration system_duration;

using std::chrono::duration_cast;

static system_duration generate_system_duration_since_epoch() {
  return (system_clock::now() + system_duration(rand() - rand()))
      .time_since_epoch();
}

static Clock::time_point generate_fake_time_point() {
  int64_t count = rand() - rand();
  Clock::duration duration_since_epoch(count);
  return Clock::time_point(duration_since_epoch);
}

static Clock::duration generate_fake_duration_since_epoch() {
  int64_t count = rand() - rand();
  return Clock::duration(count);
}

BOOST_AUTO_TEST_SUITE(org_simple_util_FakeClock)

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingSystemDuration) {
  const std::chrono::duration system_duration_since_epoch = generate_system_duration_since_epoch();

  Clock::set_now(system_duration_since_epoch);

  BOOST_CHECK(duration_cast<Clock::duration>(system_duration_since_epoch) ==
              Clock::now().time_since_epoch());
}

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingFakeDuration) {
  const auto fake_duration_since_epoch = generate_fake_duration_since_epoch();

  Clock::set_now(fake_duration_since_epoch);

  BOOST_CHECK(duration_cast<Clock::duration>(fake_duration_since_epoch) ==
              Clock::now().time_since_epoch());
}

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingSecondsDuration) {
  auto seconds_since_epoch = std::chrono::seconds(rand() - rand());

  Clock::set_now(seconds_since_epoch);

  BOOST_CHECK(duration_cast<Clock::duration>(seconds_since_epoch) ==
              Clock::now().time_since_epoch());
}

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingCount) {
  int64_t count = rand() - rand();

  Clock::set_count(count);

  BOOST_CHECK(count == Clock::get_count());
}

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingTimepoint) {
  Clock::time_point timePoint = generate_fake_time_point();

  Clock::set_now(timePoint);

  BOOST_CHECK(timePoint == Clock::now());
}

BOOST_AUTO_TEST_SUITE_END()
