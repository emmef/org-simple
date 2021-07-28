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

static std::atomic<uint64_t> test_callback_counter;
static std::atomic<void *> data_set_by_callback;

static void *random_callback_data() {
  static constexpr size_t SIZE = 1024;
  static char *ptr = new char[SIZE];

  char *result = ptr + (rand() % SIZE);
  return static_cast<void *>(result);
}

static bool callback_reset(uint64_t now, void *data) {
  test_callback_counter = now;
  data_set_by_callback = data;
  return true;
}

static bool callback_no_reset(uint64_t now, void *data) {
  return !callback_reset(now, data);
}

static void reset_clock_and_test_data() {
  data_set_by_callback = nullptr;
  test_callback_counter = 0;
  Clock::set_callback(nullptr, nullptr);
  Clock::set_count(0);
}

BOOST_AUTO_TEST_SUITE(org_simple_util_FakeClock)

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingSystemDuration) {
  const std::chrono::duration system_duration_since_epoch =
      generate_system_duration_since_epoch();

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
  uint64_t count = rand();

  Clock::set_count(count);

  BOOST_CHECK(count == Clock::get_count());
}

BOOST_AUTO_TEST_CASE(testNowIsAsSetUsingTimepoint) {
  Clock::time_point timePoint = generate_fake_time_point();

  Clock::set_now(timePoint);

  BOOST_CHECK(timePoint == Clock::now());
}

BOOST_AUTO_TEST_CASE(testCallbackNoReset) {
  void *initial_data = random_callback_data();
  void *registered_callback_data = random_callback_data();
  reset_clock_and_test_data();

  data_set_by_callback = initial_data;
  Clock::set_callback(callback_no_reset, registered_callback_data);

  BOOST_CHECK_EQUAL(data_set_by_callback, initial_data);

  uint64_t fake_time_set = rand();
  Clock::set_count(fake_time_set);
  uint64_t now = Clock::get_count();

  BOOST_CHECK_EQUAL(now, fake_time_set);
  BOOST_CHECK_EQUAL(test_callback_counter, fake_time_set);
  BOOST_CHECK_EQUAL(data_set_by_callback, registered_callback_data);

  data_set_by_callback = initial_data;
  uint64_t fake_time_set_after_callback_no_reset = rand();
  Clock::set_count(fake_time_set_after_callback_no_reset);
  uint64_t now_after_callback_auto_reset = Clock::get_count();

  BOOST_CHECK_EQUAL(now_after_callback_auto_reset, fake_time_set_after_callback_no_reset);
  BOOST_CHECK_EQUAL(test_callback_counter, fake_time_set_after_callback_no_reset);
  BOOST_CHECK_EQUAL(data_set_by_callback, registered_callback_data);


  reset_clock_and_test_data();
}

BOOST_AUTO_TEST_CASE(testCallbackReset) {
  void *initial_data = random_callback_data();
  void *registered_callback_data = random_callback_data();

  reset_clock_and_test_data();
  data_set_by_callback = initial_data;

  Clock::set_callback(callback_reset, registered_callback_data);

  BOOST_CHECK_EQUAL(data_set_by_callback, initial_data);

  uint64_t fake_time_set = rand();
  Clock::set_count(fake_time_set);
  uint64_t now = Clock::get_count();

  BOOST_CHECK_EQUAL(now, fake_time_set);
  BOOST_CHECK_EQUAL(test_callback_counter, fake_time_set);
  BOOST_CHECK_EQUAL(data_set_by_callback, registered_callback_data);

  data_set_by_callback = initial_data;
  uint64_t fake_time_set_after_callback_auto_reset = rand();
  Clock::set_count(fake_time_set_after_callback_auto_reset);
  uint64_t now_after_callback_auto_reset = Clock::get_count();

  BOOST_CHECK_EQUAL(now_after_callback_auto_reset, fake_time_set_after_callback_auto_reset);
  BOOST_CHECK_NE(test_callback_counter, fake_time_set_after_callback_auto_reset);
  BOOST_CHECK_NE(data_set_by_callback, registered_callback_data);

  reset_clock_and_test_data();
}

BOOST_AUTO_TEST_SUITE_END()
