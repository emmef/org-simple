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

namespace org::simple::util {

class Timeout {
public:
  virtual void start() noexcept {}
  virtual ~Timeout() = default;
  [[nodiscard]] virtual bool timed_out() = 0;
};

class TimeoutNever final : public Timeout {
public:
  [[nodiscard]] bool timed_out() override { return false; }
  static Timeout &instance() {
    static TimeoutNever inst;
    return inst;
  }
};

class TimeoutImmediately final : public Timeout {
public:
  [[nodiscard]] bool timed_out() override { return true; }
  static Timeout &instance() {
    static TimeoutImmediately inst;
    return inst;
  }
};

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

  [[nodiscard]] virtual type_time_point now() const noexcept = 0;
  [[nodiscard]] virtual type_time_point check_get_now() { return now(); }

  [[nodiscard]] type_time_point started() const noexcept { return started_; }
  [[nodiscard]] type_time_point deadline() const noexcept { return deadline_; }
  [[nodiscard]] type_duration duration() const noexcept { return duration_; }

  [[nodiscard]] bool timed_out() override {
    return check_get_now() > deadline_;
  }

private:
  type_duration duration_;
  type_time_point started_ = type_time_point::min();
  type_time_point deadline_ = type_time_point();
};

template <class CLOCK = std::chrono::system_clock>
class TimeoutFakedClock : public TimeoutWithDeadline<typename CLOCK::time_point,
                                                     typename CLOCK::duration> {

public:
  using type_time_point = typename CLOCK::time_point;
  using type_duration = typename CLOCK::duration;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::duration;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::started;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::deadline;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::now;

  template <typename R, typename P>
  explicit TimeoutFakedClock(std::chrono::duration<R, P> timeout_duration)
      : TimeoutWithDeadline<type_time_point, type_duration>(timeout_duration),
        now_(deadline()) {}

  TimeoutFakedClock(const TimeoutFakedClock &source)
      : TimeoutFakedClock(source.duration(), source.now_) {}

  [[nodiscard]] type_time_point now() const noexcept override { return now_; }
  void set_now(type_time_point now) { now_ = now; }

private:
  type_time_point now_;
};

template <class CLOCK = std::chrono::system_clock>
class TimeoutUsingClock : public TimeoutWithDeadline<typename CLOCK::time_point,
                                                     typename CLOCK::duration> {

public:
  using clock = CLOCK;
  using type_time_point = typename CLOCK::time_point;
  using type_duration = typename CLOCK::duration;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::duration;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::started;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::deadline;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::now;

  template <typename R, typename P>
  explicit TimeoutUsingClock(std::chrono::duration<R, P> timeout_duration)
      : TimeoutWithDeadline<type_time_point,
                            type_duration>(
            std::chrono::duration_cast<type_duration>(timeout_duration)) {}

  TimeoutUsingClock(const TimeoutUsingClock &source)
      : TimeoutWithDeadline<type_time_point,
                            type_duration>(source.duration()) {}

  [[nodiscard]] type_time_point now() const noexcept override {
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
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::duration;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::started;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::deadline;
  using TimeoutWithDeadline<type_time_point,
                            type_duration>::now;

  template <typename R, typename P>
  TimeoutSlicedSleep(std::chrono::duration<R, P> timeout_duration,
                     unsigned slices)
      : TimeoutWithDeadline<type_time_point,
                            type_duration>(
            std::chrono::duration_cast<type_duration>(timeout_duration)) {
    slice_ = std::max(type_duration(1), duration() / slices);
  }

  TimeoutSlicedSleep(const TimeoutSlicedSleep &source)
      : TimeoutSlicedSleep(source.duration(), source.slice_) {}

  [[nodiscard]] type_time_point now() const noexcept override {
    return CLOCK::now();
  }

  void start() noexcept override {
    TimeoutWithDeadline<type_time_point,
                        type_duration>::start();
    last_sleep_ = type_time_point::min();
  }

  [[nodiscard]] type_time_point check_get_now() override {
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
