#ifndef ORG_SIMPLE_CHARACTERS_H
#define ORG_SIMPLE_CHARACTERS_H
/*
 * org-simple/Characters.h
 *
 * Added by michel on 2021-12-04
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
#include <locale>
#include <org-simple/util/CharEncode.h>

namespace org::simple::charClass {

struct Ascii {
  bool isWhiteSpace(char c) const { return std::isspace(c); }
  bool isLineBreak(char c) const {
    return (c == '\n' || c == '\r' || c == '\f' || '\v');
  }
  bool isBlank(char c) const { return std::isblank(c); }
  bool isDigit(char c) const { return std::isdigit(c); }
  bool isHexDigit(char c) const { return std::isxdigit(c); }
  bool isLower(char c) const { return std::islower(c); }
  bool isUpper(char c) const { return std::isupper(c); }
  bool isAlpha(char c) const { return std::isalpha(c); }
  bool isAlphaNumeric(char c) const { return std::isalnum(c); }
  bool isPunctuation(char c) const { return std::ispunct(c); }
  bool isGraph(char c) const { return std::isgraph(c); }
  bool isControl(char c) const { return std::iscntrl(c); }
  /**
   * If the character represents an opening quote, the corresponding closing
   * quote is returned, otherwise it returns zero.
   * @param c The character to test.
   * @return the corresponding quote or zero if there is none.
   */
  char getMatchingQuote(char c) const {
    if (c == '"' || c == '\'' || c == '`') {
      return c;
    }
    return 0;
  }
  bool isQuote(char c) const { return getMatchingQuote(c) != 0; }
};

struct Unicode {
  const std::locale locale = std::locale("en_US.UTF8");
  const Ascii ascii;
  
  template <typename codePoint> bool isWhiteSpace(codePoint c) const {
    return std::isspace(c, locale);
  }
  template <typename codePoint> bool isLineBreak(codePoint c) const {
    return ascii.isLineBreak(c, locale) || c == codePoint(0x0085) || c == codePoint(0x2028) || c == codePoint(0x2029);
  }
  template <typename codePoint> bool isBlank(codePoint c) const {
    return std::isblank(c, locale);
  }
  template <typename codePoint> bool isDigit(codePoint c) const {
    return std::isdigit(c, locale);
  }
  template <typename codePoint> bool isHexDigit(codePoint c) const {
    return std::isxdigit(c, locale);
  }
  template <typename codePoint> bool isLower(codePoint c) const {
    return std::islower(c, locale);
  }
  template <typename codePoint> bool isUpper(codePoint c) const {
    return std::isupper(c, locale);
  }
  template <typename codePoint> bool isAlpha(codePoint c) const {
    return std::isalpha(c, locale);
  }
  template <typename codePoint> bool isAlphaNumeric(codePoint c) const {
    return std::isalnum(c, locale);
  }
  template <typename codePoint> bool isPunctuation(codePoint c) const {
    return std::ispunct(c, locale);
  }
  template <typename codePoint> bool isGraph(codePoint c) const {
    return std::isgraph(c, locale);
  }
  template <typename codePoint> bool isControl(codePoint c) const {
    return std::iscntrl(c, locale);
  }
  /**
   * If the character represents an opening quote, the corresponding closing
   * quote is returned, otherwise it returns zero.
   * @param c The code-point to test.
   * @return the corresponding quote or zero if there is none.
   */
  template <typename codePoint> char getMatchingQuote(codePoint c) const {
    codePoint r = ascii.getMatchingQuote(c);
    if (r != 0) {
      return r;
    }
    if (r < 127) {
      return 0;
    }
    if (c == 0x002019 || c == 0x00201D || c == 0x0000BB) {
      return c;
    }
    return codePoint(lookUpMatch(c));
  }
  template <typename codePoint> int lookUpMatch(codePoint c) const {
    switch (c) {
    case 0x00201A:
      return 0x002018;
    case 0x00201E:
      return 0x00201C;
    case 0x0000AB:
      return 0x0000BB;
    case 0x002039:
      return 0x00203A;
    case 0x00201E:
      return 0x00201D;
    case 0x0000BB:
      return 0x0000AB;
    case 0x00300C:
      return 0x00300D;
    case 0x00FE41:
      return 0x00FE42;
    case 0x00300E:
      return 0x00300F;
    case 0x00FE43:
      return 0x00FE44;
    case 0x00201C:
      return 0x00201D;
    case 0x002018:
      return 0x002019;
    case 0x00300A:
      return 0x00300B;
    case 0x003008:
      return 0x003009;
    }
    // There is ambiguity, so the single polish quote, U+201A does not return
    // 0x2019 but 0x2018.
    return 0;
  }
  template <typename codePoint> bool isQuote(codePoint c) const {
    return getMatchingQuote(c) != 0;
  }

  bool isWhiteSpace(char c) const { return ascii.isWhiteSpace(c); }
  bool isLineBreak(char c) const { return ascii.isLineBreak(c); }
  bool isBlank(char c) const { return ascii.isBlank(c); }
  bool isDigit(char c) const { return ascii.isDigit(c); }
  bool isHexDigit(char c) const { return ascii.isHexDigit(c); }
  bool isLower(char c) const { return ascii.isLower(c); }
  bool isUpper(char c) const { return ascii.isUpper(c); }
  bool isAlpha(char c) const { return ascii.isAlpha(c); }
  bool isAlphaNumeric(char c) const { return ascii.isAlphaNumeric(c); }
  bool isPunctuation(char c) const { return ascii.isPunctuation(c); }
  bool isGraph(char c) const { return ascii.isGraph(c); }
  bool isControl(char c) const { return ascii.isControl(c); }
  char getMatchingQuote(char c) const;
  bool isQuote(char c) const { return ascii.isQuote(c); }
};
struct Classifiers {
  static const Ascii &ascii() {
    static const Ascii ascii;
    return ascii;
  }
  static const Unicode &unicode() {
    static const Unicode uc{};
    return uc;
  }
};


template <typename T> struct QuoteMatcher {
  typedef bool (*function)(T cp, T &endQuote);
};

struct QuoteMatchers {

  template <typename T, T q> struct FixedSingleSymmetricMatcher {
    static constexpr bool match(T cp, T &endQuote) {
      if (cp == q) {
        endQuote = cp;
        return true;
      }
      return false;
    }
  };

  template <typename T, T q1, T q2> struct FixedDoubleSymmetricMatcher {
    static constexpr bool match(T cp, T &endQuote) {
      if (cp == q1 || cp == q2) {
        endQuote = cp;
        return true;
      }
      return false;
    }
  };

  template <typename T, T q1, T q2, T q3> struct FixedTripleSymmetricMatcher {
    static constexpr bool match(T cp, T &endQuote) {
      if (cp == q1 || cp == q2 || cp == q3) {
        endQuote = cp;
        return true;
      }
      return false;
    }
  };

  template <typename T, T qOpen, T qClose> struct FixedPairedMatcher {
    static constexpr bool match(T cp, T &endQuote) {
      if (cp == qOpen) {
        endQuote = qClose;
        return true;
      }
      return false;
    }
  };

  template <typename T, T qOpen, T qClose, T single>
  struct FixedPairedPlusSingleSymmetricMatcher {
    static constexpr bool match(T cp, T &endQuote) {
      if (cp == qOpen) {
        endQuote = qClose;
        return true;
      } else if (cp == single) {
        endQuote = single;
        return true;
      }
      return false;
    }
  };

  template <typename T>
  static constexpr typename QuoteMatcher<T>::function defaultMatch =
      FixedDoubleSymmetricMatcher<T, '\'', '"'>::match;

  template <typename T>
  static constexpr typename QuoteMatcher<T>::function singleQuotes =
      FixedSingleSymmetricMatcher<T, '\''>::match;

  template <typename T>
  static constexpr typename QuoteMatcher<T>::function doubleQuotes =
      FixedSingleSymmetricMatcher<T, '"'>::match;

  template <typename T> static bool uniCodeMatch(T cp, T &endQuote) {
    T match = charClass::Classifiers::unicode().getMatchingQuote<T>(cp);
    if (match != 0) {
      endQuote = match;
      return true;
    }
    return false;
  }

  template <typename T>
  static typename QuoteMatcher<T>::function
  getDefaultMatcherFor(const T *string,
                       typename QuoteMatcher<T>::function fallback) {
    static constexpr const T ALLOWED_QUOTE[] = "'\"`";
    static constexpr int MAX_QUOTES = sizeof(ALLOWED_QUOTE) / sizeof(T);

    bool present[MAX_QUOTES];
    // Zero quote presence
    for (int i = 0; i < MAX_QUOTES; i++) {
      present[i] = false;
    }
    bool invalid = false;
    // Mark quote presence
    for (int input = 0; string[input] != '\0'; input++) {
      T q = string[input];
      bool found = false;
      for (int i = 0; ALLOWED_QUOTE[i] != '\0'; i++) {
        if (q == ALLOWED_QUOTE[i]) {
          present[i] = true;
          found = true;
        }
      }
      if (!found) {
        invalid = true;
        break;
      }
    }

    if (!invalid) {
      // Count and condense in fixed order
      char quoteChars[MAX_QUOTES];
      int quoteCount = 0;
      for (int i = 0; i < MAX_QUOTES; i++) {
        if (present[i]) {
          quoteChars[quoteCount++] = ALLOWED_QUOTE[i];
        }
      }
      // Create result
      switch (quoteCount) {
      case 1:
        switch (quoteChars[0]) {
        case '\'':
          return FixedSingleSymmetricMatcher<T, '\''>::match;
        case '"':
          return FixedSingleSymmetricMatcher<T, '"'>::match;
        case '`':
          return FixedSingleSymmetricMatcher<T, '`'>::match;
        default:
          break;
        }
      case 2:
        switch (quoteChars[0]) {
        case '\'':
          switch (quoteChars[1]) {
          case '"':
            return FixedDoubleSymmetricMatcher<T, '\'', '"'>::match;
          case '`':
            return FixedDoubleSymmetricMatcher<T, '\'', '`'>::match;
          default:
            break;
          }
        case '"':
          return FixedDoubleSymmetricMatcher<T, '"', '`'>::match;
        default:
          break;
        }
      case 3:
        return FixedTripleSymmetricMatcher<T, '\'', '"', '`'>::match;
      default:
        break;
      }
    }
    if (fallback != nullptr) {
      return fallback;
    }
    throw std::invalid_argument("No matcher for string.");
  }
};


} // namespace org::simple::charClass

#endif // ORG_SIMPLE_CHARACTERS_H
