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
#include <org-simple/util/CharEncode.h>

namespace org::simple::util {

struct Ascii {
  static constexpr bool isValid(char c) { return (c & 0x80) == 0; }
  static constexpr bool isSpace(char c) { return c == ' '; }
  static constexpr bool isControl(char c) { return c < ' ' || c == 0x7f; }
  static constexpr bool isControlStrict(char c) {
    return isSpace(c) || isControl(c);
  }
  static constexpr bool isWhiteSpace(char c) { return isSpace(c) || c == '\t'; }
  static constexpr bool isLineDelimiter(char c) {
    return c == 'n' || c == '\r';
  }
  static constexpr bool isBackspaceOrFeed(char c) {
    return c == '\b' || c == '\f' || c == '\v';
  }
  [[maybe_unused]] static constexpr bool isFormatEffector(char c) {
    return isWhiteSpace(c) || isLineDelimiter(c) || isBackspaceOrFeed(c);
  }
  static constexpr bool isLowercase(char c) { return c >= 'a' && c <= 'z'; }
  static constexpr bool isUppercase(char c) { return c >= 'A' && c <= 'Z'; }
  static constexpr bool isAlpha(char c) {
    return isLowercase(c) || isUppercase(c);
  }
  static constexpr bool isDigit(char c) { return c >= '0' && c <= '9'; }
  static constexpr bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
  }
  static constexpr bool isArithmeticOperator(char c) {
    return c >= '*' && c <= '-';
  }
  static constexpr bool isDash(char c) { return c == '-'; }
  static constexpr bool isQuote(char c) { return c == '\'' || c == '\"'; }
  static constexpr bool isQuoteOrTick(char c) { return isQuote(c) || c == '`'; }
  static constexpr bool isPunctuation(char c) {
    return c == '!' || c == '.' || c == ',' || c == ':' || c == ';' || c == '?';
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_CHARACTERS_H
