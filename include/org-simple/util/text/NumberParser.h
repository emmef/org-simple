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

#include <limits>
#include <org-simple/util/Predicate.h>
#include <type_traits>

namespace org::simple::util::text {

struct NumberParser {
  enum class Result { Ok, TooLarge, UnexpectedCharacter, UnexpectedEndOfInput };

  static constexpr const char *resultToString(Result r) {
    switch (r) {
    case Result::Ok:
      return "Ok";
    case Result::TooLarge:
      return "TooLarge";
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

  template <typename C>
  static constexpr int digitValue(const C &value) {
    if (!isDigit(value)) {
      return digitValueUnchecked(value);
    }
  }

  template <typename C> static constexpr bool isHexDigit(const C &value) {
    return isDigit(value) ||
           (value >= static_cast<C>('A') && value <= static_cast<C>('F')) ||
           (value >= static_cast<C>('a') && value <= static_cast<C>('f'));
  }

  template <typename C>
  static constexpr int hexDigitValue(const C &value) {
    if (isDigit(value)) {
      return digitValue<false>(value);
    }
    else if (value >= static_cast<C>('A') && value <= static_cast<C>('F')) {
      return value - static_cast<C>('A') + 10;
    }
    else if (value >= static_cast<C>('a') && value <= static_cast<C>('f')) {
      return value - static_cast<C>('a') + 10;
    }
    return -1;
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
  template <typename V> static bool addDigit(V &value, V digit, bool negative) {
    static constexpr V max = std::numeric_limits<V>::max();
    static constexpr V maxBeforeValue = max / 10;
    static constexpr V maxBeforeDiff = max - 10 * maxBeforeValue;

    if constexpr (std::is_signed<V>()) {
      static constexpr V min = std::numeric_limits<V>::min();
      static constexpr V minBeforeValue = min / 10;
      static constexpr V minBeforeDiff = min - 10 * minBeforeValue;
      if (negative) {
        V neg = -digit;
        if (value < minBeforeValue ||
            (value == minBeforeValue && neg < minBeforeDiff)) {
          return false;
        }
        value *= 10;
        value += neg;
        return true;
      }
    }
    if (value > maxBeforeValue ||
        (value == maxBeforeValue && digit > maxBeforeDiff)) {
      return false;
    }
    value *= 10;
    value += digit;
    return true;
  }

  template <typename C, typename V, class S>
  requires(text::hasInputStreamSignature<S, C>) // prevent ugly formatting
      static Result readIntegralValueFromStream(S &input, V &resultValue) {
    enum class State { Initial, Reading };
    auto classifier = util::text::Classifiers::defaultInstance<C>();
    bool negative = false;
    State state = State::Initial;
    V temp = 0;
    C c;
    while (input.get(c)) {
      switch (state) {
      case State::Initial:
        if (classifier.isWhiteSpace(c)) {
          break;
        } else if (isDigit(c)) {
          V digit = digitValueUnchecked(c);
          if (addDigit(temp, digit, negative)) {
            state = State::Reading;
          } else {
            return Result::TooLarge;
          }
          break;
        } else if (c == '-') {
          if constexpr (std::is_signed<V>()) {
            if (negative) {
              return Result::UnexpectedCharacter;
            }
            negative = true;
          } else {
            return Result::UnexpectedCharacter;
          }
          break;
        } else {
          return Result::UnexpectedCharacter;
        }
      case State::Reading:
        if (isDigit(c)) {
          V digit = digitValueUnchecked(c);
          if (!addDigit(temp, digit, negative)) {
            return Result::TooLarge;
          }
          break;
        } else if (classifier.isWhiteSpace(c)) {
          resultValue = temp;
          return Result::Ok;
        } else {
          return Result::UnexpectedCharacter;
        }
      }
    }
    if (state == State::Reading) {
      resultValue = temp;
      return Result::Ok;
    }
    return Result::UnexpectedEndOfInput;
  }
};
} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_NUMBER_PARSER_H
