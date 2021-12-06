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
#include <org-simple/core/Bits.h>

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

namespace ByteEncoding {
static constexpr int getBitsForByteCount(int bytes) {
  return bytes == 1 ? 7 : bytes * 5 + 1;
}

struct Marker {
  const int markerBits;
  const int valueBits = 8 - markerBits;
  const char maskMarker = char(0xff00 >> markerBits);
  const char marker = char(maskMarker << 1);
  const char maskValue = ~maskMarker;

  /**
   * @return whether {@code byte} contains a valid marker.
   */
  bool is(char byte) const { return char(byte & maskMarker) == marker; }

  /**
   * @return the bits without the marker, that are alsways the less
   * significant bits of the byte.
   */
  char valueFrom(char byte) const { return char(maskValue & byte); }

  /**
   * @return a value that contains a valid marker and the lower significant
   * bits from {@code codePoint} that can be stored next to it.
   */
  char pack(char32_t codePoint) const { return marker | valueFrom(codePoint); }

  /**
   * Packs {@code codePoint} in {@code destination} and returns the
   * right-shifted value of {@code value} without the packed value bits.
   * @param codePoint The value to pack.
   * @param destination The destination to write the packed value to.
   * @return the right-shifted value of {@code codePoint} without the packed
   * value bits.
   */
  char32_t packGetShifted(char32_t codePoint, char &destination) const {
    destination = pack(codePoint);
    return destination >> valueBits;
  }
};

struct Continuation : public Marker {
  constexpr Continuation() : Marker{2} {}
} static constexpr continuation;

struct Leading : public Marker {
  const int encodedBytes;
  const int totalEncodedBits;
  const char32_t minimumCodePoint;
  const char32_t maximumCodePoint;

  constexpr Leading(int B)
      : Marker{B == 1 ? 1 : B + 1}, encodedBytes(B),
        totalEncodedBits(getBitsForByteCount(B)),
        minimumCodePoint(B == 1 ? 0
                                : char32_t(1) << getBitsForByteCount(B - 1)),
        maximumCodePoint((char32_t(1) << totalEncodedBits) - 1) {}

  bool inside(char32_t codePoint) const {
    return codePoint >= minimumCodePoint && codePoint <= maximumCodePoint;
  }
  char *unsafeEncode(char32_t c, char *bytes) const {
    char32_t remainder = c;
    for (int i = encodedBytes - 1; i > 0; i--) {
      remainder = continuation.packGetShifted(remainder, bytes[i]);
    }
    bytes[0] = pack(remainder);
    return bytes + encodedBytes;
  }

  char *unsafeDecode(char *bytes, char32_t &decoded) const {
    char *ptr = bytes;
    char32_t sum = valueFrom(*bytes++);
    for (int at = 1; at < encodedBytes; at++) {
      char next = *ptr++;
      if (!continuation.is(next)) {
        return nullptr;
      }
      sum <<= continuation.valueBits;
      sum += continuation.valueFrom(next);
    }
    decoded = sum;
    return ptr;
  }

} static constexpr leading[] = {1, 2, 3, 4, 5, 6};

template <int B, char32_t MAX> struct OfLength {
  static_assert(B >= 0 && B <= 6);
  static_assert(MAX == 0 || (MAX >= leading[B - 1].minimumCodePoint &&
                                MAX <= leading[B - 1].maximumCodePoint));

  static constexpr char32_t maxCodePoint =
      MAX == 0 ? Leading(B).maximumCodePoint
               : std::min(MAX, Leading(B).maximumCodePoint);


  static int encodedBytes(char firstByte) {
    for (int i = 0; i < B; i++) {
      if (leading[i].is(firstByte)) {
        return leading[i].encodedBytes;
      }
    }
    return 0;
  }

    static int encodingBytes(char32_t codePoint) {
    if (codePoint > maxCodePoint) {
      return 0;
    }
    for (int i = 0; i < B; i++) {
      if (leading[i].inside(codePoint)) {
        return leading[i].encodedBytes;
      }
    }
    return 0;
  }
  static char *encodeBytes(char32_t codePoint, char *bytes) {
    if (codePoint > maxCodePoint) {
      return nullptr;
    }
    for (int i = 0; i < B; i++) {
      if (leading[i].inside(codePoint)) {
        return leading[i].unsafeEncode(codePoint, bytes);
      }
    }
    return nullptr;
  }
  static char *decodeBytes(char *bytes, char32_t &decoded) {
    for (int i = 0; i < B; i++) {
      if (leading[i].is(*bytes)) {
        char32_t r;
        char *string = leading[i].unsafeDecode(bytes, r);
        if (string != nullptr && r <= maxCodePoint) {
          decoded = r;
          return string;
        }
      }
    }
    return nullptr;
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

    char32_t getValueAndReset() {
      if (bytesToRead_) {
        return -1;
      }
      char32_t r = character_;
      character_ = 0;
      return r;
    }

    State addGetState(char byte) {
      if (bytesToRead_ == 0) {
        for (int i = 0; i < B; i++) {
          if (leading[i].is(byte)) {
            character_ =  leading[i].valueFrom(byte);
            bytesToRead_ = i; // encoded bytes minus one
          }
        }
      } else if (continuation.is(byte)) {
        character_ <<= continuation.valueBits;
        character_ |= continuation.valueFrom(byte);
      }
      else {
        return State::INVALID;
      }
      return --bytesToRead_ ? State::READING : State::OK;
    }
  };
};
}; // namespace ByteEncoding

struct Utf8 : public ByteEncoding::OfLength<4, 0x0010ffff> {
  static constexpr char32_t MAX_UTF8_CODEPOINT = 0x0010ffff;
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_CHARACTERS_H
