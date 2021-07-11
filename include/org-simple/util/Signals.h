#ifndef ORG_SIMPLE_SIGNALS_H
#define ORG_SIMPLE_SIGNALS_H
/*
 * org-simple/Signals.h
 *
 * Added by michel on 2021-07-11
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
#include <org-simple/core/Bits.h>

namespace org::simple::util {

struct Signal {
  enum class Type {
    /**
     * Nothing was initialised
     */
    NONE = 0,
    /**
     * The signal is user (program) defined. The program decides what the signal
     * means and a signal like this can be (re)set by the program.
     */
    USER,
    /**
     * The signal was set to a positive_value unwrapped_value_enforce_nonzero by
     * a signal handler and should lead to termination. The program cannot
     * overwrite or reset a signal with this type.
     */
    SYSTEM,
    /**
     * The signal was set to a positive_value unwrapped_value_enforce_nonzero
     * programmatically by the process itself and should lead to termination.
     * The program cannot overwrite or reset a signal with this type.
     */
    PROGRAM,
    /**
     * Indicates a timeout in a wait function. It is a synthetic signal that can
     * only occur in the situation of a wait.
     */
    TIMEOUT
  };

  static Signal system(unsigned value) { return {Type::SYSTEM, value}; }

  static Signal program(unsigned value) { return {Type::PROGRAM, value}; }

  static Signal user(unsigned value) { return {Type::USER, value}; }

  static Signal timeout() { return {Type::TIMEOUT, 0}; }

  static Signal none() { return {Type::NONE, 0}; }

  static Signal unwrap(unsigned wrapped) {
    Signal result(wrapped);
    return result;
  }

  unsigned wrapped() const { return Validation::wrapped(*this); }

  static constexpr unsigned max_value() { return Validation::MAX_VALUE; }

  Type type() const { return type_; }

  static const char *type_name(Type type) {
    switch (type) {
    case Type::NONE:
      return "none";
    case Type::SYSTEM:
      return "system";
    case Type::PROGRAM:
      return "program";
    case Type::USER:
      return "user";
    case Type::TIMEOUT:
      return "timeout";
    default:
      return "none";
    }
  }

  const char *type_name() const { return type_name(type_); }

  size_t value() const { return value_; }

  bool is_timeout() const { return type_ == Type::TIMEOUT; }

  bool is_terminator() const {
    return type_ == Type::SYSTEM || type_ == Type::PROGRAM;
  }

  bool has_value() const { return is_terminator() || type_ == Type::USER; }

  bool operator==(const Signal &other) const {
    return type_ == other.type_ && value_ == other.value_;
  }

  void check_no_terminator() {
    if (is_terminator()) {
      throw std::runtime_error("org::simple::util::Signal: cannot reassign as "
                               "signal is terminator.");
    }
  }

  static bool test_invalid_wrapped_type(unsigned &invalid_wrapped_state) {
    return Validation::set_invalid_wrapped_type(invalid_wrapped_state);
  }

private:
  Type type_;
  unsigned value_;

  Signal() : type_(Type::NONE), value_(0) {}
  Signal(Type type, unsigned value)
      : type_(type), value_(Validation::valid_value(type_, value, false)) {}
  Signal(unsigned wrapped)
      : type_(Validation::unwrapped_type(wrapped)),
        value_(Validation::unwrapped_value(type_, wrapped)) {}

  struct Validation {
    using Bits = org::simple::core::Bits<unsigned>;
    static constexpr unsigned TYPE_COUNT = 5;
    static constexpr Type TYPES[TYPE_COUNT] = {
        Type::NONE, Type::USER, Type::SYSTEM, Type::PROGRAM, Type::TIMEOUT};

    template <int N>
    static constexpr unsigned MAX_TYPE_ENUM_VALUE_H =
        std::max(static_cast<unsigned>(TYPES[N]), MAX_TYPE_ENUM_VALUE_H<N - 1>);
    template <> static constexpr unsigned MAX_TYPE_ENUM_VALUE_H<-1> = 0;
    template <>
    static constexpr unsigned MAX_TYPE_ENUM_VALUE_H<TYPE_COUNT> =
        MAX_TYPE_ENUM_VALUE_H<TYPE_COUNT - 1>;

    static constexpr unsigned MAX_TYPE_VALUE =
        MAX_TYPE_ENUM_VALUE_H<TYPE_COUNT>;
    static_assert(MAX_TYPE_VALUE > 1);
    static constexpr unsigned TYPE_MASK = Bits::fill(MAX_TYPE_VALUE);
    static constexpr unsigned TYPE_BITS = Bits::most_significant(TYPE_MASK) + 1;
    static_assert(8 * sizeof(unsigned) - 8 >= TYPE_BITS);
    static constexpr unsigned VALUE_BITS = 8 * sizeof(unsigned) - TYPE_BITS;
    static constexpr unsigned MAX_VALUE = (1u << VALUE_BITS) - 1u;

    static unsigned valid_value(Type type, unsigned value, bool unwrapping) {
      switch (type) {
      case Type::SYSTEM:
      case Type::USER:
      case Type::PROGRAM:
        break;
      default:
        return 0;
      }
      if (value == 0) {
        throw std::invalid_argument(
            unwrapping ? "org::simple::util::Signal::Validation:: signal value "
                         "cannot be zero."
                       : "org::simple::util::Signal::Validation::unwrap: "
                         "signal value cannot be zero.");
      }
      const unsigned result = value & MAX_VALUE;
      if (result == value) {
        return value;
      }
      throw std::invalid_argument(unwrapping
                                      ? "org::simple::util::Signal::Validation:"
                                        " (unwrapping) signal value too large."
                                      : "org::simple::util::Signal::Validation:"
                                        " signal value too large.");
    }

    static unsigned wrapped(const Signal &signal) {
      unsigned result = static_cast<unsigned>(signal.type()) << VALUE_BITS;

      switch (signal.type()) {
      case Type::SYSTEM:
      case Type::USER:
      case Type::PROGRAM:
        result |= signal.value();
        return result;
      default:
        return result;
      }
    }

    static Type unwrapped_type(unsigned wrapped) {
      unsigned type_value = wrapped >> VALUE_BITS;

      for (size_t i = 0; i < TYPE_COUNT; i++) {
        if (static_cast<unsigned>(TYPES[i]) == type_value) {
          return TYPES[i];
        }
      }
      throw std::invalid_argument("org::simple::util::Signal::unwrap(): value "
                                  "does not represent a valid type.");
    }

    static unsigned unwrapped_value(Type type, unsigned wrapped) {
      switch (type) {
      case Type::SYSTEM:
      case Type::USER:
      case Type::PROGRAM:
        break;
      default:
        return 0;
      }
      unsigned v = valid_value(type, wrapped & MAX_VALUE, true);
      if (v == 0) {
        throw std::invalid_argument(
            "org::simple::util::Signal::unwrap(): "
            "Signal unwrapped_value_enforce_nonzero cannot be zero.");
      }
      return v;
    }

    template <int selector>
    static long long signed get_invalid_wrapped_type_helper();

    template <> static long long signed get_invalid_wrapped_type_helper<1>() {
      return MAX_TYPE_VALUE + 1;
    }

    template <> static long long signed get_invalid_wrapped_type_helper<2>() {
      return -1;
    }

    template <> static long long signed get_invalid_wrapped_type_helper<3>() {
      for (unsigned type = 0; type < MAX_TYPE_VALUE; type++) {
        bool found = false;
        for (unsigned j = 0; j < TYPE_COUNT && !found; j++) {
          if (static_cast<unsigned>(TYPES[type]) == type) {
            found = true;
          }
        }
        if (!found) {
          return type;
        }
      }
      return -1;
    }

    static bool set_invalid_wrapped_type(unsigned &invalid_wrapped_state) {
      static constexpr int selector = MAX_TYPE_VALUE < TYPE_MASK    ? 1
                                      : MAX_TYPE_VALUE < TYPE_COUNT ? 2
                                                                    : 3;
      static long long signed invalid_value =
          get_invalid_wrapped_type_helper<selector>();
      static unsigned invalid_wrapped = static_cast<unsigned>(invalid_value) << VALUE_BITS;
      if (invalid_value < 0) {
        return false;
      }
      invalid_wrapped_state = invalid_wrapped;
      return true;
    }
  };
};


} // namespace org::simple::util

#endif // ORG_SIMPLE_SIGNALS_H
