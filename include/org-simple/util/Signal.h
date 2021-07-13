#ifndef ORG_SIMPLE_SIGNAL_H
#define ORG_SIMPLE_SIGNAL_H
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

#include <org-simple/core/Bits.h>

namespace org::simple::util {

enum class SignalType {
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
  PROGRAM
};

struct Signal {

  static const char *type_name(SignalType type) {
    switch (type) {
    case SignalType::NONE:
      return "none";
    case SignalType::SYSTEM:
      return "system";
    case SignalType::PROGRAM:
      return "program";
    case SignalType::USER:
      return "user";
    default:
      return "none";
    }
  }

  static Signal system(int value) { return {SignalType::SYSTEM, value}; }

  static Signal program(int value) { return {SignalType::PROGRAM, value}; }

  static Signal user(int value) { return {SignalType::USER, value}; }

  static Signal none() { return {SignalType::NONE, 0}; }

  static Signal unwrap(unsigned wrapped) {
    Signal result(wrapped);
    return result;
  }

  unsigned wrapped() const { return Validation::wrapped(*this); }
  /**
   * Returns the maximum signal value, that for compatibility reasons is 255.
   * @return The maximum signal value.
   */
  static constexpr int max_value() { return Validation::MAX_VALUE; }

  SignalType type() const { return type_; }

  const char *type_name() const { return type_name(type_); }

  int value() const { return value_; }

  bool is_terminator() const {
    return type_ == SignalType::SYSTEM || type_ == SignalType::PROGRAM;
  }

  bool is_none() const { return type_ == SignalType::NONE; }

  bool has_value() const { return is_terminator() || type_ == SignalType::USER; }

  bool operator==(const Signal &other) const {
    return type_ == other.type_ && value_ == other.value_;
  }

  static bool test_invalid_wrapped_type(unsigned &invalid_wrapped_state) {
    return Validation::set_invalid_wrapped_type(invalid_wrapped_state);
  }

  Signal() : type_(SignalType::NONE), value_(0) {}

private:
  SignalType type_;
  unsigned value_;

  Signal(SignalType type, int value)
      : type_(type), value_(Validation::valid_value(type_, value, false)) {}
  Signal(unsigned wrapped)
      : type_(Validation::unwrapped_type(wrapped)),
        value_(Validation::unwrapped_value(type_, wrapped)) {}

  struct Validation {
    using Bits = org::simple::core::Bits<unsigned>;
    static constexpr unsigned TYPE_COUNT = 5;
    static constexpr SignalType TYPES[TYPE_COUNT] = {
        SignalType::NONE, SignalType::USER, SignalType::SYSTEM,
        SignalType::PROGRAM};

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
    static_assert(TYPE_BITS <= 8);
    static constexpr unsigned VALUE_BITS = 8;
    static constexpr unsigned VALUE_MASK = (1u << VALUE_BITS) - 1u;
    static constexpr int MAX_VALUE = VALUE_MASK;

    static unsigned char int_to_value(int value, bool unwrapping) {
      if (value <= 0) {
        throw std::invalid_argument(
            unwrapping ? "org::simple::util::Signal::Validation:: signal value "
                         "cannot be zero."
                       : "org::simple::util::Signal::Validation::unwrap: "
                         "signal value cannot be zero.");
      }
      if (value <= MAX_VALUE) {
        return value;
      }
      throw std::invalid_argument(unwrapping
                                      ? "org::simple::util::Signal::Validation:"
                                        " (unwrapping) signal value too large."
                                      : "org::simple::util::Signal::Validation:"
                                        " signal value too large.");
    }

    static unsigned char valid_value(SignalType type, int value, bool unwrapping) {
      switch (type) {
      case SignalType::SYSTEM:
      case SignalType::USER:
      case SignalType::PROGRAM:
        return int_to_value(value, unwrapping);
      default:
        return 0;
      }
    }

    static unsigned wrapped(const Signal &signal) {
      unsigned result = static_cast<unsigned>(signal.type()) << VALUE_BITS;

      switch (signal.type()) {
      case SignalType::SYSTEM:
      case SignalType::USER:
      case SignalType::PROGRAM:
        result |= VALUE_MASK & static_cast<unsigned>(signal.value());
        return result;
      default:
        return result;
      }
    }

    static SignalType unwrapped_type(unsigned wrapped) {
      unsigned type_value = wrapped >> VALUE_BITS;

      for (size_t i = 0; i < TYPE_COUNT; i++) {
        if (static_cast<unsigned>(TYPES[i]) == type_value) {
          return TYPES[i];
        }
      }
      throw std::invalid_argument("org::simple::util::Signal::unwrap(): value "
                                  "does not represent a valid type.");
    }

    static unsigned char unwrapped_value(SignalType type, unsigned wrapped) {
      switch (type) {
      case SignalType::SYSTEM:
      case SignalType::USER:
      case SignalType::PROGRAM:
        return valid_value(type, wrapped & VALUE_MASK, true);
      default:
        return 0;
      }
    }

    template <int selector>
    static long long signed get_invalid_wrapped_type_helper();

    template <> long long signed get_invalid_wrapped_type_helper<1>() {
      return MAX_TYPE_VALUE + 1;
    }

    template <> long long signed get_invalid_wrapped_type_helper<2>() {
      return -1;
    }

    template <> long long signed get_invalid_wrapped_type_helper<3>() {
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
      static unsigned invalid_wrapped = static_cast<unsigned>(invalid_value)
                                        << VALUE_BITS;
      if (invalid_value < 0) {
        return false;
      }
      invalid_wrapped_state = invalid_wrapped;
      return true;
    }
  };
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_SIGNAL_H
