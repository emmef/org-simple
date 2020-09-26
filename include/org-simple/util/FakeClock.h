#ifndef ORG_SIMPLE_FAKECLOCK_H
#define ORG_SIMPLE_FAKECLOCK_H
/*
 * org-simple/FakeClock.h
 *
 * Added by michel on 2020-09-25
 * Copyright (C) 2015-2020 Michel Fleur.
 * Source https://github.com/emmef/org-simple
 * Email org-simple@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <chrono>
#include <cstdint>

namespace org::simple::util {
struct FakeClock {
  typedef std::chrono::nanoseconds duration;
  typedef duration::rep rep;
  typedef duration::period period;
  typedef std::chrono::time_point<FakeClock, duration> time_point;

  static_assert(FakeClock::duration::min() < FakeClock::duration::zero(),
                "a clock's minimum duration cannot be less than its epoch");

  static constexpr bool is_steady = false;

  static time_point now() noexcept { return time_point() + duration(get_count()); };

  static void set_now(duration ticks_since_epoch) noexcept {
    set_count(ticks_since_epoch.count());
  };

  template <typename R, typename P>
  static void set_now(std::chrono::duration<R,P> ticks_since_epoch) noexcept {
    set_count(std::chrono::duration_cast<duration>(ticks_since_epoch).count());
  };

  static void set_now(time_point time) noexcept {
    set_count(time.time_since_epoch().count());
  };

  static void set_count(int64_t new_count) {
    control_count(new_count, Control::SET);
  }

  static int64_t get_count() {
    return control_count(0, Control::GET);
  }

  static int64_t add_get_count(int64_t value) {
    return control_count(value, Control::ADD);
  }

  // Map to C API
  static std::time_t to_time_t(const time_point &__t) noexcept {
    return std::time_t(
        duration_cast<std::chrono::seconds>(__t.time_since_epoch()).count());
  }

  static time_point from_time_t(std::time_t __t) noexcept {
    typedef std::chrono::time_point<FakeClock, std::chrono::seconds> __from;
    return time_point_cast<FakeClock::duration>(
        __from(std::chrono::seconds(__t)));
  }

private:
  enum class Control { SET, GET, ADD };
  static int64_t control_count(int64_t value, Control ctrl) {
    static thread_local int64_t count_;
    switch(ctrl) {
    case Control::SET:
      count_ = value;
      break;
    case Control::ADD:
      count_ += value;
    case Control::GET:
      break;
    }
    return count_;
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_FAKECLOCK_H
