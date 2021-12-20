#ifndef ORG_SIMPLE_CHARENCODE_H
#define ORG_SIMPLE_CHARENCODE_H
/*
 * org-simple/CharEncode.h
 *
 * Added by michel on 2021-12-07
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
#include <limits>
#include <type_traits>

namespace org::simple::charEncode {

enum class DecodingReaderState { OK, READING, INVALID };

template <typename byte, typename char_type>
concept validByteAndCharacter = std::is_integral_v<byte> &&
                                std::is_unsigned_v<byte> &&
                                std::is_integral_v<char_type> &&
                                sizeof(char_type) >= sizeof(byte);

template <typename byte, typename char_type>
concept validByteAndUnsignedCodePoint =
    validByteAndCharacter<byte, char_type> && std::is_unsigned_v<char_type>;

template <typename byte, typename char_type> struct CharacterConverter {
  static_assert(validByteAndCharacter<byte, char_type>);

  static constexpr char_type maskByte = ~(char_type(~0) << (sizeof(byte) * 8));

  static constexpr byte toByte(const char_type &source) {
    if constexpr (sizeof(byte) == sizeof(char_type)) {
      return byte(source);
    }
    return (byte)source;
  }

  static constexpr char_type toChar(const byte &source) {
    if constexpr (sizeof(byte) == sizeof(char_type)) {
      return char_type(source);
    }
    return char_type(source) & maskByte;
  }
};

/**
 * @tparam B The "byte" type, that must be unsigned.
 * @tparam C The code-point type, that must be unsigned and at least as large as
 * the "byte" type.
 * @tparam L An arbitrary limit on the maximum code-point, say for UTF-8.
 */
template <int MARKER_BITS, typename B = char8_t, typename C = char32_t>
struct AbstractMarker {
  static_assert(validByteAndUnsignedCodePoint<B, C>);
  static_assert(
      MARKER_BITS >= 1 && MARKER_BITS <= (sizeof(B) * 8 - 1),
      "Marker must be at least one bit and smaller than bits in a byte.");

  typedef B byte;
  typedef C codePoint;

  static constexpr int markerBits = MARKER_BITS;
  static constexpr int valueBits = 8 - markerBits;
  static constexpr byte maskMarker =
      byte(((long long unsigned)~0) << (sizeof(byte) * 8 - markerBits));
  static constexpr byte marker = byte(maskMarker << 1);
  static constexpr byte maskValue = ~maskMarker;
  static constexpr codePoint maskByte = CharacterConverter<B, C>::maskByte;

  /**
   * @return whether {@code b} contains a valid marker.
   */
  static constexpr bool is(const byte b) {
    return byte(b & maskMarker) == marker;
  }

  /**
   * @return the bits without the marker, that are alsways the less
   * significant bits of the byte.
   */
  static constexpr byte valueFrom(byte b) { return byte(maskValue & b); }

  /**
   * @return a value that contains a valid marker and the lower significant
   * bits from {@code b} that can be stored next to it.
   */
  static constexpr byte pack(byte b) { return marker | valueFrom(b); }

  /**
   * Packs {@code cp} in {@code destination} and returns the
   * right-shifted value of {@code value} without the packed value bits.
   * @param cp The value to pack.
   * @param destination The destination to write the packed value to.
   * @return the right-shifted value of {@code cp} without the packed
   * value bits.
   */
  static constexpr codePoint packGetShifted(codePoint cp, byte &destination) {
    destination = pack(cp);
    return cp >> valueBits;
  }
};

/**
 * @tparam B The "byte" type, that must be unsigned.
 * @tparam C The code-point type, that must be unsigned and at least as large as
 * the "byte" type.
 */
template <typename B = char8_t, typename C = char32_t>
using ContinuationMarker = AbstractMarker<2, B, C>;

template <int ENCODED_BYTES, typename B = char8_t>
static constexpr int getBitsForByteCount() {
  const int byteBits = sizeof(B) * 8;
  return ENCODED_BYTES == 1 ? byteBits - 1 : 1 + ENCODED_BYTES * (byteBits - 3);
}

/**
 * @tparam ENCODED_BYTES Number of encoded bytes that this leading marker
 * represents.
 * @tparam B The "byte" type, that must be unsigned.
 * @tparam C The code-point type, that must be unsigned and at least as large as
 * the "byte" type.
 * @tparam L An arbitrary limit on the maximum code-point, say for UTF-8.
 */
template <int ENCODED_BYTES, typename B = char8_t, typename C = char32_t,
          C L = std::numeric_limits<C>::max()>
struct LeadingMarker
    : public AbstractMarker<ENCODED_BYTES == 1 ? 1 : ENCODED_BYTES + 1, B, C> {
  static_assert(
      ENCODED_BYTES >= 1 && ENCODED_BYTES <= (sizeof(B) * 8 - 2),
      "Maximum number of bytes to encode exceeded, given the byte-size.");
  static_assert(ENCODED_BYTES == 1 ||
                    (getBitsForByteCount<ENCODED_BYTES - 1, B>() <=
                     (sizeof(C) * 8)),
                "Lowest code-point for number of encoded bytes would already "
                "exceed value that can be stored in code-point type");

  using Parent =
      AbstractMarker<ENCODED_BYTES == 1 ? 1 : ENCODED_BYTES + 1, B, C>;
  using Continuation = ContinuationMarker<B, C>;

  using byte = typename Parent::byte;
  using codePoint = typename Parent::codePoint;

  using Parent::marker;
  using Parent::markerBits;
  using Parent::maskByte;
  using Parent::maskMarker;
  using Parent::maskValue;
  using Parent::valueBits;

  using Parent::is;
  using Parent::pack;
  using Parent::packGetShifted;
  using Parent::valueFrom;

  static constexpr int encodedBytes = ENCODED_BYTES;
  static constexpr int totalEncodedBits =
      getBitsForByteCount<encodedBytes, B>();
  static constexpr codePoint minimumCodePoint =
      encodedBytes == 1
          ? 0
          : codePoint(1) << getBitsForByteCount<encodedBytes - 1, B>();

  static_assert(L >= minimumCodePoint,
                "Cannot have explicit limit below the minimum code point");

  static constexpr codePoint theoreticalMaximumCodePoint =
      totalEncodedBits == sizeof(codePoint) * 8
          ? ~0
          : (codePoint(1) << totalEncodedBits) - 1;
  static constexpr codePoint maximumCodePoint =
      std::min(L, theoreticalMaximumCodePoint);

  /**
   * Returns whether the code-point {@code p} is inside the encoding-range of
   * this marker. Often, specification forbid to encode numbers that would fir
   * in less bytes.
   * @param cp The code-point to test.
   * @return whether the code-point {@code p} is inside the encoding-range of
   * this marker.
   */
  static constexpr bool inside(codePoint cp) {
    return cp >= minimumCodePoint && cp <= maximumCodePoint;
  }

  /**
   * Encode code-point {@code c} with {@code encodedBytes} bytes, regardless of
   * code-point value. If the maximum code point is exceeded
   * @param c The code-point to encode.
   * @param bytes The buffer that contains the encoded results, that is not
   * checked for {@code nullptr}.
   * @return the next position to encode to, i.e., {@code bytes + encodedBytes}
   */
  static constexpr byte *unsafeFixedLengthEncode(codePoint c,
                                                 byte *const bytes) {
    if (maxCodePointExceeded(c)) {
      return nullptr;
    }
    codePoint remainder = c;
    for (int i = encodedBytes - 1; i > 0; i--) {
      remainder = Continuation::packGetShifted(remainder, bytes[i]);
    }
    bytes[0] = pack(remainder);
    return bytes + encodedBytes;
  }

  /**
   * Encode code-point {@code c} with {@code encodedBytes} or less bytes,
   * without {@code nullptr} check for {@code bytes}.
   * @param c The code-point to encode.
   * @param bytes The buffer that contains the encoded results, that is not
   * checked for {@code nullptr}.
   * @return the next position to encode to, i.e., {@code bytes + encodedBytes}
   */
  static constexpr byte *unsafeEncode(codePoint c, byte *const bytes) {
    if constexpr (encodedBytes == 1) {
      return (inside(c)) ? unsafeFixedLengthEncode(c, bytes) : nullptr;
    } else {
      byte *result =
          LeadingMarker<encodedBytes - 1, B, C, L>::unsafeEncode(c, bytes);
      return result      ? result
             : inside(c) ? unsafeFixedLengthEncode(c, bytes)
                         : nullptr;
    }
  }

  static constexpr char *unsafeEncode(codePoint c, char *const bytes) requires(
      !std::is_same_v<char, byte> && sizeof(byte) == sizeof(char)) {
    return reinterpret_cast<char *const>(
        unsafeEncode(c, reinterpret_cast<byte *const>(bytes)));
  }

  /**
   * Decodes the code-point in {@code bytes}, but if and only if that is encoded
   * with {@code encodedBytes} bytes and valid. If there is a valid decoded
   * code-point, it is placed in {@code decoded}.
   * @param bytes The buffer that contains the encoded input, that is not
   * checked for {@code nullptr}.
   * @param decoded The code-point that contains the encoded value if that was
   * valid, untouched otherwise.
   * @return the next position to encode to, i.e., {@code bytes + encodedBytes}
   */
  static constexpr const byte *unsafeFixedLengthDecode(const byte *const bytes,
                                                       codePoint &decoded) {
    const byte *ptr = bytes;
    if (!is(*ptr)) {
      return nullptr;
    }
    codePoint sum = valueFrom(*ptr++);
    for (int at = 1; at < encodedBytes; at++) {
      byte next = *ptr++;
      if (!Continuation::is(next)) {
        return nullptr;
      }
      sum <<= Continuation::valueBits;
      sum += Continuation::valueFrom(next);
    }
    if (maxCodePointExceeded(sum)) {
      return nullptr;
    }

    decoded = sum;
    return ptr;
  }

  /**
   * Decodes the code-point in {@code bytes}, but if and only if that is encoded
   * with {@code encodedBytes} bytes or less and valid. If there is a valid
   * decoded code-point, it is placed in {@code decoded}.
   * @param bytes The buffer that contains the encoded input, that is not
   * checked for {@code nullptr}.
   * @param decoded The code-point that contains the encoded value if that was
   * valid, untouched otherwise.
   * @return the next position to encode to, i.e., {@code bytes + encodedBytes}
   */
  static constexpr const byte *unsafeDecode(const byte *const bytes,
                                            codePoint &decoded) {
    if constexpr (encodedBytes == 1) {
      return unsafeFixedLengthDecode(bytes, decoded);
    } else {
      const byte *result =
          LeadingMarker<encodedBytes - 1, B, C, L>::unsafeDecode(bytes,
                                                                 decoded);
      return result ? result : unsafeFixedLengthDecode(bytes, decoded);
    }
  }

  /**
   * Returns the number of bytes that the {@code marker} represents if
   * it is a valid marker and the number of bytes is less than {@code
   * encodedBytes}, and zero otherwise.
   * @param marker The marker byte to test.
   * @return the number of encoded bytes represented or zero if invalid or too
   * large.
   */
  static constexpr int getBytesFromLeadingMarker(byte marker) {
    if constexpr (encodedBytes == 1) {
      return is(marker) ? 1 : 0;
    } else {
      const int bytes =
          LeadingMarker<encodedBytes - 1, B, C, L>::getBytesFromLeadingMarker(
              marker);
      return bytes != 0 ? bytes : is(marker) ? encodedBytes : 0;
    }
  }

  /**
   * Return the number of bytes required to encode the code-point {@code cp} if
   * that number of bytes is less than {@code encodedBytes}, and zero otherwise.
   * @param cp The code-point to test.
   * @return the number of bytes needed or zero if that is too large large.
   */
  static constexpr int getBytesFromCodePoint(codePoint cp) {
    if constexpr (encodedBytes == 1) {
      return inside(cp) ? encodedBytes : 0;
    } else {
      const int bytes =
          LeadingMarker<encodedBytes - 1, B, C, L>::getBytesFromCodePoint(cp);
      return bytes ? bytes : inside(cp) ? encodedBytes : 0;
    }
  }

  static constexpr int getBytesToReadSetInitialReaderValue(const byte input,
                                                           codePoint &value) {
    if constexpr (encodedBytes == 1) {
      return readerInitFixedLength(input, value);
    } else {
      const int bytes =
          LeadingMarker<encodedBytes - 1, B, C,
                        L>::getBytesToReadSetInitialReaderValue(input, value);
      return bytes ? bytes : readerInitFixedLength(input, value);
    }
  }

  class Reader {

  public:
    Reader() : bytesToRead_(0), character_(-1) {}

    [[nodiscard]] codePoint value() const { return character_; }

    codePoint getValueAndReset() {
      if (bytesToRead_) {
        return -1;
      }
      codePoint r = character_;
      character_ = 0;
      return r;
    }

    void reset() {
      character_ = 0;
      bytesToRead_ = 0;
    }

    template <typename char_type>
    requires(!std::is_same_v<byte, char_type> &&
             validByteAndCharacter<byte, char_type>) DecodingReaderState
        addGetState(char_type v) {
      return addGetState(CharacterConverter<byte, char_type>::toByte(v));
    }

    DecodingReaderState addGetState(byte byte) {
      if (bytesToRead_ == 0) {
        bytesToRead_ = getBytesToReadSetInitialReaderValue(byte, character_);
        if (bytesToRead_ == 0) {
          return DecodingReaderState::INVALID;
        }
      } else if (Continuation::is(byte)) {
        character_ <<= Continuation::valueBits;
        character_ |= Continuation::valueFrom(byte);
        if (maxCodePointExceeded(character_))
          return DecodingReaderState::INVALID;
      } else {
        return DecodingReaderState::INVALID;
      }
      return --bytesToRead_ ? DecodingReaderState::READING
                            : DecodingReaderState::OK;
    }

  private:
    int bytesToRead_;
    codePoint character_;
  };

private:
  static constexpr int readerInitFixedLength(const byte input,
                                             codePoint &value) {
    if (is(input)) {
      value = valueFrom(input);
      return encodedBytes;
    }
    return 0;
  }

  static constexpr bool maxCodePointExceeded(codePoint cp) {
    if constexpr (L < std::numeric_limits<codePoint>::max()) {
      return cp > L;
    }
    return false;
  }
};

template <int ENCODED_BYTES, typename B = char8_t, typename C = char32_t,
          C L = std::numeric_limits<C>::max()>
using DecodingReader = typename LeadingMarker<ENCODED_BYTES, B, C, L>::Reader;

typedef LeadingMarker<4, char8_t, char32_t, 0x0010ffff> Utf8Encoding;
typedef Utf8Encoding::Reader Utf8Reader;
typedef LeadingMarker<1, char8_t, char8_t> AsciiEncoding;
typedef AsciiEncoding::Reader AsciiReader;

} // namespace org::simple::charEncode

#endif // ORG_SIMPLE_CHARENCODE_H
