//
// Created by michel on 20-12-21.
//

#include <org-simple/util/StringStream.h>
#include <org-simple/util/InputStream.h>
#include "boost-unit-tests.h"

BOOST_AUTO_TEST_SUITE(org_simple_util_QuotedStateStream_Tests)

BOOST_AUTO_TEST_CASE(testNoQuotesIdentical) {
  std::string string = "This is a text without any quotes";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_NotAtEndOrStart) {
  std::string string = "This is a 'text with quotes'.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_AtEnd) {
  std::string string = "This is a 'text with quotes.'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_AtStart) {
  std::string string = "'This is a text', with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_AtStartAndEnd) {
  std::string string = "'This is a text with quotes.'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedQuote) {
  std::string string = "This is a 'text with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\'', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedQuoteEscaped) {
  std::string string = "This is a 'text with quotes.\\";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
  BOOST_CHECK_EQUAL('\'', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedQuoteEscapedQuote) {
  std::string string = "This is a 'text with quotes.\\'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\'', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_NotAtEndOrStart) {
  std::string string = "This is a 'text with quotes'.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_AtEnd) {
  std::string string = "This is a 'text with quotes.'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_AtStart) {
  std::string string = "'This is a text', with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_AtStartAndEnd) {
  std::string string = "'This is a text with quotes.'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedSecondQuote) {
  std::string string = "This is a 'text with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\'', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedSecondQuoteEscaped) {
  std::string string = "This is a 'text with quotes.\\";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
  BOOST_CHECK_EQUAL('\'', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedSecondQuoteEscapedSecondQuote) {
  std::string string = "This is a 'text with quotes.\\'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "\"'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\'', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testEscaped) {
  std::string string = "This is escaped:\\";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testEscapedQuote) {
  std::string string = "This is escaped:\\'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedQuotes_StartEnd) {
  std::string string = "'This is Peter\\'s text with quotes.'";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedQuotes) {
  std::string string = "This is 'Peter\\'s text' with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingSecondQuote) {
  std::string string = "This is 'Peter 9\" text' with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'\"");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedSecondQuote) {
  std::string string = "This is 'Peter 9\\\" text' with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'\"");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingFirstQuote) {
  std::string string = "This is \"Peter 9' text\" with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'\"");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedFirstQuote) {
  std::string string = "This is \"Peter 9\\' text\" with quotes.";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::QuotedStateStream<char> stream(input, "'\"");
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL('\0', stream.inQuote());
}

BOOST_AUTO_TEST_SUITE_END()