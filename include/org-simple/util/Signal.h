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

#include <algorithm>
#include <org-simple/core/Bits.h>

namespace org::simple::util {

enum class SignalType {
  /**
   * Nothing was initialised
   */
  NONE,
  /**
   * The signal is user (program) defined. The program decides what the set_signal
   * means and a set_signal like this can be (re)set by the program.
   */
  USER,
  /**
   * The set_signal was set to a positive_value unwrapped_value_enforce_nonzero
   * programmatically by the process itself and should lead to termination.
   * The program cannot overwrite or reset a set_signal with this type.
   */
  PROGRAM,
  /**
   * The set_signal was set to a positive_value unwrapped_value_enforce_nonzero by
   * a set_signal handler and should lead to termination. The program cannot
   * overwrite or reset a set_signal with this type.
   */
  SYSTEM,
};

struct SignalTypeStaticInfo {

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
      return "invalid";
    }
  }

  static constexpr bool is_signal(SignalType type) {
    return type != SignalType::NONE;
  }

  static int type_to_value(SignalType type) { return static_cast<int>(type); }

  static bool value_to_type(SignalType &result, int value) {
    switch (value) {
    case static_cast<int>(SignalType::NONE):
      result = SignalType::NONE;
      return true;
    case static_cast<int>(SignalType::USER):
      result = SignalType::USER;
      return true;
    case static_cast<int>(SignalType::PROGRAM):
      result = SignalType::PROGRAM;
      return true;
    case static_cast<int>(SignalType::SYSTEM):
      result = SignalType::SYSTEM;
      return true;
    default:
      return false;
    }
  }

  static constexpr int max_value() { return MAX_TYPE_VALUE; }

private:
  static_assert(static_cast<int>(SignalType::NONE) == 0);
  static_assert(static_cast<int>(SignalType::USER) == 1);
  static_assert(static_cast<int>(SignalType::PROGRAM) == 2);
  static_assert(static_cast<int>(SignalType::SYSTEM) == 3);

  static constexpr int MAX_TYPE_VALUE =
      std::max(std::max(static_cast<int>(SignalType::NONE),
                        static_cast<int>(SignalType::USER)),
               std::max(static_cast<int>(SignalType::PROGRAM),
                        static_cast<int>(SignalType::SYSTEM)));

  static_assert(MAX_TYPE_VALUE > 1);
};

namespace helper {
template <typename T, size_t S = sizeof(T)>
struct AbstractSignalAssignmentType {};
template <typename T> struct AbstractSignalAssignmentType<T, 1> {
  typedef int16_t ext_type;
  typedef uint16_t wrap_type;
};
template <typename T> struct AbstractSignalAssignmentType<T, 2> {
  typedef int32_t ext_type;
  typedef uint32_t wrap_type;
};
template <typename T> struct AbstractSignalAssignmentType<T, 4> {
  typedef int64_t ext_type;
  typedef uint64_t wrap_type;
};

} // End of namespace helper


template <typename V> struct AbstractSignalValue {
  using value_type = V;
  using external_type =
      typename helper::AbstractSignalAssignmentType<V>::ext_type;
  using wrap_type = typename helper::AbstractSignalAssignmentType<V>::wrap_type;
  static_assert(std::is_unsigned_v<value_type> &&
                std::is_integral_v<value_type>);
  static_assert(std::is_signed_v<external_type> &&
                std::is_integral_v<external_type> &&
                sizeof(external_type) > sizeof(value_type));

  static constexpr value_type MAX_VALUE = ~(0);

  AbstractSignalValue() : value_(0) {}

  explicit AbstractSignalValue(external_type value)
      : value_(validated_non_zero(value)) {}

  explicit AbstractSignalValue(value_type value) : value_(value) {}

  external_type get() const { return value_; }
  value_type get_value() const { return value_; }

  AbstractSignalValue &operator=(external_type value) {
    value_ = validated_non_zero(value);
    return *this;
  }

  void zero() { value_ = 0; }

  static value_type validated_non_zero(external_type value) {
    if (value > 0 && value <= static_cast<external_type>(MAX_VALUE)) {
      return static_cast<value_type>(value);
    }

    throw std::invalid_argument("org::simple::util::SignalValue: value must be "
                                "positive and not exceed maximum.");
  }

  bool operator == (const AbstractSignalValue &other) const {
    return value_ == other.value_;
  }

  bool operator == (value_type other) const {
    return value_ == other;
  }

private:
  value_type value_;
};

template <typename V> struct AbstractSignal {
  typedef typename AbstractSignalValue<V>::value_type value_type;
  typedef typename AbstractSignalValue<V>::external_type external_type;
  typedef typename AbstractSignalValue<V>::wrap_type wrap_type;
  static constexpr value_type MAX_VALUE = AbstractSignalValue<V>::MAX_VALUE;

private:
  static constexpr int BITS_VALUE = sizeof(value_type) * 8;
  static constexpr int BITS_SIGNAL_TYPE =
      org::simple::core::Bits<wrap_type>::most_significant(
          SignalTypeStaticInfo::max_value()) +
      1;
  static constexpr int BITS_SIGNAL_TYPE_AND_TERMINATE = BITS_SIGNAL_TYPE + 1;
  static constexpr int BITS_TOTAL = BITS_VALUE + BITS_SIGNAL_TYPE_AND_TERMINATE;
  static_assert(BITS_TOTAL < sizeof(wrap_type) * 8);
  static constexpr value_type MASK_VALUE = ~0;
  static constexpr wrap_type FLAG_TERMINATE = static_cast<wrap_type>(1)
                                              << sizeof(value_type) * 8;
  static constexpr short SHIFTS_SIGNAL_TYPE = sizeof(value_type) * 8 + 1;

  SignalType type_;
  bool terminates_;
  AbstractSignalValue<V> value_;

  AbstractSignal(SignalType type, external_type value, bool terminates)
      : type_(type), value_(value), terminates_(terminates) {}

  AbstractSignal(wrap_type wrapped) {
    if (!SignalTypeStaticInfo::value_to_type(type_,
                                             wrapped >> SHIFTS_SIGNAL_TYPE)) {
      throw std::invalid_argument("org::simple::util::Signal: wrapped value "
                                  "does not represent a valid type.");
    }
    if (!SignalTypeStaticInfo::is_signal(type_)) {
      terminates_ = false;
      return;
    }
    unsigned msk = MASK_VALUE;
    value_ = wrapped & msk;
    terminates_ = (wrapped & FLAG_TERMINATE) != 0;
  }

public:
  AbstractSignal() : type_(SignalType::NONE), terminates_(false) {}

  static AbstractSignal system(external_type value, bool terminates = true) {
    return {SignalType::SYSTEM, value, terminates};
  }

  static AbstractSignal program(external_type value, bool terminates = true) {
    return {SignalType::PROGRAM, value, terminates};
  }

  static AbstractSignal user(external_type value, bool terminates = true) {
    return {SignalType::USER, value, terminates};
  }

  /**
   * @return the type name of this set_signal.
   */
  const char *type_name() const {
    return SignalTypeStaticInfo::type_name(type_);
  }

  /**
   * Returns the type of this set_signal.
   * @return returns an org::simple::util::SignalType.
   */
  SignalType type() const { return type_; }

  /**
   * Returns the value of this set_signal. If the type is
   * org::simple::util::SignalType::NONE, the value is zero. Otherwise it is a
   * value between 1 and 255.
   * @return a value between 0 and 255.
   */
  external_type value() const { return value_.get(); }

  bool terminates() const { return terminates_; }

  bool is_signal() const { return SignalTypeStaticInfo::is_signal(type_); }

  bool is_valued() const { return SignalTypeStaticInfo::is_signal(type_); }

  bool operator == (const AbstractSignal &other) const {
    return type_ == other.type_ && terminates_ == other.terminates_ && value_ == other.value_;
  }

  static AbstractSignal unwrap(wrap_type wrapped) {
    AbstractSignal result(wrapped);
    return result;
  }

  wrap_type wrapped() const {
    wrap_type wrapped = SignalTypeStaticInfo::type_to_value(type_);
    wrapped <<= SHIFTS_SIGNAL_TYPE;
    if (SignalTypeStaticInfo::is_signal(type_)) {
      wrapped |= value_.get_value();
    }
    if (terminates_) {
      wrapped |= FLAG_TERMINATE;
    }
    return wrapped;
  }
};

typedef unsigned char default_signal_value_type;
typedef AbstractSignal<default_signal_value_type> Signal;

} // namespace org::simple::util

#endif // ORG_SIMPLE_SIGNAL_H
