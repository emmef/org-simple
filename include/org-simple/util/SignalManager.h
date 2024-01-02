#ifndef ORG_SIMPLE_UTIL_M_SIGNAL_MANAGER_H
#define ORG_SIMPLE_UTIL_M_SIGNAL_MANAGER_H
/*
 * org-simple/util/SignalManager.h
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

template <bool E> class SignalCallbackHandler {};
template <> class SignalCallbackHandler<true> {

  typedef void (*cb_type)(void *);
  std::atomic<cb_type> cb_ = nullptr;
  std::atomic<void *> d_ = nullptr;

protected:
  void callback() const {
    auto cbc = cb_.load();
    if (cbc) {
      cbc(d_.load());
    }
  }

public:
  void set_callback(void (*cb)(void *), void *d) {
    cb_ = cb;
    d_ = cb ? d : nullptr;
  }
};
template <> class SignalCallbackHandler<false> {
protected:
  void callback() const {}

public:
  void set_callback(void (*)(void *), void *) {}
};

template <typename V, bool callbacks = false>
class AbstractSignalManager : public SignalCallbackHandler<callbacks> {
  using SignalCallbackHandler<callbacks>::callback;

  typedef AbstractSignal<V> Signal;
  typedef typename Signal::wrap_type wrap_type;
  typedef typename Signal::value_type value_type;

  static constexpr int DEFAULT_LOCK_FREE_RETRIES = 1000;
  static_assert(std::atomic<wrap_type>::is_always_lock_free);
  std::atomic<wrap_type> wrapped_signal_ = 0;

public:
  SignalResult reset(int attempts = 0) { return set_signal({}, attempts); }

  SignalResult system(int value, int attempts = 0) {
    return set_signal(Signal::system(value), attempts);
  }
  SignalResult program(int value, int attempts = 0) {
    return set_signal(Signal::program(value), attempts);
  }
  SignalResult user(int value, int attempts = 0) {
    return set_signal(Signal::user(value), attempts);
  }

  SignalResult set_signal(const Signal &signal, int attempts = 0) {
    int maxAttempts = attempts == 0 ? DEFAULT_LOCK_FREE_RETRIES : attempts;
    wrap_type newValue = signal.wrapped();
    wrap_type wrapped = wrapped_signal_;
    int attempt = 0;
    while (maxAttempts < 0 || attempt < maxAttempts) {
      Signal unwrapped = Signal::unwrap(wrapped);
      if (unwrapped.terminates()) {
        return SignalResult::NOT_ALLOWED;
      }
      if (wrapped_signal_.compare_exchange_strong(wrapped, newValue,
                                                  std::memory_order_acq_rel)) {
        return SignalResult::SUCCESS;
      }
      attempt++;
    }
    return SignalResult::FAIL;
  }

  [[nodiscard]] bool has_signal_value() const { return wrapped_signal_ != 0; }

  [[nodiscard]] wrap_type get_signal_value() const {
    return get_signal().value();
  }

  Signal get_signal() const { return Signal::unwrap(wrapped_signal_); }

  template <class Clock, class Duration>
  bool busy_wait_until(
      Signal &result,
      const std::chrono::time_point<Clock, Duration> &timeout_time) const {
    wrap_type current = wrapped_signal_;
    callback();
    while (current == 0 && Clock::now() < timeout_time) {
      current = wrapped_signal_;
      callback();
    }
    return get_and_set_result(result, current);
  };

  template <class Rep, class Per, class Clock = std::chrono::system_clock>
  [[maybe_unused]] bool busy_wait_for(Signal &result,
                     const std::chrono::duration<Rep, Per> &duration) const {
    wrap_type current = wrapped_signal_;
    auto start = Clock::now();
    while (current == 0 && Clock::now() - start < duration) {
      callback();
      current = wrapped_signal_;
    }
    return get_and_set_result(result, current);
  };

  bool busy_wait_spin(Signal &result, int retries) const {
    wrap_type current = wrapped_signal_;
    for (int i = 0; current == 0 && i < retries; i++) {
      callback();
      current = wrapped_signal_;
    }
    return get_and_set_result(result, current);
  }

  [[maybe_unused]] static AbstractSignalManager &get_default_instance() {
    static AbstractSignalManager manager_;
    return manager_;
  }

  [[maybe_unused]] void force_reset() { wrapped_signal_ = 0; }

private:
  bool get_and_set_result(Signal &result, wrap_type wrapped) const {
    if (wrapped) {
      result = Signal(wrapped);
      return true;
    }
    return false;
  }
};

typedef AbstractSignalManager<default_signal_value_type> SignalManager;
} // namespace org::simple::util

#endif // ORG_SIMPLE_UTIL_M_SIGNAL_MANAGER_H
