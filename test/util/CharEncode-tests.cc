//
// Created by michel on 07-12-21.
//

#include "boost-unit-tests.h"
#include <iostream>
#include <org-simple/core/Bits.h>
#include <org-simple/util/CharEncode.h>

typedef char8_t byte;
typedef char32_t codePoint;
template <int MARKER_BITS>
using Marker =
    org::simple::charEncode::AbstractMarker<MARKER_BITS, byte, codePoint>;
template <int MARKER_BITS>
using Leading =
    org::simple::charEncode::LeadingMarker<MARKER_BITS, byte, codePoint>;
using Continuation =
    org::simple::charEncode::ContinuationMarker<byte, codePoint>;
typedef Leading<4> RawUtf8;
template <int MARKER_BITS, codePoint L = std::numeric_limits<codePoint>::max()>
using Reader =
    org::simple::charEncode::DecodingReader<MARKER_BITS, byte, codePoint>;
using ReaderState = org::simple::charEncode::DecodingReaderState;

template <typename number, bool separators = true>
static const char *binary(number num) {
  return org::simple::core::bits::renderBits<number, '_', separators ? 4 : 0>(num);
}

static const std::vector<long long unsigned> &generatePatterns() {
  static const unsigned hexDigits[] = {0x0, 0x1, 0x02, 0x03, 0x05, 0x07, 0xb};
  static constexpr int count = sizeof(hexDigits) / sizeof(unsigned);
  static std::vector<long long unsigned> results;
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < count; j++) {
      if (i != j) {
        unsigned value = hexDigits[i] | unsigned(hexDigits[j] << 4);
        long long unsigned result = 0;
        for (size_t k = 0; k < sizeof(long long unsigned); k++) {
          result <<= 8;
          result |= value;
        }
        results.push_back(result);
      }
    }
  }
  return results;
}

static const std::vector<long long unsigned> &patterns() {
  static const std::vector<long long unsigned> p = generatePatterns();
  return p;
}

BOOST_AUTO_TEST_SUITE(org_simple_util_CharEncode_Tests)

template <int bits>
static void checkMarkerValues(Marker<1>::byte marker,
                              Marker<1>::byte maskMarker,
                              Marker<1>::byte maskValue, int valueBits) {
  Marker<bits> m1;

  BOOST_CHECK_EQUAL(m1.markerBits, bits);
  BOOST_CHECK_EQUAL(m1.marker, marker);
  BOOST_CHECK_EQUAL(m1.maskMarker, maskMarker);
  BOOST_CHECK_EQUAL(m1.maskValue, maskValue);
  BOOST_CHECK_EQUAL(m1.valueBits, valueBits);
}

BOOST_AUTO_TEST_CASE(testMarkerValue) {

  checkMarkerValues<1>(0, 0x80, 0x7f, 7);
  checkMarkerValues<2>(0x80, 0xc0, 0x3f, 6);
  checkMarkerValues<3>(0xc0, 0xe0, 0x1f, 5);
  checkMarkerValues<4>(0xe0, 0xf0, 0x0f, 4);
  checkMarkerValues<5>(0xf0, 0xf8, 0x07, 3);
  checkMarkerValues<6>(0xf8, 0xfc, 0x03, 2);
  checkMarkerValues<7>(0xfc, 0xfe, 0x01, 1);
}

template <int bits> void testMarkerPackAndCorrectRetrieval() {
  Marker<bits> marker;
  int iterations = 0;
  for (char byte = 0; marker.valueFrom(byte) == byte; byte++, iterations++) {
    char packed = marker.pack(byte);
    BOOST_CHECK(marker.is(packed));
    char unpacked = marker.valueFrom(packed);
    BOOST_CHECK_EQUAL(unpacked, byte);
  }
  BOOST_CHECK(iterations > 0);
}

BOOST_AUTO_TEST_CASE(testMarkerPackValidAndCorrectValue) {
  testMarkerPackAndCorrectRetrieval<1>();
  testMarkerPackAndCorrectRetrieval<2>();
  testMarkerPackAndCorrectRetrieval<3>();
  testMarkerPackAndCorrectRetrieval<4>();
  testMarkerPackAndCorrectRetrieval<5>();
  testMarkerPackAndCorrectRetrieval<6>();
  testMarkerPackAndCorrectRetrieval<7>();
}

BOOST_AUTO_TEST_CASE(testMarkersUsedByLeadingAndContinuationBytes) {
  BOOST_CHECK_EQUAL(Marker<1>::markerBits, Leading<1>::markerBits);
  BOOST_CHECK_EQUAL(Marker<2>::markerBits, Continuation::markerBits);
  BOOST_CHECK_EQUAL(Marker<3>::markerBits, Leading<2>::markerBits);
  BOOST_CHECK_EQUAL(Marker<4>::markerBits, Leading<3>::markerBits);
  BOOST_CHECK_EQUAL(Marker<5>::markerBits, Leading<4>::markerBits);
  BOOST_CHECK_EQUAL(Marker<6>::markerBits, Leading<5>::markerBits);
  BOOST_CHECK_EQUAL(Marker<7>::markerBits, Leading<6>::markerBits);
}

BOOST_AUTO_TEST_CASE(testLeadinMinimaAndMaxima) {
  BOOST_CHECK_EQUAL(0, Leading<1>::minimumCodePoint);
  BOOST_CHECK_EQUAL(0x7f, Leading<1>::maximumCodePoint);
  BOOST_CHECK_EQUAL(0x80, Leading<2>::minimumCodePoint);
  BOOST_CHECK_EQUAL(0x7ff, Leading<2>::maximumCodePoint);
  BOOST_CHECK_EQUAL(0x800, Leading<3>::minimumCodePoint);
  BOOST_CHECK_EQUAL(0xffff, Leading<3>::maximumCodePoint);
  BOOST_CHECK_EQUAL(0x10000, Leading<4>::minimumCodePoint);
  BOOST_CHECK_EQUAL(0x1fffff, Leading<4>::maximumCodePoint);
  BOOST_CHECK_EQUAL(0x200000, Leading<5>::minimumCodePoint);
  BOOST_CHECK_EQUAL(0x3ffffff, Leading<5>::maximumCodePoint);
  BOOST_CHECK_EQUAL(0x4000000, Leading<6>::minimumCodePoint);
  BOOST_CHECK_EQUAL(0x7fffffff, Leading<6>::maximumCodePoint);
}

template <short Bytes, short DecodeBytes>
void checkScatterForCharacterAndBack(const char32_t c) {
  static constexpr bool sameBytes = Bytes == DecodeBytes;
  byte buffer[6] = {0, 0, 0, 0, 0, 0};

  BOOST_CHECK_EQUAL(Bytes, Leading<Bytes>::unsafeFixedLengthEncode(c, buffer) -
                               buffer);
  if constexpr (sameBytes) {
    codePoint fixedDecodeResult;
    BOOST_CHECK_EQUAL(Bytes, Leading<Bytes>::unsafeFixedLengthDecode(
                                 buffer, fixedDecodeResult) -
                                 buffer);
    BOOST_CHECK_EQUAL(c, fixedDecodeResult);
    codePoint decodeResultSameBytes;
    BOOST_CHECK_EQUAL(
        Bytes,
        Leading<Bytes>::unsafeDecode(buffer, decodeResultSameBytes) - buffer);
    BOOST_CHECK_EQUAL(c, decodeResultSameBytes);
  } else {
    codePoint decodeResultDifferentBytes;
    BOOST_CHECK_EQUAL(Bytes, Leading<DecodeBytes>::unsafeDecode(
                                 buffer, decodeResultDifferentBytes) -
                                 buffer);
    BOOST_CHECK_EQUAL(c, decodeResultDifferentBytes);
  }
}

template <short Bytes, short DecodeBytes = Bytes>
void checkScatterForCharacters() {

  static constexpr codePoint min = Leading<Bytes>::minimumCodePoint;
  static constexpr codePoint max = Leading<Bytes>::maximumCodePoint;

  BOOST_CHECK_EQUAL(Leading<Bytes>::encodedBytes, Bytes);
  checkScatterForCharacterAndBack<Bytes, DecodeBytes>(min);
  checkScatterForCharacterAndBack<Bytes, DecodeBytes>(min + 1);
  checkScatterForCharacterAndBack<Bytes, DecodeBytes>(max - 1);

  for (long long unsigned x : patterns()) {
    const codePoint pattern = x & ~codePoint(0);
    for (codePoint c = pattern; c != 0; c >>= 1) {
      if (c >= min && c <= max) {
        checkScatterForCharacterAndBack<Bytes, DecodeBytes>(c);
      }
    }
    for (codePoint c = pattern; c != 0; c <<= 1) {
      if (c >= min && c <= max) {
        checkScatterForCharacterAndBack<Bytes, DecodeBytes>(c);
      }
    }
  }
}

template <short Bytes, short EntryMarker>
void checkNumberOfBytesDeducted(const char32_t c) {
  BOOST_CHECK_EQUAL(Bytes, Leading<EntryMarker>::getBytesFromCodePoint(c));
}

template <short Bytes, short EntryMarker = Bytes>
void checkNumberOfBytesDeducted() {
  static constexpr codePoint min = Leading<Bytes>::minimumCodePoint;
  static constexpr codePoint max = Leading<Bytes>::maximumCodePoint;

  BOOST_CHECK_EQUAL(Leading<Bytes>::encodedBytes, Bytes);
  checkNumberOfBytesDeducted<Bytes, EntryMarker>(min);
  checkNumberOfBytesDeducted<Bytes, EntryMarker>(min + 1);
  checkNumberOfBytesDeducted<Bytes, EntryMarker>(max - 1);
  for (long long unsigned x : patterns()) {
    const codePoint pattern = x & ~codePoint(0);
    for (codePoint c = pattern; c != 0; c >>= 1) {
      if (c >= min && c <= max) {
        checkNumberOfBytesDeducted<Bytes, EntryMarker>(c);
      }
    }
    for (codePoint c = pattern; c != 0; c <<= 1) {
      if (c >= min && c <= max) {
        checkNumberOfBytesDeducted<Bytes, EntryMarker>(c);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testLeaderMarkerEncodeAndDecode) {
  checkScatterForCharacters<1>();

  checkScatterForCharacters<1, 2>();
  checkScatterForCharacters<2>();

  checkScatterForCharacters<1, 3>();
  checkScatterForCharacters<2, 3>();
  checkScatterForCharacters<3>();

  checkScatterForCharacters<1, 4>();
  checkScatterForCharacters<2, 4>();
  checkScatterForCharacters<3, 4>();
  checkScatterForCharacters<4>();

  checkScatterForCharacters<1, 5>();
  checkScatterForCharacters<2, 5>();
  checkScatterForCharacters<3, 5>();
  checkScatterForCharacters<4, 5>();
  checkScatterForCharacters<5>();

  checkScatterForCharacters<1, 6>();
  checkScatterForCharacters<2, 6>();
  checkScatterForCharacters<3, 6>();
  checkScatterForCharacters<4, 6>();
  checkScatterForCharacters<5, 6>();
  checkScatterForCharacters<6>();
}

BOOST_AUTO_TEST_CASE(testNumberOfDeductedBytes) {
  checkNumberOfBytesDeducted<1>();

  checkNumberOfBytesDeducted<1, 2>();
  checkNumberOfBytesDeducted<2>();

  checkNumberOfBytesDeducted<1, 3>();
  checkNumberOfBytesDeducted<2, 3>();
  checkNumberOfBytesDeducted<3>();

  checkNumberOfBytesDeducted<1, 4>();
  checkNumberOfBytesDeducted<2, 4>();
  checkNumberOfBytesDeducted<3, 4>();
  checkNumberOfBytesDeducted<4>();

  checkNumberOfBytesDeducted<1, 5>();
  checkNumberOfBytesDeducted<2, 5>();
  checkNumberOfBytesDeducted<3, 5>();
  checkNumberOfBytesDeducted<4, 5>();
  checkNumberOfBytesDeducted<5>();

  checkNumberOfBytesDeducted<1, 6>();
  checkNumberOfBytesDeducted<2, 6>();
  checkNumberOfBytesDeducted<3, 6>();
  checkNumberOfBytesDeducted<4, 6>();
  checkNumberOfBytesDeducted<5, 6>();
  checkNumberOfBytesDeducted<6>();
}

template <short Bytes>
static bool readCodePoint(const char *bytes, codePoint &result) {
  Reader<Bytes> reader;
  int i;
  for (i = 0; i < 6; i++) {
    switch (reader.addGetState(bytes[i])) {
    case ReaderState::READING:
      break;
    case ReaderState::OK:
      result = reader.getValueAndReset();
      return true;
    case ReaderState::INVALID:
      return false;
    }
  }
  return false;
}

template <short Bytes, short DecodeBytes>
void checkEncodeAndReadForCharacterAndBack(const char32_t c) {
  static constexpr bool sameBytes = Bytes == DecodeBytes;
  byte buffer[6] = {0, 0, 0, 0, 0, 0};

  BOOST_CHECK_EQUAL(Bytes, Leading<Bytes>::unsafeFixedLengthEncode(c, buffer) -
                               buffer);
  if constexpr (sameBytes) {
    codePoint fixedDecodeResult;
    BOOST_CHECK(readCodePoint<DecodeBytes>(
        reinterpret_cast<const char *>(buffer), fixedDecodeResult));
    BOOST_CHECK_EQUAL(c, fixedDecodeResult);
  }
}

template <short Bytes, short DecodeBytes = Bytes>
void checkEncodeAndReadForCharacters() {

  static constexpr codePoint min = Leading<Bytes>::minimumCodePoint;
  static constexpr codePoint max = Leading<Bytes>::maximumCodePoint;

  BOOST_CHECK_EQUAL(Leading<Bytes>::encodedBytes, Bytes);
  checkEncodeAndReadForCharacterAndBack<Bytes, DecodeBytes>(min);
  checkEncodeAndReadForCharacterAndBack<Bytes, DecodeBytes>(min + 1);
  checkEncodeAndReadForCharacterAndBack<Bytes, DecodeBytes>(max - 1);
  for (long long unsigned x : patterns()) {
    const codePoint pattern = x & ~codePoint(0);
    for (codePoint c = pattern; c != 0; c >>= 1) {
      if (c >= min && c <= max) {
        checkEncodeAndReadForCharacterAndBack<Bytes, DecodeBytes>(c);
      }
    }
    for (codePoint c = pattern; c != 0; c <<= 1) {
      if (c >= min && c <= max) {
        checkEncodeAndReadForCharacterAndBack<Bytes, DecodeBytes>(c);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(testEncodeAndRead) {
  checkEncodeAndReadForCharacters<1>();

  checkEncodeAndReadForCharacters<1, 2>();
  checkEncodeAndReadForCharacters<2>();

  checkEncodeAndReadForCharacters<1, 3>();
  checkEncodeAndReadForCharacters<2, 3>();
  checkEncodeAndReadForCharacters<3>();

  checkEncodeAndReadForCharacters<1, 4>();
  checkEncodeAndReadForCharacters<2, 4>();
  checkEncodeAndReadForCharacters<3, 4>();
  checkEncodeAndReadForCharacters<4>();

  checkEncodeAndReadForCharacters<1, 5>();
  checkEncodeAndReadForCharacters<2, 5>();
  checkEncodeAndReadForCharacters<3, 5>();
  checkEncodeAndReadForCharacters<4, 5>();
  checkEncodeAndReadForCharacters<5>();

  checkEncodeAndReadForCharacters<1, 6>();
  checkEncodeAndReadForCharacters<2, 6>();
  checkEncodeAndReadForCharacters<3, 6>();
  checkEncodeAndReadForCharacters<4, 6>();
  checkEncodeAndReadForCharacters<5, 6>();
  checkEncodeAndReadForCharacters<6>();
}

static void testUtf8CodePoint(byte *encoded, const codePoint cp) {
  typedef org::simple::charEncode::Utf8Encoding Encoding;
  byte *nextEncodePtr = Encoding::unsafeEncode(cp, encoded);
  BOOST_CHECK(nextEncodePtr != nullptr);
  ptrdiff_t bytesWritten = nextEncodePtr - encoded;
  BOOST_CHECK(bytesWritten >= 1 && bytesWritten <= Encoding::encodedBytes);
  codePoint decoded;
  const byte *nextDecodePtr = Encoding::unsafeDecode(encoded, decoded);
  BOOST_CHECK(nextDecodePtr != nullptr);
  ptrdiff_t bytesRead = nextDecodePtr - encoded;
  BOOST_CHECK_EQUAL(long(bytesWritten), long(bytesRead));
  BOOST_CHECK_EQUAL(cp, decoded);
}

BOOST_AUTO_TEST_CASE(testUtf8AllCodepoints) {
  typedef org::simple::charEncode::Utf8Encoding Encoding;
  byte encoded[Encoding::encodedBytes];

  for (long long unsigned x : patterns()) {
    const codePoint pattern = x & ~codePoint(0);
    codePoint min = Encoding::minimumCodePoint;
    codePoint max = Encoding::maximumCodePoint;
    for (codePoint c = pattern; c != 0; c >>= 1) {
      if (c >= min && c <= max) {
        testUtf8CodePoint(encoded, c);
      }
    }
    for (codePoint c = pattern; c != 0; c <<= 1) {
      if (c >= min && c <= max) {
        testUtf8CodePoint(encoded, c);
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
