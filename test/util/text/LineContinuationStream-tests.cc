//
// Created by michel on 20-12-21.
//

#include "org-simple/text/StringStream.h"
#include "org-simple/text/LineContinuation.h"
#include "boost-unit-tests.h"

BOOST_AUTO_TEST_SUITE(org_simple_util_LineContinuationStream_Tests)

BOOST_AUTO_TEST_CASE(testInputWithoutContinuationIsSame) {
  std::string string = "\nThis text\nUses\n\nPosix-newlines!\n";
  std::string expected = string;
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithFaultyContinuationIsSame) {
  std::string string = "\nThis text\nUses\\ \n\nPosix-newlines!\n";
  std::string expected = string;
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithCorrectContinuation) {
  std::string string = "This text\nUses\\\nPosix-newlines!\n";
  std::string expected = "This text\nUsesPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithContinuationAtEnd) {
  std::string string = "This text\nUses\nPosix-newlines!\n\\";
  std::string expected = "This text\nUses\nPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithContinuationAndNewLineAtEnd) {
  std::string string = "This text\nUses\nPosix-newlines!\n\\\n";
  std::string expected = "This text\nUses\nPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithMultipleContinuations) {
  std::string string = "This text\nUses\\\n\\\nPosix-newlines!\n";
  std::string expected = "This text\nUsesPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithMultipleContinuationsAndWhiteSpace) {
  std::string string = "This text\nUses\\\n\t\\\nPosix-newlines!\n";
  std::string expected = "This text\nUses\tPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithMultipleContinuationsAndOther1) {
  std::string string = "This text\nUses\\\n\tO\t\\\nPosix-newlines!\n";
  std::string expected = "This text\nUses\tO\tPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInputWithMultipleContinuationsAndOther2) {
  std::string string = "This text\nUses\\\n\tO\\\t\nPosix-newlines!\n";
  std::string expected = "This text\nUses\tO\\\t\nPosix-newlines!\n";
  org::simple::text::StringInputStream<char> stringStream(string);
  org::simple::text::LineContinuationStream<char> stream(stringStream);
  org::simple::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_SUITE_END()
