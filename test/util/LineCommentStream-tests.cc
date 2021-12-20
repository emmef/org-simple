//
// Created by michel on 20-12-21.
//

#include <org-simple/util/StringStream.h>
#include <org-simple/util/InputStream.h>
#include "boost-unit-tests.h"

BOOST_AUTO_TEST_SUITE(org_simple_util_LineCommentStream_Tests)

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentIdentical) {
  std::string string = "This is a text without a line-comment";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");
  
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentAtStartOfLine_NoLF) {
  std::string string = "#This is a text without a line-comment";
  std::string expected = "";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentAtStartOfLine_WithLFEnd) {
  std::string string = "#This is a text without a line-comment\n";
  std::string expected = "\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentAtStartOfLine_WithLFStartEnd) {
  std::string string = "\n#This is a text without a line-comment\n";
  std::string expected = "\n\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentInLine_WithoutLF) {
  std::string string = "This is a text with# a line-comment";
  std::string expected = "This is a text with";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentInLine_WithLFAtEnd) {
  std::string string = "This is a text with# a line-comment\n";
  std::string expected = "This is a text with\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentAndExtraLine) {
  std::string string = "This is a text with# a line-comment\n another line.";
  std::string expected = "This is a text with\n another line.";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testSingleCharacterCommentInLineQuoted) {
  std::string string = "This is a text 'with# a line-comment'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "#");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}


BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentIdentical) {
  std::string string = "This is a text without a line-comment";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");
  
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentAtStartOfLine_NoLF) {
  std::string string = "/*This is a text without a line-comment";
  std::string expected = "";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentAtStartOfLine_WithLFEnd) {
  std::string string = "/*This is a text without a line-comment\n";
  std::string expected = "\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentAtStartOfLine_WithLFStartEnd) {
  std::string string = "\n/*This is a text without a line-comment\n";
  std::string expected = "\n\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentInLine_WithoutLF) {
  std::string string = "This is a text with/* a line-comment";
  std::string expected = "This is a text with";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentInLine_WithLFAtEnd) {
  std::string string = "This is a text with/* a line-comment\n";
  std::string expected = "This is a text with\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentInLine_WithNextLine) {
  std::string string = "This is a text with/* a line-comment\n another line.";
  std::string expected = "This is a text with\n another line.";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentAtEnd) {
  std::string string = "This is a text/*";
  std::string expected = "This is a text";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentAtEndPartial) {
  std::string string = "This is a text/";
  std::string expected = "This is a text/";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentInLineQuoted) {
  std::string string = "This is a text 'with/* a line-comment'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterUnfinishedComment) {
  std::string string = "This is a text with a line-comment: /- lala";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterUnfinishedCommentAtTheEnd_wrongCharacter) {
  std::string string = "This is a text with a line-comment: /-";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterUnfinishedCommentAtTheEnd) {
  std::string string = "This is a text with a line-comment: /";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterCommentAtTheEnd_Long5_5) {
  std::string string = "This is a text with a line-comment: 12345";
  std::string expected = "This is a text with a line-comment: ";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterUnfinishedCommentAtTheEnd_Long5_4) {
  std::string string = "This is a text with a line-comment: 1234";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testDoubleCharacterUnfinishedCommentAtTheEnd_Long5_3) {
  std::string string = "This is a text with a line-comment: 123";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::LineCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));

  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_SUITE_END()
