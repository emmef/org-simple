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

namespace helper {

template <bool threadLocal> class FakeClockCounter;
template <> class FakeClockCounter<false> {
protected:
  static uint64_t &counter() {
    static uint64_t counter_;
    return counter_;
  }
};
template <> class FakeClockCounter<true> {
protected:
  static uint64_t &counter() {
    static thread_local uint64_t counter_;
    return counter_;
  }
};

template <bool threadLocal>
struct AbstractFakeClock : private FakeClockCounter<threadLocal> {
  typedef bool (*get_callback_function)(uint64_t now, void *);

  typedef std::chrono::nanoseconds duration;
  typedef duration::rep rep;
  typedef duration::period period;
  typedef std::chrono::time_point<AbstractFakeClock, duration> time_point;

  static_assert(AbstractFakeClock::duration::min() <
                AbstractFakeClock::duration::zero(),
                "a clock's minimum duration cannot be less than its epoch");

  static constexpr bool is_steady = false;

  static uint64_t get_count() {
    uint64_t now = FakeClockCounter<threadLocal>::counter();
    auto cb = callback();
    if (cb) {
      auto d = data();
      if (cb(now, d)) {
        callback(true, nullptr);
        data(true, nullptr);
      }
    }
    return now;
  }

  static time_point now() { return time_point() + duration(get_count()); };

  template <typename R, typename P>
  static void set_now(std::chrono::duration<R, P> ticks_since_epoch) {
    set_count(std::chrono::duration_cast<duration>(ticks_since_epoch).count());
  };

  /**
   * Sets a callback that is used when #get_count() or #now() is invoked.
   *
   * The callback gets the current tick plus the data provided in \c
   * callback_data as arguments. If the callback returns \c true, the callback
   * is immediately removed.
   *
   * @param get_callback
   * @param callback_data
   */
  static void set_callback(get_callback_function function, void *callback_data) {
    if (function) {
      callback(true, function);
      data(true, callback_data);
    }
    else {
      callback(true, nullptr);
      data(true, callback_data);
    }
  }

  static void set_now(duration ticks_since_epoch) {
    set_count(ticks_since_epoch.count());
  };

  static void set_now(time_point time) {
    set_count(time.time_since_epoch().count());
  };

  static void set_count(uint64_t new_count) { set(new_count); }

  static int64_t add_get_count(int64_t value) { return add_and_get(value); }

  // Map to C API
  static std::time_t to_time_t(const time_point &__t) {
    return std::time_t(
        duration_cast<std::chrono::seconds>(__t.time_since_epoch()).count());
  }

  static time_point from_time_t(std::time_t __t) {
    typedef std::chrono::time_point<AbstractFakeClock, std::chrono::seconds>
        __from;
    return time_point_cast<AbstractFakeClock::duration>(
        __from(std::chrono::seconds(__t)));
  }

private:
  AbstractFakeClock() {};

  static get_callback_function callback(bool set = false, get_callback_function f = nullptr) {
    static std::atomic<get_callback_function> callback_;
    if (set) {
      callback_ = f;
      return nullptr;
    }
    else {
      return callback_;
    }
  }

  static void * data(bool set = false, void *d = nullptr) {
    static std::atomic<void *> data_;
    if (set) {
      data_ = d;
      return nullptr;
    }
    else {
      return data_;
    }
  }

  static uint64_t add_and_get(uint64_t value) {
    return (FakeClockCounter<threadLocal>::counter() += value);
  }

  static void set(uint64_t value) {
    FakeClockCounter<threadLocal>::counter() = value;
  }
};

} // end of namespace helper

typedef helper::AbstractFakeClock<false> FakeClock;
typedef helper::AbstractFakeClock<true> FakeClockThreadLocal;

} // namespace org::simple::util

#endif // ORG_SIMPLE_FAKECLOCK_H
