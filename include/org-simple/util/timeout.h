#ifndef ORG_SIMPLE_TIMEOUT_H
#define ORG_SIMPLE_TIMEOUT_H
/*
 * org-simple/timeout.h
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
#include <org-simple/core/attributes.h>
#include <thread>

namespace org::simple {
struct timeout {

  struct base {
    virtual void start() noexcept = 0;
    org_nodiscard virtual bool in_time() = 0;
    org_nodiscard virtual bool timed_out() = 0;
  };

  static base &never() {
    class time_out_never : public base {
      void start() noexcept override {}
      org_nodiscard bool timed_out() override { return false; }
      org_nodiscard bool in_time() override { return true; }
    } static instance;
    return instance;
  }

  static base &immediate() {
    class time_out_immediate : public base {
      void start() noexcept override {}
      org_nodiscard bool timed_out() override { return true; }
      org_nodiscard bool in_time() override { return false; }
    } static instance;
    return instance;
  }

  template <class CLOCK = std::chrono::system_clock>
  class simple : public base {
    using time_point = typename CLOCK::time_point;
    using duration = typename CLOCK::duration;
    time_point deadline_;
    duration duration_;

  public:
    explicit simple(duration timeout) noexcept {
      duration_ = std::max(duration(1), timeout);
    }

    template <typename R, typename P>
    explicit simple(std::chrono::duration<R, P> timeout) noexcept
        : simple(std::chrono::duration_cast<duration>(timeout)) {}

    void start() noexcept override {
      deadline_ = std::chrono::system_clock::now() + duration_;
    }

    org_nodiscard bool timed_out() override {
      return std::chrono::system_clock::now() > deadline_;
    }

    org_nodiscard bool in_time() override {
      return std::chrono::system_clock::now() <= deadline_;
    }
  };

  template <class CLOCK = std::chrono::system_clock>
  class sliced : public base {
    using time_point = typename CLOCK::time_point;
    using duration = typename CLOCK::duration;

    time_point deadline_;
    time_point last_sleep_;
    duration slice_;
    duration duration_;

  public:
    explicit sliced(duration timeout, unsigned slices) noexcept {
      duration_ = std::max(duration(1), timeout);
      slice_ = std::max(duration(1), duration_ / slices);
    }

    template <typename R, typename P>
    explicit sliced(std::chrono::duration<R, P> dur,
                           unsigned slices) noexcept
        : sliced(std::chrono::duration_cast<duration>(dur), slices) {}

    void start() noexcept override {
      time_point now = std::chrono::system_clock::now();
      deadline_ = now + duration_;
      last_sleep_ = now;
    }

    org_nodiscard bool timed_out() override {
      return check_might_sleep() > deadline_;
    }

    org_nodiscard bool in_time() override {
      return check_might_sleep() <= deadline_;
    }

  private:
    time_point check_might_sleep() {
      time_point now = std::chrono::system_clock::now();
      if (now > deadline_) {
        return now;
      }
      std::this_thread::sleep_for(std::min(slice_, deadline_ - last_sleep_));
      last_sleep_ = std::chrono::system_clock::now();
      return last_sleep_;
    }
  };
};

} // namespace org::simple

#endif // ORG_SIMPLE_TIMEOUT_H
