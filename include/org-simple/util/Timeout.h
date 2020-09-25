#ifndef ORG_SIMPLE_TIMEOUT_H
#define ORG_SIMPLE_TIMEOUT_H
/*
 * org-simple/Timeout.h
 *
 * Added by michel on 2020-09-23
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

#include <algorithm>
#include <chrono>
#include <limits>
#include <thread>
#include <type_traits>

#include <org-simple/core/attributes.h>

namespace org::simple::util {

class Timeout {
public:
  virtual void start() noexcept {}
  virtual ~Timeout() = default;
  org_nodiscard virtual bool timed_out() = 0;
};

class TimeoutNever final : public Timeout {
public:
  org_nodiscard bool timed_out() override { return false; }
  static Timeout &instance() {
    static TimeoutNever inst;
    return inst;
  }
};

class TimeoutImmediately final : public Timeout {
public:
  org_nodiscard bool timed_out() override { return true; }
  static Timeout &instance() {
    static TimeoutImmediately inst;
    return inst;
  }
};

template <typename type_time_point, bool is_arithmetic>
struct TimePointValuesSpec {};

template <typename type_time_point>
struct TimePointValuesSpec<type_time_point, true> {
  static constexpr const type_time_point initial = 0;

  static constexpr const type_time_point minimum =
      std::numeric_limits<type_time_point>::min();
};

template <typename type_time_point>
struct TimePointValuesSpec<type_time_point, false> {
  static constexpr const type_time_point initial = type_time_point();

  static constexpr const type_time_point minimum = type_time_point::min();
};

template <typename type_time_point>
using TimePointValues =
    TimePointValuesSpec<type_time_point, std::is_arithmetic_v<type_time_point>>;

template <typename time_point, typename duration_type>
class TimeoutWithDeadline : public Timeout {
public:
  using type_duration = duration_type;
  using type_time_point = time_point;

  explicit TimeoutWithDeadline(type_duration timeout_duration)
      : duration_(timeout_duration) {}

  TimeoutWithDeadline(const TimeoutWithDeadline &source)
      : duration_(source.duration_) {}

  void start() noexcept override {
    started_ = now();
    deadline_ = started_ + duration_;
  }

  org_nodiscard virtual type_time_point now() const noexcept = 0;
  org_nodiscard virtual type_time_point check_get_now() { return now(); }

  org_nodiscard type_time_point started() const noexcept { return started_; }
  org_nodiscard type_time_point deadline() const noexcept { return deadline_; }
  org_nodiscard type_duration duration() const noexcept { return duration_; }

  org_nodiscard bool timed_out() override {
    return check_get_now() > deadline_;
  }

private:
  type_duration duration_;
  type_time_point started_ = TimePointValues<type_time_point>::minimum;
  type_time_point deadline_ = TimePointValues<type_time_point>::initial;
};

class TimeoutFakedClock : public TimeoutWithDeadline<long, long> {
  long now_;

public:
  using type_time_point = long;
  using type_duration = long;
  using TimeoutWithDeadline<long, long>::duration;
  using TimeoutWithDeadline<long, long>::started;
  using TimeoutWithDeadline<long, long>::deadline;
  using TimeoutWithDeadline<long, long>::now;

  TimeoutFakedClock(long timeout_duration, long initial_now)
      : TimeoutWithDeadline<long, long>(timeout_duration), now_(initial_now) {}

  explicit TimeoutFakedClock(long timeout_duration)
      : TimeoutWithDeadline<long, long>(timeout_duration), now_(deadline()) {}

  template <typename R, typename P>
  explicit TimeoutFakedClock(std::chrono::duration<R, P> timeout_duration)
      : TimeoutWithDeadline<long, long>(timeout_duration.count()),
        now_(deadline()) {}

  TimeoutFakedClock(const TimeoutFakedClock &source)
      : TimeoutFakedClock(source.duration(), source.now_) {}

  org_nodiscard type_time_point now() const noexcept override { return now_; }
  void set_now(long now) { now_ = now; }
};

template <class CLOCK = std::chrono::system_clock>
class TimeoutUsingClock : public TimeoutWithDeadline<typename CLOCK::time_point,
                                                     typename CLOCK::duration> {

public:
  using clock = CLOCK;
  using type_time_point = typename CLOCK::time_point;
  using type_duration = typename CLOCK::duration;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::duration;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::started;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::deadline;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::now;

  template <typename R, typename P>
  explicit TimeoutUsingClock(std::chrono::duration<R, P> timeout_duration)
      : TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>(
            std::chrono::duration_cast<type_duration>(timeout_duration)) {}

  TimeoutUsingClock(const TimeoutUsingClock &source)
      : TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>(source.duration()) {}

  org_nodiscard type_time_point now() const noexcept override {
    return CLOCK::now();
  }
};

template <class CLOCK = std::chrono::system_clock>
class TimeoutSlicedSleep
    : public TimeoutWithDeadline<typename CLOCK::time_point,
                                 typename CLOCK::duration> {

public:
  using clock = CLOCK;
  using type_time_point = typename CLOCK::time_point;
  using type_duration = typename CLOCK::duration;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::duration;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::started;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::deadline;
  using TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>::now;

  template <typename R, typename P>
  TimeoutSlicedSleep(std::chrono::duration<R, P> timeout_duration,
                     unsigned slices)
      : TimeoutWithDeadline<typename CLOCK::time_point,
                            typename CLOCK::duration>(
            std::chrono::duration_cast<type_duration>(timeout_duration)) {
    slice_ = std::max(type_duration(1), duration() / slices);
  }

  TimeoutSlicedSleep(const TimeoutSlicedSleep &source)
      : TimeoutSlicedSleep(source.duration(), source.slice_) {}

  org_nodiscard type_time_point now() const noexcept override {
    return CLOCK::now();
  }

  void start() noexcept override {
    TimeoutWithDeadline<typename CLOCK::time_point,
                        typename CLOCK::duration>::start();
    last_sleep_ = TimePointValues<type_time_point>::minimum;
  }

  org_nodiscard type_time_point check_get_now() override {
    type_time_point t = now();
    type_time_point dl = deadline();
    if (t > dl) {
      return t;
    }
    std::this_thread::sleep_for(std::min(slice_, dl - last_sleep_));
    last_sleep_ = now();
    return last_sleep_;
  }

private:
  type_time_point last_sleep_;
  type_duration slice_;
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_TIMEOUT_H
