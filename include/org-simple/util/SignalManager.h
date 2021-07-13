#ifndef ORG_SIMPLE_SIGNALMANAGER_H
#define ORG_SIMPLE_SIGNALMANAGER_H
/*
 * org-simple/SignalManager.h
 *
 * Added by michel on 2021-07-12
 * Copyright (C) 2015-2021 Michel Fleur.
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

#include <atomic>
#include <chrono>
#include <org-simple/util/Signal.h>

namespace org::simple::util {

enum class SignalResult { FAIL = 0, SUCCESS = 1, NOT_ALLOWED = 2 };

template<typename V>
class AbstractSignalManager {
public:
  typedef AbstractSignal<V> Signal;
  typedef typename Signal::wrap_type wrap_type;
  typedef typename Signal::external_type value_type;

  static constexpr int DEFAULT_LOCK_FREE_RETRIES = 1000;
  static_assert(std::atomic<wrap_type>::is_always_lock_free);
  std::atomic<wrap_type> wrapped_signal_ = 0;

public:

  SignalResult reset(int attempts = 0) {
    return signal({}, attempts);
  }
  SignalResult system(int value, int attempts = 0) {
    return signal(Signal::system(value), attempts);
  }
  SignalResult program(int value, int attempts = 0) {
    return signal(Signal::program(value), attempts);
  }
  SignalResult user(int value, int attempts = 0) {
    return signal(Signal::user(value), attempts);
  }

  SignalResult signal(const Signal &signal, int attempts)  {
    int maxAttempts = attempts == 0 ? DEFAULT_LOCK_FREE_RETRIES : attempts;
    wrap_type newValue = signal.wrapped();
    wrap_type wrapped = wrapped_signal_;
    int attempt = 0;
    while (maxAttempts < 0 || attempt < maxAttempts) {
      Signal unwrapped = Signal::unwrap(wrapped);
      if (unwrapped.terminates()) {
        return SignalResult::NOT_ALLOWED;
      }
      if (wrapped_signal_.compare_exchange_weak(wrapped, newValue,
                                                std::memory_order_acq_rel)) {
        return SignalResult::SUCCESS;
      }
      attempt++;
    }
    return SignalResult::FAIL;
  }

  [[nodiscard]] bool has_signal_value() const {
    return get_signal().is_valued();
  }

  [[nodiscard]] wrap_type get_signal_value() const {
    const Signal &sig = get_signal();
    return sig.is_valued() ? sig.value() : 0;
  }

  Signal get_signal() const { return Signal::unwrap(wrapped_signal_); }

  template <class Clock, class Duration>
  bool
  wait_until(Signal &result,
             const std::chrono::time_point<Clock, Duration> &timeout_time) {
    auto func = [timeout_time, this](wrap_type wrapped) {
      while (Clock::now() < timeout_time) {
        wrap_type current = wrapped_signal_;
        if (changed_and_set(wrapped, current)) {
          return current;
        }
      }
      return (wrap_type)0;
    };
    return abstract_wait(result, func);
  }

  bool wait_retries(Signal &result, int retries) {
    auto func = [retries, this](wrap_type wrapped) {
      for (int i = 0; i < retries; i++) {
        wrap_type current = wrapped_signal_;
        if (changed_and_set(wrapped, current)) {
          return current;
        }
      }
      return (wrap_type)0;
    };
    return abstract_wait(result, func);
  }

  static AbstractSignalManager &instance;

  static AbstractSignalManager &get_default_instance() {
    static AbstractSignalManager manager_;
    return manager_;
  }

  AbstractSignalManager(){}

private:

  static bool changed_and_set(wrap_type old_value, wrap_type current_value) {
    return current_value && current_value != old_value;
  }

  bool abstract_wait(Signal &result, std::function<wrap_type (wrap_type)> wait_function) {
    wrap_type wrapped_result = result.wrapped();
    wrap_type wrapped = wrapped_signal_;
    if (changed_and_set(wrapped_result, wrapped)) {
      result = { wrapped };
      return true;
    }
    wrap_type after_wait_wrapped = wait_function(wrapped);
    if (changed_and_set(wrapped_result, after_wait_wrapped)) {
      result = { after_wait_wrapped };
      return true;
    }
    return false;
  }
};

typedef AbstractSignalManager<default_signal_value_type> SignalManager;
} // namespace org::simple::util

#endif // ORG_SIMPLE_SIGNALMANAGER_H
