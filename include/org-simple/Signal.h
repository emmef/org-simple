#ifndef ORG_SIMPLE_M_SIGNAL_H
#define ORG_SIMPLE_M_SIGNAL_H
/*
 * org-simple/Signal.h
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
#include <bit>
#include <limits>
#include <stdexcept>

namespace org::simple {

typedef unsigned short SignalIntegralType;

enum class SignalType : SignalIntegralType {
  /**
   * Nothing was initialised
   */
  NONE = 0,
  /**
   * The signal is user (program) defined. The program decides what the
   * set_signal means and a set_signal like this can be (re)set by the program.
   */
  USER,
  /**
   * The set_signal was set to a positive_value unwrapped_value_enforce_nonzero
   * programmatically by the process itself and should lead to termination.
   * The program cannot overwrite or reset a set_signal with this type.
   */
  PROGRAM,
  /**
   * The set_signal was set to a positive_value unwrapped_value_enforce_nonzero
   * by a set_signal handler and should lead to termination. The program cannot
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

  static constexpr SignalType maxType() {
    return std::max(std::max(SignalType::NONE, SignalType::PROGRAM),
                    std::max(SignalType::SYSTEM, SignalType::USER));
  }

  static constexpr SignalIntegralType maxValue() {
    return static_cast<SignalIntegralType >(maxType());
  }
};

namespace helper {

template <typename V, size_t S = sizeof(V)>
struct AbstractSignalTypeStructure {};
template <typename V> struct AbstractSignalTypeStructure<V, 1> {
  typedef uint32_t WrapType;
};
template <typename V> struct AbstractSignalTypeStructure<V, 2> {
  typedef uint32_t WrapType;
};
template <typename V> struct AbstractSignalTypeStructure<V, 4> {
  typedef uint64_t WrapType;
};

template <class V> struct WrappedSignalHelper {
  static_assert(std::is_unsigned_v<V> && std::is_integral_v<V>);
  using wrap_type = typename AbstractSignalTypeStructure<V>::WrapType;
  using value_type = V;
  static constexpr V maxValue = std::numeric_limits<V>::max();

  SignalType signalType = SignalType::NONE;
  bool terminates = false;
  value_type value : sizeof(V) * 8 = 0;
};

} // End of namespace helper

template <typename V> struct AbstractSignal {
  using WrappedSignal = helper::WrappedSignalHelper<V>;
  using wrap_type = typename WrappedSignal::wrap_type;
  using value_type = typename WrappedSignal::value_type;
  static_assert(sizeof(WrappedSignal) == sizeof(wrap_type));

  static constexpr value_type maxValue = WrappedSignal::maxValue;

private:
  union SignalUnion {
    WrappedSignal signal;
    wrap_type wrapped;

    SignalUnion() : signal() {}
  } signal_;

  AbstractSignal(SignalType type, value_type value, bool terminates) {
    signal_.signal.signalType = type;
    signal_.signal.value = value;
    signal_.signal.terminates = terminates;
  }

  static inline constexpr value_type nonZero(value_type value) {
    if (value) {
      return value;
    }
    throw std::invalid_argument("A signal value should not be zero.");
  }

public:
  explicit AbstractSignal(wrap_type wrapped) {
    SignalUnion u;
    u.wrapped = wrapped;
    if (u.signal.terminates > 1 || u.signal.signalType > SignalTypeStaticInfo::maxType()) {
      throw std::invalid_argument("Invalid packed value for signal");
    }
    signal_.wrapped = wrapped;
  }

  AbstractSignal() : AbstractSignal(SignalType::NONE, 0, false) {}

  static AbstractSignal system(value_type value, bool terminates = true) {
    return {SignalType::SYSTEM, nonZero(value), terminates};
  }

  static AbstractSignal program(value_type  value, bool terminates = true) {
    return {SignalType::PROGRAM, nonZero(value), terminates};
  }

  static AbstractSignal user(value_type  value, bool terminates = true) {
    return {SignalType::USER, nonZero(value), terminates};
  }

  /**
   * @return the type name of this set_signal.
   */
  const char *type_name() const {
    return SignalTypeStaticInfo::type_name(signal_.signal.signalType);
  }

  /**
   * Returns the type of this set_signal.
   * @return returns an org::simple::SignalType.
   */
  SignalType type() const { return signal_.signal.signalType; }

  /**
   * Returns the value of this set_signal. If the type is
   * org::simple::SignalType::NONE, the value is zero. Otherwise it is a
   * value between 1 and 255.
   * @return a value between 0 and 255.
   */
  value_type value() const { return signal_.signal.value; }

  bool terminates() const { return signal_.signal.terminates; }

  bool is_signal() const { return SignalTypeStaticInfo::is_signal(type()); }

  bool operator==(const AbstractSignal &other) const {
    return type() == other.type() && terminates() == other.terminates() &&
           value() == other.value();
  }

  static AbstractSignal unwrap(wrap_type wrapped) {
    AbstractSignal result(wrapped);
    return result;
  }

  wrap_type wrapped() const {
    return signal_.wrapped;
  }
};

typedef unsigned char default_signal_value_type;
typedef AbstractSignal<default_signal_value_type> Signal;

} // namespace org::simple

#endif // ORG_SIMPLE_M_SIGNAL_H
