//
// Created by michel on 01-01-22.
//

#include <org-simple/util/text/EchoStream.h>
#include <org-simple/util/text/StringStream.h>
#include "boost-unit-tests.h"

using namespace org::simple::util::text;

using StringStream = StringInputStream<char>;

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_EchoStream)

BOOST_AUTO_TEST_CASE(testIdenticalWithUnderlyingStream) {
  StringStream string = "Hello world!";
  std::string expected;
  char c;
  while (string.get(c)) {
    expected += c;
  }

  EchoStream<char, StringStream> echo(&string);
  std::string actual;
  string.rewind();
  while (echo.get(c)) {
    actual += c;
  }

  BOOST_CHECK_EQUAL(expected, actual);
}

BOOST_AUTO_TEST_CASE(testPeekreturnsLastGot) {
  StringStream string = "Hello world!";
  EchoStream<char, StringStream> echo(&string);
  char c;
  while (echo.get(c)) {
    BOOST_CHECK_EQUAL(c, echo.peek());
  }
}

BOOST_AUTO_TEST_CASE(testRepeat) {
  StringStream string = "Hello world!";
  std::string expected = "Hellllo worlld!";
  EchoStream<char, StringStream> echo(&string);
  std::string actual;
  char c;
  while (echo.get(c)) {
    if (c == 'l') {
      echo.repeat();
    }
    actual += c;
  }

  BOOST_CHECK_EQUAL(expected, actual);
}

BOOST_AUTO_TEST_CASE(testEmpty) {
  StringStream string = "Hello world!";
  std::string expected = "Hello";
  EchoStream<char, StringStream> echo(&string);
  std::string actual;
  char c;
  while (echo.get(c)) {
    if (c == 'o') {
      echo = nullptr;
    }
    actual += c;
  }

  BOOST_CHECK_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_SUITE_END()
