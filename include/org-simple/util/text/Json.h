#ifndef ORG_SIMPLE_UTIL_TEXT_M_JSON_H
#define ORG_SIMPLE_UTIL_TEXT_M_JSON_H
/*
 * org-simple/util/text/Json.h
 *
 * Added by michel on 2022-01-25
 * Copyright (C) 2015-2022 Michel Fleur.
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

#include <org-simple/util/text/CharEncode.h>
#include <stdexcept>
#include <string>

namespace org::simple::util::text {

class JsonException : public std::exception {
  std::string message;

public:
  JsonException(const char *m) : message("JSON: ") { message += m; }

  JsonException &operator<<(const char *snippet) {
    message += snippet;
    return *this;
  }

  JsonException &operator<<(char c) {
    static constexpr const char digit[] = "0123456789abcdef";
    // Negative numbers are fine as tThe input is expected to be correct UTF-8.
    if (c >= ' ' && c < 127) {
      message += c;
    } else if (c == '\\' || c == '/' || c == '"') {
      message += '\\';
      message += c;
    } else if (c == '\b') {
      message += '\\';
      message += 'b';
    } else if (c == '\f') {
      message += '\\';
      message += 'f';
    } else if (c == '\n') {
      message += '\\';
      message += 'n';
    } else if (c == '\r') {
      message += '\\';
      message += 'r';
    } else if (c == '\t') {
      message += '\\';
      message += 't';
    } else {
      message += "\\x";
      message += digit[c / 16];
      message += digit[c % 16];
    }
    return *this;
  }

  template <typename V> requires(std::is_integral_v<V>)JsonException &operator<<(const V &v) {
    static constexpr const char digit[] = "0123456789abcdef";
    static constexpr int startShifts = sizeof(V) * 8 - 4;

    for (int shifts = startShifts; shifts >= 0; shifts -= 4) {
      message += digit[(v >> shifts) & 0x00f];
    }
    return *this;
  }

  const char *what() const noexcept override { return message.c_str(); }
};

class JsonUnexpectedEndOfInput : public JsonException {
public:
  JsonUnexpectedEndOfInput(const char *m)
      : JsonException("Unexpected end of input: ") {
    this->operator<<(m);
  }
};

class JsonUnexpectedCharacter : public JsonException {
public:
  JsonUnexpectedCharacter(const char *m, char c)
      : JsonException("Unexpected character ") {
    (*this) << c << " " << m;
  }
};

class JsonUnicodeEscapeException : public JsonException {
public:
  JsonUnicodeEscapeException(const char *m) : JsonException("Escaped unicode: "){
    (*this) << m;
  }
  JsonUnicodeEscapeException(const char *m, char c) : JsonException("Escaped unicode: "){
    (*this) << m << "; instead got '" << c << '\'';
  }
  template <typename V>
  requires(std::is_integral_v<V> && sizeof(V) > 1)
  JsonUnicodeEscapeException(const char *m, V v) : JsonException("Escaped unicode: "){
    (*this) << m << ": " << v;
  }
};

template <typename C> static constexpr bool isJsonWhitespace(const C &c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

template <typename CodePoint, class Function>
bool codePointToUtf8(Function add, const CodePoint &c) {
  char8_t utf8[4];
  char8_t *end;
  char32_t value = c;
  if constexpr (sizeof(CodePoint) == 1) {
    value &= 0x00ff;
  }
  end = LeadingMarker<4, char8_t, char32_t>::unsafeEncode(value, utf8);
  bool added = true;
  for (const char8_t *begin = utf8; begin < end && added; begin++) {
    added &= add(static_cast<char>(*begin));
  }
  return added;
}

struct JsonEscapeState {
  uint16_t type = 0;
  uint16_t count = 0;
  char32_t value = 0;

  bool operator == (const JsonEscapeState &other) const {
    return type == other.type && count == other.count && value == other.value;
  }
};

template <typename CodePoint, class Function>
requires(std::is_same_v<CodePoint, char> || std::is_same_v<CodePoint, char8_t> || std::is_same_v<CodePoint, char16_t> || std::is_same_v<CodePoint, char32_t>) //
    bool addJsonStringCharacter(const CodePoint &cp, JsonEscapeState &escaped,
                                Function add) {
  static constexpr char32_t MARK_LEADING = 0xD800;
  static constexpr char32_t MARK_TRAILING = 0xDC00;
  static constexpr char32_t MARK_MASK = 0xFC00;
  static constexpr char32_t MARK_NOMASK = 0x03ff;
  char32_t c = cp;
  if constexpr (std::is_same_v<CodePoint, char>) {
    c &= 0x00ff;
  }
  if (c < ' ') {
    return false;
  }
  if (escaped.type == 0) {
    if (c <= 127) {
      if (c == '\\') {
        escaped.type = 1;
        return true;
      }
      return add(char(c));
    }
    if constexpr (sizeof(CodePoint) == 1) {
      return add(cp);
    }
    return codePointToUtf8(add, c);
  }
  if (escaped.type == 1) {
    if (c == '\\' || c == '/' || c == '"') {
      escaped = {};
      return add(c);
    }
    if (c == 'b') {
      escaped = {};
      return add('\b');
    }
    if (c == 'f') {
      escaped = {};
      return add('\f');
    }
    if (c == 'n') {
      escaped = {};
      return add('\n');
    }
    if (c == 'r') {
      escaped = {};
      return add('\r');
    }
    if (c == 't') {
      escaped = {};
      return add('\t');
    }
    if (c == 'u') {
      escaped = {};
      escaped.type = 2;
      return true;
    }
    throw JsonUnexpectedCharacter("is not part of a valid escape sequence", c);
  }
  if (escaped.type == 2) {
    if (escaped.count == 4) {
      if (c != '\\') {
        throw JsonUnicodeEscapeException("Expecting trailing surrogate pair, starting with '\\'", char(c));
      }
    }
    else if (escaped.count == 5) {
      if (c != 'u') {
        throw JsonUnicodeEscapeException("Expecting trailing surrogate pair, starting with \"\\u\"", char(c));
      }
    }
    else {
      escaped.value <<= 4;
      if (c >= '0' && c <= '9')  {
        escaped.value += (c - '0');
      }
      else if (c >= 'a' && c <= 'f') {
        escaped.value += (c - 'a' + 10);
      }
      else {
        throw JsonUnicodeEscapeException("Expected hexadecimal digit", char(c));
      }
      if (escaped.count == 3) {
        if (escaped.value < 0xd800 || escaped.value >= 0xe000) {
          bool result = codePointToUtf8(add, escaped.value);
          escaped = {};
          return result;
        }
        if ((escaped.value & MARK_MASK) == MARK_LEADING) {
          escaped.value &= MARK_NOMASK;
        }
        else {
          throw JsonUnicodeEscapeException("Invalid leading surrogate value", char16_t (escaped.value));
        }
      }
      if (escaped.count == 9) {
        if ((escaped.value & MARK_MASK) == MARK_TRAILING) {
          char32_t hi = (escaped.value & 0xffff0000) >> 6;
          char32_t lo = escaped.value & MARK_NOMASK;
          char32_t cp = (hi | lo) + 0x10000;
          bool result = codePointToUtf8(add, cp);
          escaped = {};
          return result;
        }
        else {
          throw JsonUnicodeEscapeException("Invalid trailing surrogate unicode", char16_t (escaped.value));
        }
      }
    }
    escaped.count++;
  }

  return true;
}

class JsonStringBuilder {
  virtual bool addChar(const char &c) = 0;
  virtual const char *getValue() const = 0;
  virtual size_t getLength() const = 0;

  uint64_t escaped = 0;

public:
  const char *string() const {
    if (getLength() == 0) {
      throw JsonException("Empty name");
    } else if (escaped) {
      throw JsonException(
          "String terminated in the middle of an escape sequence.");
    }
    return getValue();
  }

  template <typename CodePoint> bool add(CodePoint cp) {
    return addJsonStringCharacter(cp, escaped,
                                  [this](char c) { return addChar(c); });
  }
};

class JsonContext {
public:
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_JSON_H
