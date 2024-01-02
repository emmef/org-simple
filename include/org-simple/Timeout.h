#ifndef ORG_SIMPLE_M_TIMEOUT_H
#define ORG_SIMPLE_M_TIMEOUT_H
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

namespace org::simple {

class Timeout {
public:
  virtual void start() {}
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

template <typename Clock = std::chrono::system_clock>
class TimeoutWithDeadline : public Timeout {
public:
  typedef Clock clock;
  typedef typename Clock::duration duration;
  typedef typename Clock::time_point time_point;

  explicit TimeoutWithDeadline(duration timeout)
      : timeout_(timeout), deadline_(clock::now() + timeout_) {}

  template <typename R, typename P>
  explicit TimeoutWithDeadline(std::chrono::duration<R, P> timeout)
      : TimeoutWithDeadline(std::chrono::duration_cast<duration>(timeout)) {}

  TimeoutWithDeadline(const TimeoutWithDeadline &source)
      : TimeoutWithDeadline(source.timeout()) {}

  TimeoutWithDeadline(TimeoutWithDeadline &&source)
      : TimeoutWithDeadline(source.timeout()) {}

  [[nodiscard]] bool timed_out() override {
    time_point t = clock::now();
    if (t > deadline_) {
      return true;
    }
    execute_policy(t);
    return clock::now() > deadline_;
  }

  [[nodiscard]] time_point deadline() const { return deadline_; }
  [[nodiscard]] time_point timeout() const { return timeout_; }

  virtual void execute_policy([[maybe_unused]] time_point now){};

private:
  duration timeout_;
  time_point deadline_;
};

template <class Clock = std::chrono::system_clock>
class TimeoutSlicedSleep : public TimeoutWithDeadline<Clock> {

public:
  using TimeoutWithDeadline<Clock>::timeout;
  using TimeoutWithDeadline<Clock>::deadline;

  typedef typename TimeoutWithDeadline<Clock>::clock clock;
  typedef typename TimeoutWithDeadline<Clock>::time_point time_point;
  typedef typename TimeoutWithDeadline<Clock>::duration duration;

  explicit TimeoutSlicedSleep(duration timeout, unsigned slices)
      : TimeoutWithDeadline<Clock>(timeout), last_sleep_(time_point::min()),
        slice_(std::max(duration(1), timeout / std::max(1u, slices))) {}

  template <typename R, typename P>
  explicit TimeoutSlicedSleep(std::chrono::duration<R, P> timeout,
                              unsigned slices)
      : TimeoutWithDeadline<Clock>(
            std::chrono::duration_cast<duration>(timeout), slices) {}

  TimeoutSlicedSleep(const TimeoutSlicedSleep &source)
      : TimeoutSlicedSleep(source.timeout, source.slice_) {}

  TimeoutSlicedSleep(const TimeoutSlicedSleep &&source)
      : TimeoutSlicedSleep(source.timeout, source.slice_) {}

  void execute_policy(time_point) override {
    time_point dl = deadline();
    std::this_thread::sleep_for(std::min(slice_, dl - last_sleep_));
    last_sleep_ = clock::now();
  }

  duration slice() const { return slice_; }

private:
  time_point last_sleep_;
  duration slice_;

  explicit TimeoutSlicedSleep(duration timeout, duration slice)
      : TimeoutWithDeadline<Clock>(timeout), last_sleep_(time_point::min()),
        slice_(slice) {}
};

} // namespace org::simple

#endif // ORG_SIMPLE_M_TIMEOUT_H
