//
// Created by michel on 20-12-21.
//

#include <org-simple/util/StringStream.h>
#include <org-simple/util/InputStream.h>
#include "boost-unit-tests.h"


BOOST_AUTO_TEST_SUITE(org_simple_util_PosixNewlineStream_Tests)

BOOST_AUTO_TEST_CASE(testPosixNewlineToPosix_endsWithNewLine) {
  std::string string = "This text\nUses\n\nPosix-newlines!\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testPosixNewlineToPosix_startsWithNewLine) {
  std::string string = "\nThis text\nUses\n\nPosix-newlines!";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testPosixNewlineToPosix_endsWithNoNewLine) {
  std::string string = "This text\nUses\n\nPosix-newlines!";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRLFNewlineToPosix_endsWithNewLine) {
  std::string string = "This text\r\nUses\r\n\r\nPosix-newlines!\r\n";
  std::string expected = "This text\nUses\n\nPosix-newlines!\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}


BOOST_AUTO_TEST_CASE(testCRLFNewlineToPosix_startsWithNewLine) {
  std::string string = "\r\nThis text\r\nUses\r\n\r\nPosix-newlines!";
  std::string expected = "\nThis text\nUses\n\nPosix-newlines!";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRLFNewlineToPosix_endsWithNoNewLine) {
  std::string string = "This text\r\nUses\r\n\r\nPosix-newlines!";
  std::string expected = "This text\nUses\n\nPosix-newlines!";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRNewlineToPosix_endsWithNewLine) {
  std::string string = "This text\rUses\r\rPosix-newlines!\r";
  std::string expected = "This text\nUses\n\nPosix-newlines!\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}


BOOST_AUTO_TEST_CASE(testCRNewlineToPosix_startsWithNewLine) {
  std::string string = "\rThis text\rUses\r\rPosix-newlines!";
  std::string expected = "\nThis text\nUses\n\nPosix-newlines!";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRNewlineToPosix_endsWithNoNewLine) {
  std::string string = "This text\rUses\r\rPosix-newlines!";
  std::string expected = "This text\nUses\n\nPosix-newlines!";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::PosixNewlineStream<char> stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_SUITE_END()
