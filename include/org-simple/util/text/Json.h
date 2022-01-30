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

#include <memory>
#include <org-simple/util/text/CharEncode.h>
#include <org-simple/util/text/EchoStream.h>
#include <org-simple/util/text/InputStream.h>
#include <stdexcept>
#include <string>

namespace org::simple::util::text {

template <typename CodePoint>
concept isCodePoint =
    std::is_same_v<CodePoint, char> || std::is_same_v<CodePoint, char8_t> ||
    std::is_same_v<CodePoint, char16_t> || std::is_same_v<CodePoint, char32_t>;

class JsonException : public std::exception {
  std::string message;

public:
  JsonException(const char *m) : message("JSON: ") { message += m; }

  JsonException &operator<<(const char *snippet) {
    message += snippet;
    return *this;
  }

  template <typename C>
  requires(isCodePoint<C>) //
      JsonException &addChar(const C &c) {
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
      for (int i = sizeof(C) * 4 - 4; i >= 0; i -= 4) {
        message += digit[(c >> i) & 0x0f];
      }
    }
    return *this;
  }

  template <typename V>
  requires(std::is_integral_v<V>) JsonException &addHexDigits(const V &v) {
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
  template <typename C>
  requires(isCodePoint<C>) //
      JsonUnexpectedCharacter(const char *m, C c)
      : JsonException("Unexpected character '") {
    this->addChar(c) << "' " << m;
  }
};

class JsonUnicodeEscapeException : public JsonException {
public:
  JsonUnicodeEscapeException(const char *m)
      : JsonException("Escaped unicode: ") {
    (*this) << m;
  }
  template <typename C>
  requires(std::is_integral_v<C>) //
      JsonUnicodeEscapeException(const char *m, C c, bool asChar)
      : JsonException("Escaped unicode: ") {
    (*this) << m << "; instead got ";
    if (asChar) {
      this->addChar('\'').addChar(c).addChar('\'');
    } else {
      this->addHexDigits(c);
    }
    this->operator<<("'");
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

  bool operator==(const JsonEscapeState &other) const {
    return type == other.type && count == other.count && value == other.value;
  }
};

template <typename CodePoint, class Function>
requires(isCodePoint<CodePoint>) //
    bool addJsonStringCharacter(const CodePoint &cp, JsonEscapeState &escaped,
                                Function add);

class JsonStringBuilder {
  JsonEscapeState escaped = {};

protected:
  virtual const char *getValue() const = 0;
  virtual bool addChar(const char &c) = 0;

public:
  virtual size_t getLength() const = 0;
  virtual void resetValue() = 0;

  void reset() {
    escaped = {};
    resetValue();
  }

  const char *getString() const {
    if (escaped.type) {
      JsonException e("Unfinished escape sequence.");
      if (getLength() > 0) {
        e << ": \"" << getString() << "\"";
      }
      throw e;
    }
    return getValue();
  }

  const char *getName() const {
    const char *value = getString();
    if (getLength() == 0 || *value == '\0') {
      throw JsonException("Empty name");
    }
    return value;
  }

  template <typename CodePoint> void add(CodePoint cp) {
    if (!addJsonStringCharacter(cp, escaped,
                                [this](char c) { return addChar(c); })) {
      throw JsonException("Maximum string or name length exceeded.");
    }
  }
};

class DefaultJsonStringBuilder : public JsonStringBuilder {
  std::unique_ptr<char> ptr;
  char *at;
  char *end;
  const size_t max;

  bool resize() {
    char *start = ptr.get();
    size_t capacity = end - start;
    if (capacity >= max) {
      return false;
    }
    size_t newCapacity = max / 2 > capacity ? capacity * 2 : max;
    char *p = new char[newCapacity];

    std::copy(start, at + 1, p);
    const auto atPos = at - start;
    ptr.reset(p);
    end = p + newCapacity;
    at = p + atPos;
    return true;
  }

protected:
  const char *getValue() const final { return ptr.get(); }

  bool addChar(const char &c) final {
    if ((at < end - 1) || (resize())) {
      *at++ = c;
      *at = '\0';
      return true;
    }
    return false;
  }

public:
  DefaultJsonStringBuilder(size_t minCapacity, size_t maxCapacity)
      : ptr(new char[std::max(10lu, minCapacity)]), at(ptr.get()),
        end(at + std::max(10lu, minCapacity)), max(maxCapacity) {}

  size_t getLength() const final { return at - ptr.get(); }

  void resetValue() final {
    at = ptr.get();
    *at = '\0';
  }
};

class JsonContext {
  enum class State { Object, AssignMent, PreValue, PostValue, Array };

  template <typename C>
  requires(isCodePoint<C>) //
      static void readJsonName(JsonContext &context, InputStream<C> &input) {
    JsonStringBuilder &name = context.nameBuilder();
    name.resetValue();
    C c;
    while (input.get(c)) {
      if (c == '"') {
        context.pushName(name.getName());
        return;
      }
      name.add(c);
    }
    JsonUnexpectedEndOfInput e("While reading object name");
    if (name.getLength() > 0) {
      e << ": \"" << name.getString() << "\"";
    }
    throw e;
  }

  template <typename C>
  requires(isCodePoint<C>) //
      static void readJsonString(JsonContext &context, InputStream<C> &input) {
    JsonStringBuilder &string = context.stringBuilder();
    string.resetValue();
    C c;
    while (input.get(c)) {
      if (c == '"') {
        context.setString(string.getString());
        return;
      }
      string.add(c);
    }
    JsonUnexpectedEndOfInput e("While reading object name");
    if (string.getLength() > 0) {
      e << ": \"" << string.getString() << "\"";
    }
    throw e;
  }

  template <typename C>
  requires(isCodePoint<C>) //
      static void readJsonNumber(JsonContext &context, EchoStream<C> &input) {
    JsonStringBuilder &string = context.stringBuilder();
    string.resetValue();
    C c;
    while (input.get(c)) {
      if ((c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-' ||
          c == 'e' || c == 'E') {
        string.add(c);
      } else if (isJsonWhitespace(c)) {
        context.setNumber(string.getString());
        return;
      } else if (c == '}' || c == ',' || c == ']') {
        input.repeat();
        context.setNumber(string.getString());
        return;
      }
    }
    JsonUnexpectedEndOfInput e("While reading object name");
    if (string.getLength() > 0) {
      e << ": \"" << string.getString() << "\"";
    }
    throw e;
  }

  template <typename C>
  requires(isCodePoint<C>) static void match(InputStream<C> &input,
                                             const char *value) {
    C c;
    const char *p = value;
    for (; *p != 0 && input.get(c); p++) {
      if (C(*p) != c) {
        throw JsonUnexpectedCharacter("Invalid literal constant", c);
      }
    }
    if (*p != 0) {
      throw JsonUnexpectedEndOfInput("While reading literal constant");
    }
  }

  template <typename C>
  requires(isCodePoint<C>) //
      static void readValue(JsonContext &context, C c, EchoStream<C> &input) {
    if (c == '"') {
      readJsonString(context, input);
    } else if (c == 't') {
      input.replay(c);
      match(input, "true");
      context.setBoolean(true);
    } else if (c == 'f') {
      input.replay(c);
      match(input, "false");
      context.setBoolean(false);
    } else if (c == 'n') {
      input.replay(c);
      match(input, "null");
      context.setNull();
    } else if ((c >= '0' && c <= '9') || (c == '.') || (c == '+') || c == '-') {
      input.replay(c);
      readJsonNumber(context, input);
    } else {
      throw JsonUnexpectedCharacter("Expecting valid start of value", c);
    }
  }

  template <typename C>
  requires(isCodePoint<C>) //
      static void readJsonArray(JsonContext &context, EchoStream<C> &input) {
    int count = 0;
    C c;
    State state = State::Array;
    while (input.get(c)) {
      switch (state) {
      case State::Array:
        if (c == ']') {
          return;
        }
        context.pushIndex(count);
        count++;
        state = State::PreValue;
        input.repeat();
        break;
      case State::PreValue:
        if (c == '[') {
          readJsonArray(context, input);
        } else if (c == '{') {
          readJsonObject(context, input);
        } else if (isJsonWhitespace(c)) {
          break;
        } else {
          readValue(context, c, input);
        }
        context.popIndex();
        state = State::PostValue;
        break;
      case State::PostValue:
        if (c == ',') {
          state = State::Array;
        } else if (c == ']') {
          return;
        } else if (!isJsonWhitespace(c)) {
          throw JsonUnexpectedCharacter("Expecting next value or array end ']'",
                                        c);
        }
        break;
      default:
        break;
      }
    }
  }

  template <typename C>
  requires(isCodePoint<C>) //
      static void readJsonObject(JsonContext &context, EchoStream<C> &input) {
    C c;
    State state = State::Object;
    while (input.get(c)) {
      switch (state) {
      case State::Object:
        if (c == '"') {
          state = State::AssignMent;
          readJsonName(context, input);
        } else if (c == '}') {
          return;
        } else if (!isJsonWhitespace(c)) {
          throw JsonUnexpectedCharacter("Expecting quote '\"' to start name",
                                        c);
        }
        break;
      case State::AssignMent:
        if (c == ':') {
          state = State::PreValue;
        } else if (!isJsonWhitespace(c)) {
          throw JsonUnexpectedCharacter("Expecting assignment ':'", c);
        }
        break;
      case State::PreValue:
        if (c == '[') {
          readJsonArray(context, input);
        } else if (c == '{') {
          readJsonObject(context, input);
        } else if (isJsonWhitespace(c)) {
          break;
        } else {
          readValue(context, c, input);
        }
        context.popName();
        state = State::PostValue;
        break;
      case State::PostValue:
        if (c == ',') {
          state = State::Object;
        } else if (c == '}') {
          return;
        } else if (!isJsonWhitespace(c)) {
          throw JsonUnexpectedCharacter(
              "Expecting next value or object end '}'", c);
        }
        break;
      default:
        break;
      }
    }
  }

protected:
  virtual JsonStringBuilder &nameBuilder() = 0;
  virtual JsonStringBuilder &stringBuilder() = 0;
  virtual void pushIndex(int index) = 0;
  virtual void popIndex() = 0;
  virtual void pushName(const char *name) = 0;
  virtual void popName() = 0;
  virtual void setString(const char *string) = 0;
  virtual void setNumber(const char *string) = 0;
  virtual void setBoolean(bool value) = 0;
  virtual void setNull() = 0;

public:
  template <typename C>
  static void readJson(JsonContext &context, InputStream<C> &input) {
    EchoStream<C> replayStream(&input);
    C c;
    while (replayStream.get(c)) {
      if (c == '{') {
        readJsonObject(context, replayStream);
        return;
      } else if (!isJsonWhitespace(c)) {
        throw JsonUnexpectedCharacter("Expecting start of root-object '{'", c);
      }
    }
  }
};

template <typename CodePoint, class Function>
requires(isCodePoint<CodePoint>) //
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
        throw JsonUnicodeEscapeException(
            "Expecting trailing surrogate pair, starting with '\\'", c, true);
      }
    } else if (escaped.count == 5) {
      if (c != 'u') {
        throw JsonUnicodeEscapeException(
            "Expecting trailing surrogate pair, starting with \"\\u\"", c,
            true);
      }
    } else {
      escaped.value <<= 4;
      if (c >= '0' && c <= '9') {
        escaped.value += (c - '0');
      } else if (c >= 'a' && c <= 'f') {
        escaped.value += (c - 'a' + 10);
      } else {
        throw JsonUnicodeEscapeException("Expected hexadecimal digit", c, true);
      }
      if (escaped.count == 3) {
        if (escaped.value < 0xd800 || escaped.value >= 0xe000) {
          bool result = codePointToUtf8(add, escaped.value);
          escaped = {};
          return result;
        }
        if ((escaped.value & MARK_MASK) == MARK_LEADING) {
          escaped.value &= MARK_NOMASK;
        } else {
          throw JsonUnicodeEscapeException("Invalid leading surrogate value",
                                           char16_t(escaped.value), false);
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
        } else {
          throw JsonUnicodeEscapeException("Invalid trailing surrogate unicode",
                                           char16_t(escaped.value), false);
        }
      }
    }
    escaped.count++;
  }

  return true;
}

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_JSON_H
