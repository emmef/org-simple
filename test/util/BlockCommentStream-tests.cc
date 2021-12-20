//
// Created by michel on 20-12-21.
//

#include <org-simple/util/StringStream.h>
#include <org-simple/util/InputStream.h>
#include "boost-unit-tests.h"

BOOST_AUTO_TEST_SUITE(org_simple_util_BlockCommentStream_Tests)

BOOST_AUTO_TEST_CASE(testNoCommentIdentical) {
  std::string string = "This is a text without a block-comment";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");
  
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentOmitted) {
  std::string string = "/*This is a text with a block-comment*/";
  std::string expected = "";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentOmittedNewLines) {
  std::string string = "\n/*This is a text with a block-comment*/\n";
  std::string expected = "\n\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testUnfinishedComment) {
  std::string string = "\n/*This is a text with a block-comment*\n";
  std::string expected = "\n";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentOmittedNewLinesInside) {
  std::string string = "/*\nThis is a text with a block-comment\n*/";
  std::string expected = "";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentHalfwayOmittedNewLinesInside) {
  std::string string = "This is a text/* with a block-comment\n*/";
  std::string expected = "This is a text";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentUnclosed) {
  std::string string = "/*This is a text with an unclosed block-comment";
  std::string expected = "";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentUnclosedNewLines) {
  std::string string = "/*This is a text with an unclosed block-comment.\nPitty...\n";
  std::string expected = "";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}


BOOST_AUTO_TEST_CASE(testCommentStartedInQuotes) {
  std::string string = "This is 'a text/* with an ineffective block-comment'\n*/";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentEndedInQuotes) {
  std::string string = "This is a text /*with a block-comment 'ended in quotes */'";
  std::string expected = "This is a text ";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "/*");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLong) {
  std::string string = "This text12345 has no quality!!54321...";
  std::string expected = "This text...";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_4) {
  std::string string = "This text 12345 has no quality!!5432...";
  std::string expected = "This text ";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_3) {
  std::string string = "This text 12345 has no quality!!543...";
  std::string expected = "This text ";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_4_EOF) {
  std::string string = "This text 12345 has no quality!!5432";
  std::string expected = "This text ";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_3_EOF) {
  std::string string = "This text 12345 has no quality!!543";
  std::string expected = "This text ";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> quoted(input, "'");
  org::simple::util::BlockCommentStream<char> stream(quoted, "12345");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}


BOOST_AUTO_TEST_SUITE_END()
