//
// Created by michel on 05-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/util/Characters.h>
#include <org-simple/core/Bits.h>

namespace {

using Utf8 = org::simple::util::Utf8;
using ExtensionByte = org::simple::util::Utf8::ExtensionByte;
template <short BYTES> using Range = org::simple::util::Utf8::Range<BYTES>;

template <short Bytes>
void checkRangeConstants(short bits, char marker, char markerMask,
                         char32_t firstCodePoint, char32_t lastCodePoint) {
  using R = Range<Bytes>;
  BOOST_CHECK_EQUAL(Range<Bytes>::BYTES, Bytes);
  BOOST_CHECK_EQUAL(R::BITS, bits);
  BOOST_CHECK_EQUAL(R::MARKER, marker);
  BOOST_CHECK_EQUAL(R::MARKER_MASK, markerMask);
  BOOST_CHECK_EQUAL(R::VALUE_MASK, ~markerMask);
  BOOST_CHECK_EQUAL(R::FIRST_CODEPOINT, firstCodePoint);
  BOOST_CHECK_EQUAL(R::LAST_CODEPOINT, lastCodePoint);
}

template<short Bytes>
void checkBytesForCharacters() {
  BOOST_CHECK_EQUAL(Utf8::getBytesForCharacter(Range<Bytes>::FIRST_CODEPOINT), Bytes);
  BOOST_CHECK_EQUAL(Utf8::getBytesForCharacter((Range<Bytes>::LAST_CODEPOINT + Range<Bytes>::FIRST_CODEPOINT)/2), Bytes);
  BOOST_CHECK_EQUAL(Utf8::getBytesForCharacter(Range<Bytes>::LAST_CODEPOINT - 1), Bytes);
  BOOST_CHECK_EQUAL(Utf8::getBytesForCharacter(Range<Bytes>::LAST_CODEPOINT), Bytes);
}

template<short Bytes>
void checkScatterForCharacterAndBack(char32_t c) {
  char buffer[5];
  const char *next;
  BOOST_CHECK_EQUAL(Utf8::unsafeScatter(c, buffer), buffer + Bytes);
  BOOST_CHECK_EQUAL(Utf8::Reader::readGextNext(buffer, &next), c);
  BOOST_CHECK_EQUAL(next - buffer, Bytes);
}

template<short Bytes>
void checkScatterForCharacters() {
  checkScatterForCharacterAndBack<Bytes>(Range<Bytes>::FIRST_CODEPOINT);
  checkScatterForCharacterAndBack<Bytes>(Range<Bytes>::FIRST_CODEPOINT + 1);
  checkScatterForCharacterAndBack<Bytes>((Range<Bytes>::FIRST_CODEPOINT + Range<Bytes>::LAST_CODEPOINT)/2);
  checkScatterForCharacterAndBack<Bytes>(Range<Bytes>::LAST_CODEPOINT - 1);
  checkScatterForCharacterAndBack<Bytes>(Range<Bytes>::LAST_CODEPOINT);
  char32_t c = org::simple::core::Bits<char32_t>::fill(Range<Bytes>::FIRST_CODEPOINT);
  while (c <= Range<Bytes>::LAST_CODEPOINT) {
    checkScatterForCharacterAndBack<Bytes>(c);
    c <<= 1;
    c |= 1;
  }
}

} // anonymous namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_Characters)

BOOST_AUTO_TEST_CASE(testConstantsOfUtf8ExtensionByteConstants) {
  BOOST_CHECK_EQUAL(ExtensionByte::MARKER, char(0x80));
  BOOST_CHECK_EQUAL(ExtensionByte::MARKER_MASK, char(0xc0));
  BOOST_CHECK_EQUAL(ExtensionByte::VALUE_MASK, char(0x3f));
  BOOST_CHECK_EQUAL(ExtensionByte::BITS, 6);
}

BOOST_AUTO_TEST_CASE(testConstantsOfUtsf8Range_1Byte) {
  checkRangeConstants<1>(7, 0, 0x80, 0, 0x7f);
}

BOOST_AUTO_TEST_CASE(testConstantsOfUtsf8Range_2Bytes) {
  checkRangeConstants<2>(11, 0xc0, 0xe0, 0x80, 0x7ff);
}

BOOST_AUTO_TEST_CASE(testConstantsOfUtsf8Range_3Bytes) {
  checkRangeConstants<3>(16, 0xe0, 0xf0, 0x800, 0xffff);
}

BOOST_AUTO_TEST_CASE(testConstantsOfUtsf8Range_4Bytes) {
  checkRangeConstants<4>(21, 0xf0, 0xf8, 0x10000, 0x10ffff);
}

BOOST_AUTO_TEST_CASE(testNumberOfBytesFromMarkers) {
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<1>::MARKER), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<2>::MARKER), 2);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<3>::MARKER), 3);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<4>::MARKER), 4);

  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<1>::MARKER + 1), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<2>::MARKER + 1), 2);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<3>::MARKER + 1), 3);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<4>::MARKER + 1), 4);

  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x40), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x20), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x10), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x08), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x04), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x02), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(0x01), 1);
}

BOOST_AUTO_TEST_CASE(testNumberOfBytesFromInvalidMarkers) {
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(Range<1>::MARKER | 0x80), 0);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(char(0xf80)), 0);
  BOOST_CHECK_EQUAL(Utf8::getBytesFromMarker(char(0xf90)), 0);
}

BOOST_AUTO_TEST_CASE(testNumberOfBytesForCharacter) {
  checkBytesForCharacters<1>();
  checkBytesForCharacters<2>();
  checkBytesForCharacters<3>();
  checkBytesForCharacters<4>();

  BOOST_CHECK_EQUAL(Utf8::getBytesForCharacter(0), 1);
  BOOST_CHECK_EQUAL(Utf8::getBytesForCharacter(Range<4>::LAST_CODEPOINT + 1), 0);
}

BOOST_AUTO_TEST_CASE(testScatterUnicodeBytesAndReverse) {
  checkScatterForCharacters<1>();
  checkScatterForCharacters<2>();
  checkScatterForCharacters<3>();
  checkScatterForCharacters<4>();
}

BOOST_AUTO_TEST_SUITE_END()
