//
// Created by michel on 20-12-21.
//

#include "org-simple/util/text/StringStream.h"
#include "org-simple/util/text/UnixNewLine.h"
#include "boost-unit-tests.h"


BOOST_AUTO_TEST_SUITE(org_simple_util_UnixNewLineStream_Tests)

BOOST_AUTO_TEST_CASE(testUnixNewLineToUnix_endsWithNewLine) {
  std::string string = "This text\nUses\n\nUnix-newlines!\n";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testUnixNewLineToUnix_startsWithNewLine) {
  std::string string = "\nThis text\nUses\n\nUnix-newlines!";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testUnixNewLineToUnix_endsWithNoNewLine) {
  std::string string = "This text\nUses\n\nUnix-newlines!";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRLFNewLineToUnix_endsWithNewLine) {
  std::string string = "This text\r\nUses\r\n\r\nUnix-newlines!\r\n";
  std::string expected = "This text\nUses\n\nUnix-newlines!\n";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}


BOOST_AUTO_TEST_CASE(testCRLFNewLineToUnix_startsWithNewLine) {
  std::string string = "\r\nThis text\r\nUses\r\n\r\nUnix-newlines!";
  std::string expected = "\nThis text\nUses\n\nUnix-newlines!";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRLFNewLineToUnix_endsWithNoNewLine) {
  std::string string = "This text\r\nUses\r\n\r\nUnix-newlines!";
  std::string expected = "This text\nUses\n\nUnix-newlines!";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRNewLineToUnix_endsWithNewLine) {
  std::string string = "This text\rUses\r\rUnix-newlines!\r";
  std::string expected = "This text\nUses\n\nUnix-newlines!\n";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}


BOOST_AUTO_TEST_CASE(testCRNewLineToUnix_startsWithNewLine) {
  std::string string = "\rThis text\rUses\r\rUnix-newlines!";
  std::string expected = "\nThis text\nUses\n\nUnix-newlines!";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testCRNewLineToUnix_endsWithNoNewLine) {
  std::string string = "This text\rUses\r\rUnix-newlines!";
  std::string expected = "This text\nUses\n\nUnix-newlines!";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::UnixNewLineStream<char> stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_SUITE_END()
