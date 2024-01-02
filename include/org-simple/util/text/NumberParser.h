#ifndef ORG_SIMPLE_UTIL_TEXT_M_NUMBER_PARSER_H
#define ORG_SIMPLE_UTIL_TEXT_M_NUMBER_PARSER_H
/*
 * org-simple/util/text/NumberParser.h
 *
 * Added by michel on 2021-12-29
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

#include <cmath>
#include <limits>
#include <org-simple/util/Predicate.h>
#include <org-simple/util/text/Characters.h>
#include <type_traits>
#include <charconv>

namespace org::simple::util::text {

struct NumberParser {
  enum class Result {
    Ok,
    OutOfRange,
    UnexpectedCharacter,
    UnexpectedEndOfInput,
    InputTooLong
  };

  static constexpr const char *resultToString(Result r) {
    switch (r) {
    case Result::Ok:
      return "Ok";
    case Result::OutOfRange:
      return "OutOfRange";
    case Result::UnexpectedCharacter:
      return "UnexpectedCharacter";
    case Result::UnexpectedEndOfInput:
      return "UnexpectedEndOfInput";
    case Result::InputTooLong:
      return "InputTooLong";
    default:
      return "[unknown]";
    }
  }

  template <typename C> static constexpr bool isDigit(const C &value) {
    return value >= static_cast<C>('0') && value <= static_cast<C>('9');
  }

  template <typename C>
  static constexpr int digitValueUnchecked(const C &value) {
    return value - static_cast<C>('0');
  }

  template <typename C> static constexpr int digitValue(const C &value) {
    if (!isDigit(value)) {
      return digitValueUnchecked(value);
    }
  }

  template <typename C> static constexpr bool isHexDigit(const C &value) {
    return isDigit(value) ||
           (value >= static_cast<C>('A') && value <= static_cast<C>('F')) ||
           (value >= static_cast<C>('a') && value <= static_cast<C>('f'));
  }

  template <typename C> static constexpr int hexDigitValue(const C &value) {
    if (isDigit(value)) {
      return digitValue<false>(value);
    } else if (value >= static_cast<C>('A') && value <= static_cast<C>('F')) {
      return value - static_cast<C>('A') + 10;
    } else if (value >= static_cast<C>('a') && value <= static_cast<C>('f')) {
      return value - static_cast<C>('a') + 10;
    }
    return -1;
  }

  template <typename V>
  static constexpr bool isIntegralNumber = std::is_integral<V>() &&
                                           !std::is_same<bool, V>();

  /**
   * Adds a digit to the integral number being built in \c value, where digit
   * SHOULD be between 0 and 9 (not checked). Returns \c true if the digit was
   * added successfully, and \c false when the result would become too large for
   * the used value type.
   * @tparam V The type of value.
   * @param value The value being built.
   * @param digit The digit to add.
   * @return \c true on success, and \c false if number is out of range.
   */
  template <typename V>
  requires(isIntegralNumber<V>) //
      static bool addDigitToPositive(V &value, unsigned digit) {
    static constexpr V max = std::numeric_limits<V>::max();
    static constexpr V maxBeforeValue = max / 10;
    static constexpr V maxBeforeDiff = max - 10 * maxBeforeValue;

    if (value > maxBeforeValue ||
        (value == maxBeforeValue && digit > maxBeforeDiff)) {
      return false;
    }
    value *= 10;
    value += digit;
    return true;
  }

  /**
   * Adds a digit to the integral number being built in \c value, where digit
   * SHOULD be between 0 and 9 (not checked). Returns \c true if the digit was
   * added successfully, and \c false when the result would become too large
   * (both positive or negative, if applicable), for the used value type. In
   * case of a signed number, the \c negative flag indicates a negative number,
   * which is necessary to parse the full range for signed types whose negative
   * range is larger than their positive range.
   * @tparam V The type of value.
   * @param value The value being built.
   * @param digit The digit to add.
   * @param negative Indicates if the value is/should be negative.
   * @return \c true on success, and \c false if number is out of range.
   */
  template <typename V>
  requires(isIntegralNumber<V>) //
      static bool addDigit(V &value, unsigned digit, bool negative) {
    if constexpr (std::is_signed<V>()) {
      static constexpr V min = std::numeric_limits<V>::min();
      static constexpr V minBeforeValue = min / 10;
      static constexpr V minBeforeDiff = min - 10 * minBeforeValue;
      if (negative) {
        V neg = -static_cast<V>(digit);
        if (value < minBeforeValue ||
            (value == minBeforeValue && neg < minBeforeDiff)) {
          return false;
        }
        value *= 10;
        value += neg;
        return true;
      }
    }
    return addDigitToPositive(value, digit);
  }

  template <typename C, typename V, class S>
  requires(text::hasInputStreamSignature<S, C> &&isIntegralNumber<V>) //
      static Result readIntegralValueFromStream(S &input, V &resultValue) {
    enum class State { Initial, Reading };
    auto classifier = util::text::Classifiers::defaultInstance<C>();
    bool negative = false;
    State state = State::Initial;
    int digitsSeen = 0;
    V temp = 0;
    C c;
    while (input.get(c)) {
      if (state == State::Initial) {
        if (classifier.isWhiteSpace(c)) {
          continue;
        } else if (isDigit(c)) {
          if (!addDigit(temp, digitValueUnchecked(c), negative)) {
            return Result::OutOfRange;
          }
          digitsSeen++;
        } else if (c == '-') {
          if constexpr (!std::is_signed<V>()) {
            return Result::UnexpectedCharacter;
          } else {
            negative = true;
          }
        } else {
          return Result::UnexpectedCharacter;
        }
        state = State::Reading;
      } else if (state == State::Reading) {
        if (isDigit(c)) {
          if (!addDigit(temp, digitValueUnchecked(c), negative)) {
            return Result::OutOfRange;
          }
          digitsSeen++;
        } else if (classifier.isWhiteSpace(c)) {
          resultValue = temp;
          return Result::Ok;
        } else {
          return Result::UnexpectedCharacter;
        }
      }
    }
    if (digitsSeen > 0) {
      resultValue = temp;
      return Result::Ok;
    }
    return Result::UnexpectedEndOfInput;
  }

  template <typename V>
  static Result setResultValue(V &value, long long signed mantissa,
                               long long signed exponent, int decimals) {
    typedef long double temp;
    static constexpr temp TEN = 10;

    if (mantissa == 0) {
      value = 0;
      return Result::Ok;
    }
    bool positive;
    temp m1 = ((positive = mantissa > 0)) ? mantissa : -mantissa;

    auto effExp = exponent - (decimals > 0 ? decimals : 0);
    temp e10 = pow(TEN, effExp);
    temp result;
    if (e10 != HUGE_VALL && e10 != -HUGE_VALL) {
      result = m1 * e10;
      if (result == HUGE_VALL || result == -HUGE_VALL) {
        return Result::OutOfRange;
      }
    }
    else if (effExp > 0) {
      return Result::OutOfRange;
    }
    else  {
      int log10MantissaFloor = floorl(log10l(m1));
      temp m0 = mantissa * pow(TEN, - log10MantissaFloor);
      e10 = pow(10, effExp + log10MantissaFloor);
      if (e10 == -HUGE_VALL) {
        return Result::OutOfRange;
      }
      result = m0 * e10;
      if (result == HUGE_VALL || result == -HUGE_VALL) {
        return Result::OutOfRange;
      }
    }
    if (result > std::numeric_limits<V>::max()) {
      return Result::OutOfRange;
    }
    value = positive ? result : -result;
    return Result::Ok;
  }

  template <typename C, typename V, class S>
  requires(text::hasInputStreamSignature<S, C> &&std::is_floating_point_v<V>) //
      static Result readRealValueFromStream(S &input, V &resultValue) {
    enum class State { MantissaStart, MantissaRead, ExponenStart, ExponenRead };
    typedef long long signed buildType;
    auto classifier = util::text::Classifiers::defaultInstance<C>();
    bool negative;
    State state = State::MantissaStart;
    buildType mantissa = 0;
    buildType exponent = 0;
    int decimals = -1;
    int seenDigits = 0;
    C c;
    while (input.get(c)) {
      if (state == State::MantissaStart) {
        if (classifier.isWhiteSpace(c)) {
          continue;
        } else if (isDigit(c)) {
          if (!addDigit(mantissa, digitValueUnchecked(c), negative)) {
            return Result::OutOfRange;
          }
          seenDigits++;
        } else if (c == '-') {
          negative = true;
        } else if (c == '+') {
          negative = false;
        } else if (c == '.') {
          decimals = 0;
        } else {
          return Result::UnexpectedCharacter;
        }
        state = State::MantissaRead;
      } else if (state == State::MantissaRead) {
        if (isDigit(c)) {
          if (!addDigit(mantissa, digitValueUnchecked(c), negative)) {
            return Result::OutOfRange;
          }
          if (decimals >= 0) {
            decimals++;
          }
          seenDigits++;
        } else if (c == '.') {
          if (decimals >= 0) {
            return Result::UnexpectedCharacter;
          }
          decimals = 0;
        } else if (classifier.isWhiteSpace(c)) {
          return setResultValue(resultValue, mantissa, exponent, decimals);
        } else if (c == 'E' || c == 'e') {
          state = State::ExponenStart;
          seenDigits = 0;
          negative = false;
        } else {
          return Result::UnexpectedCharacter;
        }
      } else if (state == State::ExponenStart) {
        if (isDigit(c)) {
          if (!addDigit(exponent, digitValueUnchecked(c), negative)) {
            return Result::OutOfRange;
          }
          seenDigits++;
        } else if (c == '-') {
          negative = true;
        } else if (c == '+') {
          negative = false;
        } else if (classifier.isWhiteSpace(c)) {
          return setResultValue(resultValue, mantissa, exponent, decimals);
        } else {
          return Result::UnexpectedCharacter;
        }
        state = State::ExponenRead;
      } else if (state == State::ExponenRead) {
        if (isDigit(c)) {
          if (!addDigit(exponent, digitValueUnchecked(c), negative)) {
            return Result::OutOfRange;
          }
          state = State::ExponenRead;
          seenDigits++;
        } else if (classifier.isWhiteSpace(c)) {
          return setResultValue(resultValue, mantissa, exponent, decimals);
        } else {
          return Result::UnexpectedCharacter;
        }
      }
    }
    if (seenDigits > 0) {
      return setResultValue(resultValue, mantissa, exponent, decimals);
    }
    return Result::UnexpectedEndOfInput;
  }
};
} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_NUMBER_PARSER_H
