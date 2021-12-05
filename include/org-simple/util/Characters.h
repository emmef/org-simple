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

struct Utf8 {

  struct ExtensionByte {
    static constexpr char MARKER_MASK = char(0xc0);
    static constexpr char VALUE_MASK = char(~0xc0);
    static constexpr char MARKER = char(0x80);
    static constexpr short BITS = 6;

    static constexpr char32_t unsafeScatter(char32_t c, char *p) {
      *p = MARKER | char(VALUE_MASK & c);
      return c >> BITS;
    }

    static constexpr bool is(char c) { return char(c & MARKER_MASK) == MARKER; }

    static constexpr char getMaskedValue(char c) { return char(c & VALUE_MASK); }
  };

  template <short B> struct Range {
    static constexpr char32_t MAX_CODEPOINT = 0x0010ffff;
    static_assert(B >= 1 && B <= 4);

    static constexpr short BYTES = B;
    static constexpr short BITS = BYTES == 1 ? 7 : BYTES * 5 + 1;
    static constexpr char MARKER =
        BYTES == 1 ? 0 : char(0xfffffffe << (7 - BYTES));
    static constexpr char MARKER_MASK =
        BYTES == 1 ? char(0x80) : char(0xffffffff << (7 - BYTES));
    static constexpr char VALUE_MASK = ~MARKER_MASK;
    static constexpr char32_t LAST_CODEPOINT =
        std::min(MAX_CODEPOINT, char32_t((1 << BITS) - 1));

    static constexpr char32_t FIRST_CODEPOINT =
        BYTES == 1
            ? 0
            : Range<std::max(short(BYTES - 1), short(1))>::LAST_CODEPOINT + 1;

    /**
     * Returns whether the given character is a byte marker for a BYTE bytes
     * code-point.
     */
    static constexpr bool isByteMarker(char c) {
      char check = char(c & MARKER_MASK);
      return check == MARKER;
    }

    static constexpr char32_t getMaskedValue(char c) { return c & VALUE_MASK; }

    static constexpr char gerMarkerByte(char c) {
      return char(MARKER | getMaskedValue(c));
    }

  private:
    friend struct Range<1>;
    friend struct Range<2>;
    friend struct Range<3>;
    friend struct Range<4>;
    friend struct Utf8;

    static constexpr short getBytesFromMarker(char c) {
      if constexpr (BYTES > 1) {
        return isByteMarker(c) ? BYTES
                               : Range<BYTES - 1>::getBytesFromMarker(c);
      } else {
        return isByteMarker(c) ? BYTES : 0;
      }
    }

    static constexpr short getBytesForCharacter(char32_t c) {
      if constexpr (BYTES < 4) {
        return c <= LAST_CODEPOINT ? BYTES
                                   : Range<BYTES + 1>::getBytesForCharacter(c);
      } else {
        return c <= LAST_CODEPOINT ? BYTES : 0;
      }
    }

    static constexpr char *unsafeScatter(char32_t c, char *destination) {
      char32_t remainder = c;
      for (short i = BYTES - 1; i > 0; i--) {
        remainder = ExtensionByte::unsafeScatter(remainder, destination + i);
      }
      char marker = Range<BYTES>::MARKER;
      char mask = Range<BYTES>::VALUE_MASK;
      *destination = marker | char(mask & remainder);
      return destination + BYTES;
    }
  };

  static constexpr short getBytesFromMarker(char c) {
    return Range<4>::getBytesFromMarker(c);
  }

  static constexpr short getBytesForCharacter(char32_t c) {
    return Range<1>::getBytesForCharacter(c);
  }

  /**
   * Write the specified unicode character \c c to a \c destination and return
   * the position after the written bytes, or \c nullptr is the character
   * cannot be represented in UTF-8. This function does not check whether
   * \c destination is \c nullptr.
   * @param c The unicode character
   * @param destination The character array that will contain the bytes.
   * @return pointer to the position in \c destination to write the next
   * character.
   */
  static constexpr char *unsafeScatter(char32_t c, char *destination) {
    switch (getBytesForCharacter(c)) {
    case 1:
      return Range<1>::unsafeScatter(c, destination);
    case 2:
      return Range<2>::unsafeScatter(c, destination);
    case 3:
      return Range<3>::unsafeScatter(c, destination);
    case 4:
      return Range<4>::unsafeScatter(c, destination);
    default:
      return nullptr;
    }
  }

  static constexpr bool isUtf8(char32_t c) {
    return getBytesForCharacter(c) != 0;
  }
  static constexpr bool isAscii(char32_t c) {
    return Range<1>::isByteMarker(c);
  }

  class Reader {
    int bytesToRead_ = 0;
    char32_t character_ = 0;

  public:
    enum class State { OK, READING, INVALID };

    [[nodiscard]] int bytesToRead() const { return bytesToRead_; }
    [[nodiscard]] State state() const {
      return bytesToRead_ == 0 ? State::OK : State::READING;
    }

    [[nodiscard]] char32_t value() const { return character_; }

    char32_t getValueAndReset()  {
      if (bytesToRead_) {
        return -1;
      }
      char32_t r = character_;
      character_ = 0;
      return r;
    }

    State addGetState(char byte) {
      if (bytesToRead_ == 0) {
        short bytes = getBytesFromMarker(byte);
        switch (bytes) {
        case 1:
          character_ = byte;
          break;
        case 2:
          character_ = Range<2>::getMaskedValue(byte);
          break;
        case 3:
          character_ = Range<3>::getMaskedValue(byte);
          break;
        case 4:
          character_ = Range<4>::getMaskedValue(byte);
          break;
        default:
          return State::INVALID;
        }
        bytesToRead_ = bytes - 1;
        return State::READING;
      } else if (Utf8::ExtensionByte::is(byte)) {
        character_ <<= ExtensionByte::BITS;
        character_ |= ExtensionByte::getMaskedValue(byte);
        return --bytesToRead_ ? State::READING : State::OK;
      }
      return State::INVALID;
    }

    static char32_t read(const char *text) {
      return text ? readUnsafe(text) : -1;
    }

    static char32_t readUnsafe(const char *text) {
      const char *ptr = text;
      if (isAscii(*ptr)) {
        return *ptr;
      }
      Reader reader;
      if (reader.addGetState(*ptr++) != Reader::State::READING) {
        return -1;
      }
      Reader::State state;
      do {
        state = reader.addGetState(*ptr++);
      } while (state == Reader::State::READING);
      return state == Reader::State::OK ? reader.value() : -1;
    }

    static char32_t readGextNext(const char *text, const char **next) {
      return text ? readUnsafeGetNext(text, next) : -1;
    }

    static char32_t readUnsafeGetNext(const char *text, const char **next) {
      if (next == nullptr) {
        return readUnsafe(text);
      }
      const char *ptr = text;
      if (isAscii(*ptr)) {
        *next = ptr + 1;
        return *ptr;
      }
      Reader reader;
      if (reader.addGetState(*ptr++) != Reader::State::READING) {
        *next = text;
        return -1;
      }
      Reader::State state;
      do {
        state = reader.addGetState(*ptr++);
      } while (state == Reader::State::READING);
      if (state == Reader::State::OK) {
        *next = ptr;
        return reader.value();
      }
      *next = ptr;
      return -1;
    }
  };
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_CHARACTERS_H
