//
// Created by michel on 05-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/core/Bits.h>
#include <org-simple/util/Characters.h>

namespace {

using Marker = org::simple::util::ByteEncoding::Marker;
using Continuation = org::simple::util::ByteEncoding::Continuation;
using Leading = org::simple::util::ByteEncoding::Leading;
using Utf8 = org::simple::util::Utf8;

using namespace org::simple::util::ByteEncoding;


template <short Bytes> void checkScatterForCharacterAndBack(char32_t c) {
  char buffer[5];
  BOOST_CHECK_EQUAL(Utf8::encodeBytes(c, buffer) - buffer, Bytes);
  char32_t result;
  BOOST_CHECK_EQUAL(Utf8::decodeBytes(buffer, result) - buffer, Bytes);
  BOOST_CHECK_EQUAL(c, result);
}

template <short Bytes> void checkScatterForCharacters() {
  static constexpr Leading l = leading[Bytes - 1];
  static constexpr char32_t min = l.minimumCodePoint;
  static constexpr char32_t max = std::min(l.maximumCodePoint, Utf8::MAX_UTF8_CODEPOINT);
  BOOST_CHECK_EQUAL(l.encodedBytes, Bytes);
  checkScatterForCharacterAndBack<Bytes>(min);
  checkScatterForCharacterAndBack<Bytes>(min);
  checkScatterForCharacterAndBack<Bytes>(
      (min + max) / 2);
  checkScatterForCharacterAndBack<Bytes>(max - 1);
  checkScatterForCharacterAndBack<Bytes>(max);
  char32_t c =
      org::simple::core::Bits<char32_t>::fill(min);
  while (c <= max) {
    checkScatterForCharacterAndBack<Bytes>(c);
    c <<= 1;
    c |= 1;
  }
}

} // anonymous namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_Characters)

BOOST_AUTO_TEST_CASE(testMarkerValue) {
  Marker m1 { 1 };

  BOOST_CHECK_EQUAL(m1.markerBits, 1);
  BOOST_CHECK_EQUAL(m1.marker, char(0));
  BOOST_CHECK_EQUAL(m1.maskMarker, char(0x80));
  BOOST_CHECK_EQUAL(m1.maskValue, char(0x7f));
  BOOST_CHECK_EQUAL(m1.valueBits, 7);

  Marker m2{2};
  BOOST_CHECK_EQUAL(m2.markerBits, 2);
  BOOST_CHECK_EQUAL(m2.marker, char(0x80));
  BOOST_CHECK_EQUAL(m2.maskMarker, char(0xc0));
  BOOST_CHECK_EQUAL(m2.maskValue, char(0x3f));
  BOOST_CHECK_EQUAL(m2.valueBits, 6);

  Marker m3{3};
  BOOST_CHECK_EQUAL(m3.markerBits, 3);
  BOOST_CHECK_EQUAL(m3.marker, char(0xc0));
  BOOST_CHECK_EQUAL(m3.maskMarker, char(0xe0));
  BOOST_CHECK_EQUAL(m3.maskValue, char(0x1f));
  BOOST_CHECK_EQUAL(m3.valueBits, 5);

  Marker m4{4};
  BOOST_CHECK_EQUAL(m4.markerBits, 4);
  BOOST_CHECK_EQUAL(m4.marker, char(0xe0));
  BOOST_CHECK_EQUAL(m4.maskMarker, char(0xf0));
  BOOST_CHECK_EQUAL(m4.maskValue, char(0x0f));
  BOOST_CHECK_EQUAL(m4.valueBits, 4);

  Marker m5{5};
  BOOST_CHECK_EQUAL(m5.markerBits, 5);
  BOOST_CHECK_EQUAL(m5.marker, char(0xf0));
  BOOST_CHECK_EQUAL(m5.maskMarker, char(0xf8));
  BOOST_CHECK_EQUAL(m5.maskValue, char(0x07));
  BOOST_CHECK_EQUAL(m5.valueBits, 3);

  Marker m6{6};
  BOOST_CHECK_EQUAL(m6.markerBits, 6);
  BOOST_CHECK_EQUAL(m6.marker, char(0xf8));
  BOOST_CHECK_EQUAL(m6.maskMarker, char(0xfc));
  BOOST_CHECK_EQUAL(m6.maskValue, char(0x03));
  BOOST_CHECK_EQUAL(m6.valueBits, 2);

  Marker m7{7};
  BOOST_CHECK_EQUAL(m7.markerBits, 7);
  BOOST_CHECK_EQUAL(m7.marker, char(0xfc));
  BOOST_CHECK_EQUAL(m7.maskMarker, char(0xfe));
  BOOST_CHECK_EQUAL(m7.maskValue, char(0x01));
  BOOST_CHECK_EQUAL(m7.valueBits, 1);
}

BOOST_AUTO_TEST_CASE(testMarkerPackValidAndCorrectValue) {
  for (int bits = 1; bits <= 7; bits++) {
    Marker marker {bits};
    for (char byte = 0; marker.valueFrom(byte) == byte ; byte++) {
      char packed = marker.pack(byte);
      BOOST_CHECK(marker.is(packed));
      char unpacked = marker.valueFrom(packed);
      BOOST_CHECK_EQUAL(unpacked, byte);
    }
  }
}

BOOST_AUTO_TEST_CASE(testMarkersUsedByLeadingAndContinuationBytes) {
  BOOST_CHECK_EQUAL(Marker{1}.markerBits, leading[0].markerBits);
  BOOST_CHECK_EQUAL(Marker{2}.markerBits, continuation.markerBits);
  BOOST_CHECK_EQUAL(Marker{3}.markerBits, leading[1].markerBits);
  BOOST_CHECK_EQUAL(Marker{4}.markerBits, leading[2].markerBits);
  BOOST_CHECK_EQUAL(Marker{5}.markerBits, leading[3].markerBits);
  BOOST_CHECK_EQUAL(Marker{6}.markerBits, leading[4].markerBits);
  BOOST_CHECK_EQUAL(Marker{7}.markerBits, leading[5].markerBits);
}

BOOST_AUTO_TEST_CASE(testLeadinMinimaAndMaxima) {
  BOOST_CHECK_EQUAL(0, leading[0].minimumCodePoint);
  BOOST_CHECK_EQUAL(0x7f, leading[0].maximumCodePoint);
  BOOST_CHECK_EQUAL(0x80, leading[1].minimumCodePoint);
  BOOST_CHECK_EQUAL(0x7ff, leading[1].maximumCodePoint);
  BOOST_CHECK_EQUAL(0x800, leading[2].minimumCodePoint);
  BOOST_CHECK_EQUAL(0xffff, leading[2].maximumCodePoint);
  BOOST_CHECK_EQUAL(0x10000, leading[3].minimumCodePoint);
  BOOST_CHECK_EQUAL(0x1fffff, leading[3].maximumCodePoint);
  BOOST_CHECK_EQUAL(0x200000, leading[4].minimumCodePoint);
  BOOST_CHECK_EQUAL(0x3ffffff, leading[4].maximumCodePoint);
  BOOST_CHECK_EQUAL(0x4000000, leading[5].minimumCodePoint);
  BOOST_CHECK_EQUAL(0x7fffffff, leading[5].maximumCodePoint);
}



BOOST_AUTO_TEST_CASE(testNumberOfBytesFromMarkers) {
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[0].marker), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[1].marker), 2);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[2].marker), 3);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[3].marker), 4);

  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[0].marker + 1), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[1].marker + 1), 2);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[2].marker + 1), 3);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[3].marker + 1), 4);

  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x40), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x20), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x10), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x08), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x04), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x02), 1);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(0x01), 1);
}

BOOST_AUTO_TEST_CASE(testNumberOfBytesFromInvalidMarkers) {
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(leading[0].marker | 0x80), 0);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(char(0xf80)), 0);
  BOOST_CHECK_EQUAL(Utf8::encodedBytes(char(0xf90)), 0);
}

BOOST_AUTO_TEST_CASE(testScatterUnicodeBytesAndReverse) {
  checkScatterForCharacters<1>();
  checkScatterForCharacters<2>();
  checkScatterForCharacters<3>();
  checkScatterForCharacters<4>();
}

BOOST_AUTO_TEST_SUITE_END()
