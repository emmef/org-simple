#ifndef ORG_SIMPLE_UTIL_CONFIG_M_INTEGRAL_VALUE_READER_H
#define ORG_SIMPLE_UTIL_CONFIG_M_INTEGRAL_VALUE_READER_H
/*
 * org-simple/util/config/IntegralValueReader.h
 *
 * Added by michel on 2021-12-28
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

#include <org-simple/util/config/ConfigReaders.h>

namespace org::simple::util::config {

struct IntegralValueReadingBasics {

  template <typename V>
  static bool setWithinMax(V &temp, V diff, bool negative) {
    static constexpr V max = std::numeric_limits<V>::max();
    static constexpr V maxBeforeValue = max / 10;
    static constexpr V maxBeforeDiff = max - 10 * maxBeforeValue;

    if constexpr (std::is_signed<V>()) {
      static constexpr V min = std::numeric_limits<V>::min();
      static constexpr V minBeforeValue = min / 10;
      static constexpr V minBeforeDiff = min - 10 * minBeforeValue;
      if (negative) {
        V neg = -diff;
        if (temp < minBeforeValue ||
            (temp == minBeforeValue && neg < minBeforeDiff)) {
          return false;
        }
        temp *= 10;
        temp += neg;
        return true;
      }
    }
    if (temp > maxBeforeValue ||
        (temp == maxBeforeValue && diff > maxBeforeDiff)) {
      return false;
    }
    temp *= 10;
    temp += diff;
    return true;
  }

  template <typename C, typename V>
  static ReaderResult
  readIntegralValueFromStream(text::InputStream<C> &input, V &resultValue) {
    enum class State { Initial, Reading };
    auto classifier = util::text::Classifiers::defaultInstance<C>();
    auto numberClassifier = util::text::Classifiers::ascii();
    bool negative = false;
    State state = State::Initial;
    V temp = 0;
    C c;
    while (input.get(c)) {
      switch (state) {
      case State::Initial:
        if (classifier.isWhiteSpace(c)) {
          break;
        } else if (numberClassifier.isDigit(c)) {
          V diff = c - '0';
          if (setWithinMax(temp, diff, negative)) {
            state = State::Reading;
          } else {
            return ValueReader<C>::invalid("Number too large");
          }
          break;
        } else if (c == '-') {
          if constexpr (std::is_signed<V>()) {
            if (negative) {
              return ValueReader<C>::invalid("Unexpected extra minus sign");
            }
            negative = true;
          } else {
            return ValueReader<C>::invalid(
                "Unexpected minus sign (for unsigned number)");
          }
          break;
        } else {
          return ValueReader<C>::invalid("Unexpected character");
        }
      case State::Reading:
        if (numberClassifier.isDigit(c)) {
          V diff = c - '0';
          if (!setWithinMax(temp, diff, negative)) {
            return ValueReader<C>::invalid("Number too large");
          }
          break;
        } else if (numberClassifier.isWhiteSpace(c)) {
          resultValue = temp;
          return ReaderResult::Ok;
        } else {
          return ValueReader<C>::invalid("Unexpected character");
        }
      }
    }
    if (state == State::Reading) {
      resultValue = temp;
      return ReaderResult::Ok;
    }
    return ValueReader<C>::invalid("No data or unexpected end of input");
  }
};

template <typename C, typename V>
class IntegralNumberReader : public SingleValueReader<C, V> {
  static_assert(std::is_integral<V>() && !std::is_same<bool, V>());
  static constexpr bool isSigned = std::is_signed<V>();

  using ValueReader<C>::invalid;

protected:
  virtual ReaderResult readValue(text::InputStream<C> &input, V &resultValue) {
    return IntegralValueReadingBasics::readIntegralValueFromStream(
        input, resultValue);
  }

public:
  template <typename... A>
  IntegralNumberReader(A... arguments)
      : SingleValueReader<C, V>(arguments...) {}
};

} // namespace org::simple::util::config

#endif // ORG_SIMPLE_UTIL_CONFIG_M_INTEGRAL_VALUE_READER_H
