//
// Created by michel on 21-12-21.
//

#include <org-simple/util/StringStream.h>
#include <org-simple/util/TextFilters.h>
#include "boost-unit-tests.h"

BOOST_AUTO_TEST_SUITE(org_simple_util_BlockAndLineComment_Tests)

/// Repeat of same tests, but now with nestingLevels

BOOST_AUTO_TEST_CASE(testNoCommentIdentical) {
  std::string string = "This is a text without a block-comment";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::BlockCommentStream<char> stream1(input, "/*", 1u);
  org::simple::util::LineCommentStream<char> stream(stream1, "//");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testLineInBlockComment_Normal) {
  std::string string = "This is a text with a block-comment:\n"
                       "/*"
                       " An here we have a line-comment too: // this is ignored";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::util::BlockCommentStream<char> stream1(input, "/*", 1u);
  org::simple::util::LineCommentStream<char> stream(stream1, "//");

  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}
/*
// This is another /* Thins */

BOOST_AUTO_TEST_SUITE_END()

