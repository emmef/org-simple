//
// Created by michel on 20-12-21.
//

#include <org-simple/util/text/StringStream.h>
#include "boost-unit-tests.h"

using CStringInputStream = org::simple::util::text::CStringInputStream<char>;
using StringInputStream = org::simple::util::text::StringInputStream<char>;
using StringCollector = org::simple::util::text::InputCollector<char>;

namespace {

StringInputStream five("12345");
StringInputStream ten("1234567890");
StringInputStream fifteen("123456789012345");

}

BOOST_AUTO_TEST_SUITE(org_simple_util_StringStream_Tests)

BOOST_AUTO_TEST_CASE(testCStringInputStreamInitMatchesInputCString) {
  const char * cString = "12345";
  CStringInputStream stream(cString);
  BOOST_CHECK_EQUAL(stream.getCString(), cString);
  const char * ptr = cString;
  bool stringTerminated;
  do {
    char stringCharacter = *ptr;
    stringTerminated = stringCharacter == '\0';
    char streamCharacter;
    bool streamRead = stream.get(streamCharacter);
    BOOST_CHECK_EQUAL(stringTerminated, !streamRead);
    if (!stringTerminated) {
      BOOST_CHECK_EQUAL(stringCharacter, streamCharacter);
    }
    ptr++;
  }
  while (!stringTerminated);
}

BOOST_AUTO_TEST_CASE(testStringInputStreamInitMatchesInputCString) {
  std::basic_string<char> string("12345");
  StringInputStream stream(string);
  BOOST_CHECK_EQUAL(stream.getCString(), string.c_str());
  BOOST_CHECK_EQUAL(stream.getString(), string);
  size_t pos = 0;
  bool stringTerminated;
  do {
    stringTerminated = pos >= string.length();
    char streamCharacter;
    bool streamRead = stream.get(streamCharacter);
    BOOST_CHECK_EQUAL(stringTerminated, !streamRead);
    if (!stringTerminated) {
      char stringCharacter = string[pos];
      BOOST_CHECK_EQUAL(stringCharacter, streamCharacter);
    }
    pos++;
  }
  while (!stringTerminated);
}

BOOST_AUTO_TEST_CASE(testCStringReassignMatchesNewCString) {
  const char * cString = "12345";
  const char * newString = "098765431221";
  CStringInputStream stream(cString);
  BOOST_CHECK_EQUAL(stream.getCString(), cString);
  stream.set(newString);
  BOOST_CHECK_EQUAL(stream.getCString(), newString);
}

BOOST_AUTO_TEST_CASE(testStringReassignMatchesNewStrings) {
  const char * cString = "12345";
  const char * newString1 = "098765431221";
  std::basic_string<char> newString2 = "098765431221";
  StringInputStream stream(cString);
  BOOST_CHECK_EQUAL(stream.getCString(), cString);
  stream.set(newString1);
  BOOST_CHECK(stream.getString() == newString1);
  stream.set(newString2);
  BOOST_CHECK(stream.getString() == newString2);
}

BOOST_AUTO_TEST_CASE(testCStringInputStreamCollectorMatchesCString) {
  const char * cString = "12345";
  size_t length;
  for (length = 0; cString[length] != '\0'; length++);
  CStringInputStream stream(cString);
  StringCollector collector(length);
  BOOST_CHECK_EQUAL(length, collector.consume(stream));
  BOOST_CHECK(collector.getString() == stream.getCString());
  BOOST_CHECK_EQUAL(true, collector.isFull());
}

BOOST_AUTO_TEST_CASE(testStringInputStreamCollectorMatchesString) {
  std::basic_string<char> string = "12345";
  StringInputStream stream(string);
  StringCollector collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(true, collector.isFull());
  BOOST_CHECK(collector.getString() == stream.getString());
}

BOOST_AUTO_TEST_CASE(testStringInputStreamTooSmallCollectorFails) {
  std::basic_string<char> string = "12345678901234567890";
  StringInputStream stream(string);
  size_t limit = 10;
  StringCollector collector(limit);
  BOOST_CHECK_EQUAL(limit, collector.consume(stream));
  BOOST_CHECK(collector.getString() != stream.getString());
  BOOST_CHECK_EQUAL(true, collector.isFull());
  for (size_t i = 0; i < limit; i++) {
    BOOST_CHECK_EQUAL(string[i], collector.getString()[i]);
  }
}

BOOST_AUTO_TEST_CASE(testStringInputStreamCollectorReadTwice) {
  std::basic_string<char> string = "12345";
  size_t length = string.length();
  StringInputStream stream(string);
  StringCollector collector(length * 3);
  BOOST_CHECK_EQUAL(length, collector.consume(stream));
  stream.rewind();
  BOOST_CHECK_EQUAL(length, collector.consume(stream));
  BOOST_CHECK_EQUAL(length * 2, collector.getString().length());
  BOOST_CHECK_EQUAL(false, collector.isFull());
  for (size_t i = 0; i < length * 2; i++) {
    BOOST_CHECK_EQUAL(string[i % length], collector.getString()[i]);
  }
}

BOOST_AUTO_TEST_CASE(testCStringInputStreamCollectorReadTwice) {
  const char * string = "12345";
  size_t length;
  for (length = 0; string[length] != '\0'; length++);
  CStringInputStream stream(string);
  StringCollector collector(length * 3);
  BOOST_CHECK_EQUAL(length, collector.consume(stream));
  stream.rewind();
  BOOST_CHECK_EQUAL(length, collector.consume(stream));
  BOOST_CHECK_EQUAL(length * 2, collector.getString().length());
  BOOST_CHECK_EQUAL(false, collector.isFull());
  for (size_t i = 0; i < length * 2; i++) {
    BOOST_CHECK_EQUAL(string[i % length], collector.getString()[i]);
  }
}


BOOST_AUTO_TEST_SUITE_END()
