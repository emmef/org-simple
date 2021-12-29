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
#include <type_traits>

namespace org::simple::util::text {

struct NumberParser {
  enum class Result { Ok,
    OutOfRange, UnexpectedCharacter, UnexpectedEndOfInput };

  static constexpr const char *resultToString(Result r) {
    switch (r) {
    case Result::Ok:
      return "Ok";
    case Result::OutOfRange:
      return "LimitsExceeded";
    case Result::UnexpectedCharacter:
      return "UnexpectedCharacter";
    case Result::UnexpectedEndOfInput:
      return "UnexpectedEndOfInput";
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
      }
      else if (state == State::Reading) {
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
  static Result setResultValue(V &result, long long signed mantissa,
                               long long signed exponent, int decimals) {
    typedef long double temp;
    static constexpr temp LOG_10_OF_2 = M_LN2 / M_LN10;

    const temp effExp2 =
        (decimals > 0 ? (exponent - decimals) : exponent) * LOG_10_OF_2;
    const int effExp2Whole = floorl(effExp2);
    const temp effExp2Fraction = effExp2 - effExp2Whole;

    const temp v = static_cast<temp>(mantissa) * pow(2.0, effExp2Fraction);
    const temp r = ldexpl(v, effExp2Whole);
    if (r == HUGE_VAL || r == HUGE_VALF || r == HUGE_VALL) {
      return Result::OutOfRange;
    }
    result = r;
    return Result::Ok;
  }

  template <typename C, typename V, class S>
  requires(text::hasInputStreamSignature<S, C> &&std::is_floating_point<V>()) //
      static Result readRealValueFromStream(S &input, V &resultValue) {
    enum class State { MantissaStart, MantissaRead, ExponenStart, ExponenRead };
    typedef long long signed buildType;
    auto classifier = util::text::Classifiers::defaultInstance<C>();
    bool negative;
    State state = State::Initial;
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
