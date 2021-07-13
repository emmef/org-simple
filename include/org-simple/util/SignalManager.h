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
#include <condition_variable>
#include <mutex>
#include <org-simple/util/Signal.h>

namespace org::simple::util {

enum class SignalResult { FAIL = 0, SUCCESS = 1, NOT_ALLOWED = 2 };

struct AbstractSignalManager {

  SignalResult reset(int attempts = 0) {
    return signal(Signal::none(), attempts);
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

  [[nodiscard]] bool has_signal_value() const {
    return get_signal().has_value();
  }

  [[nodiscard]] unsigned get_signal_value() const {
    const Signal &sig = get_signal();
    return sig.has_value() ? sig.value() : 0;
  }

  virtual ~AbstractSignalManager() = default;
  virtual SignalResult signal(const Signal &signal, int attempts = 0) = 0;
  virtual Signal get_signal() const = 0;
};

class ThrowingSignalManager : public AbstractSignalManager {
  AbstractSignalManager *delegate_;

public:
  ThrowingSignalManager(AbstractSignalManager *delegate)
      : delegate_(delegate) {}

  SignalResult signal(const Signal &signal, int attempts = 0) final {
    SignalResult result = delegate_->signal(signal, attempts);
    if (result == SignalResult::NOT_ALLOWED) {
      throw std::runtime_error(
          "org::simple::util::ThrowingAbstractSignalManager: it is not allowed "
          "to overwrite the current set signal.");
    }
    return result;
  }

  Signal get_signal() const final { return delegate_->get_signal(); }
};

class SignalManager : public AbstractSignalManager {
  static constexpr int DEFAULT_LOCK_FREE_RETRIES = 1000;
  using Mutex = std::mutex;
  using Lock = std::unique_lock<Mutex>;
  mutable Mutex mutex_;
  mutable std::condition_variable cond_;
  static_assert(std::atomic<unsigned>::is_always_lock_free);
  std::atomic<unsigned> wrapped_signal_ = 0;

public:
  class LockFree;

  static SignalManager &instance;

  static SignalManager &get_default_instance() {
    static SignalManager manager_;
    return manager_;
  }

  SignalManager()
      : lock_free_(*this), throwing_(this), throwing_lock_free_(&lock_free_){};

  LockFree &lockFree() { return lock_free_; };
  ThrowingSignalManager &throwing() { return throwing_; }
  ThrowingSignalManager &throwingLockFree() { return throwing_lock_free_; }

  Signal get_signal() const final { return Signal::unwrap(wrapped_signal_); }

  SignalResult signal(const Signal &signal, int attempts) final {
    return set_signal(signal, attempts);
  }

  Signal wait() {
    Signal result;
    Lock lock(mutex_);
    auto func = [this, &lock]() {
      this->cond_.wait(lock, [this]() { return this->has_signal_value(); });
      return true;
    };
    abstract_wait(result, func);
    return result;
  }

  template <class Clock, class Duration>
  bool
  wait_until(Signal &result,
             const std::chrono::time_point<Clock, Duration> &timeout_time) {
    Lock lock(mutex_);
    auto func = [timeout_time, this, &lock]() {
      return this->cond_.wait_until(
          lock, timeout_time, [this]() { return this->has_signal_value(); });
    };
    return abstract_wait(result, func);
  }

  template <class Rep, class Period>
  bool wait_for(Signal &result,
                const std::chrono::duration<Rep, Period> &rel_time) {
    Lock lock(mutex_);
    auto function = [rel_time, this, &lock]() {
      return this->cond_.wait_for(
          lock, rel_time, [this]() { return this->has_signal_value(); });
    };
    return abstract_wait(result, function);
  }

  class LockFree : public AbstractSignalManager {
    SignalManager &manager_;

  public:
    explicit LockFree(SignalManager &manager) : manager_(manager) {}

    SignalResult signal(const Signal &signal, int attempts) final {
      return manager_.lock_free_set_signal(signal, attempts);
    }

    [[nodiscard]] Signal get_signal() const final {
      return manager_.get_signal();
    }

    template <class Clock, class Duration>
    bool
    wait_until(Signal &result,
               const std::chrono::time_point<Clock, Duration> &timeout_time) {
      auto func = [timeout_time, this]() {
        bool result = this->manager_.has_signal_value();
        while (!result && Clock::now() < timeout_time) {
          result = this->manager_.has_signal_value();
        }
        return result;
      };
      return manager_.abstract_wait(result, func);
    }

    bool wait_retries(Signal &result, int retries) {
      auto func = [retries, this]() {
        bool result = this->manager_.has_signal_value();
        for (int i = 0; !result && i < retries; i++) {
          result = this->manager_.has_signal_value();
        }
        return result;
      };
      return manager_.abstract_wait(result, func);
    }
  };

private:
  LockFree lock_free_;
  ThrowingSignalManager throwing_;
  ThrowingSignalManager throwing_lock_free_;

  SignalResult set_signal(const Signal &signal, int retries) {
    SignalResult result;
    {
      Lock lock(mutex_);
      result = try_set_signal(signal, retries, true);
    }
    if (result == SignalResult::SUCCESS && signal.has_value()) {
      cond_.notify_all();
    }
    return result;
  }

  SignalResult try_set_signal(const Signal &signal, int retries,
                              bool allow_overwrite_non_terminator) {
    int maxAttempts = retries == 0 ? DEFAULT_LOCK_FREE_RETRIES : retries;
    unsigned newValue = signal.wrapped();
    unsigned wrapped = wrapped_signal_;
    int attempt = 0;
    while (maxAttempts < 0 || attempt < maxAttempts) {
      Signal unwrapped = Signal::unwrap(wrapped);
      bool disallowed = allow_overwrite_non_terminator
                            ? unwrapped.is_terminator()
                            : unwrapped.has_value();
      if (disallowed) {
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

  SignalResult lock_free_set_signal(const Signal &signal, int retries) {
    return try_set_signal(signal, retries, false);
  }

  bool abstract_wait(Signal &result, std::function<bool()> wait_function) {
    Signal sig = Signal::unwrap(wrapped_signal_);
    if (sig.has_value()) {
      result = sig;
      return true;
    }
    if (wait_function()) {
      sig = Signal::unwrap(wrapped_signal_);
      if (sig.has_value()) {
        result = sig;
        return true;
      }
    }
    return false;
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_SIGNALMANAGER_H
