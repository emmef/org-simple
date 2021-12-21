//
// Created by michel on 21-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/util/StringStream.h>
#include <org-simple/util/TextFilters.h>

namespace {
static constexpr const char *singleLineComment = "//";
static constexpr const char *blockComment = "/*";
static constexpr const char *symmetricQuotes = "\"'";

class CommentStream : public org::simple::util::QuoteAndEscapeState<char> {
  org::simple::util::StringInputStream<char> input;
  org::simple::util::InputCollector<char> collector;
  org::simple::util::CommentStream<char> stream;
  size_t l;

public:
  CommentStream(const std::string string, unsigned nesting = 0,
                const char *blockPhrase = nullptr)
      : input(string), collector(string.length()),
        stream(input, singleLineComment,
               blockPhrase ? blockPhrase : blockComment, nesting,
               symmetricQuotes),
        l(nesting) {
    collect();
  }

  const std::string &getInput() const { return input.getString(); }
  const std::string &getOutput() const { return collector.getString(); }

  size_t length() const { return l; }

  void collect() {
    input.rewind();
    collector.reset();
    l = collector.consume(stream);
  }

  bool inQuote() const override { return stream.inQuote(); }
  char getOpenQuote() const override { return stream.getOpenQuote(); }
  char getCloseQuote() const override { return stream.getCloseQuote(); }
  bool isEscaped() const override { return stream.isEscaped(); }
  unsigned getLevel() const { return stream.getLevel(); }
  bool inComment() const { return stream.inComment(); }
};
} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_BlockAndLineComment_Tests)

BOOST_AUTO_TEST_CASE(testNoCommentIdentical) {
  CommentStream stream("This is a text without a block-comment");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testNoCommentWithQuoteIdentical) {
  CommentStream stream("This is a text 'without' a block-comment");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testNoCommentWithQuoteUnclosedIdentical) {
  CommentStream stream("This is a text 'without a block-comment");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testNoCommentWithQuoteEscapedClosedIdentical) {
  CommentStream stream("This is a text 'without\\' a block-comment");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testNoCommentWithQuoteUnclosedEscapeAtEndIdentical) {
  CommentStream stream("This is a text 'without a block-comment\\");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentInQuoteIdentical) {
  CommentStream stream("This is a text 'without //' a block-comment");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentInQuoteIdentical) {
  CommentStream stream("This is a text 'without /*' a block-comment");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentBothCharsEscapedIdentical) {
  CommentStream stream("This is a text.\\/\\/");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentEscapedOmitted) {
  CommentStream stream("This is a text.\\//");
  std::string expected = "This is a text.\\";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentNoNewLineOmitted) {
  CommentStream stream("This is a text.// The rest is history");
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentNewLineOmitted) {
  CommentStream stream("This is a text.// The rest is history\n");
  std::string expected = "This is a text.\n";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentWithBlockCommentIgnored) {
  CommentStream stream("This is a text.// The rest is /*history");
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentWithBlockCommentCloseIgnored) {
  CommentStream stream("This is a text.// The rest is */history");
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentWithBlockCommentIgnoredNewLine) {
  CommentStream stream("This is a text.// The rest is /*history\n");
  std::string expected = "This is a text.\n";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testLineCommentWithBlockCommentCloseIgnoredNewLine) {
  CommentStream stream("This is a text.// The rest is */history\n");
  std::string expected = "This is a text.\n";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentSingleLineOmitted) {
  CommentStream stream("This is a text./* The rest is history.*/");
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentSingleLineOmittedNewLine) {
  CommentStream stream("This is a text./* The rest is history.*/\n");
  std::string expected = "This is a text.\n";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentMultiLineOmittedNewLine) {
  CommentStream stream("This is a text./* The rest is history.\n"
                       "- which includes this;\n"
                       "*/\n");
  std::string expected = "This is a text.\n";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentMultiLineOmitted) {
  CommentStream stream("This is a text./* The rest is history.\n"
                       "- which includes this;\n"
                       "*/");
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentMultiLineWithLineCommentOmittedNewLine) {
  CommentStream stream("This is a text./* The rest is history.\n"
                       "- which includes this; // even though only just!\n"
                       "*/\n");
  std::string expected = "This is a text.\n";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testBlockCommentMultiLineWithLineCommentOmitted) {
  CommentStream stream("This is a text./* The rest is history.\n"
                       "- which includes this; // even though only just!\n"
                       "*/");
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
}

BOOST_AUTO_TEST_CASE(testSingleNestedComment) {
  CommentStream stream(
      "This is a text/* with a block comment/* with a block comment */*/.", 1);
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(
    testSingleNestedCommentWithExtraTextBetweenClosesWithQuote) {
  CommentStream stream("This is a text/* with a block comment/* with a block "
                       "comment */Doesn't (see the quote?) really matter*/.", 1);
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testSingleNestedCommentWithExtraTextBetweenCloses) {
  CommentStream stream("This is a text/* with a block comment/* with a block "
                       "comment */Does not really matter*/.", 1);
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(
    testSingleNestedCommentWithExtraTextBetweenClosesWithEscapedQuote) {
  CommentStream stream("This is a text/* with a block comment/* with a block "
                       "comment */Does not \\'t not really matter*/.", 1);
  std::string expected = "This is a text.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testSingleNestedCommentNotClosed) {
  CommentStream stream(
      "This is a text/* with a block comment/* with a block comment */.", 1);
  std::string expected = "This is a text";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testDoubleNestedCommentWhenSingleAllowed) {
  CommentStream stream("This is a text/* with a block comment/* with a block "
                       "comment/* and too deep!*/*/*/.", 1);
  std::string expected = "This is a text*/.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testDoubleNestedCommentWhenDoubleAllowed) {
  CommentStream stream("This is a text/* with a block comment/* with a block "
                       "comment/* and too deep!*/*/*/.", 1);
  std::string expected = "This is a text*/.";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLong) {
  CommentStream stream("This text12345 has no quality!!54321...", 0, "12345");
  std::string expected = "This text...";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_4) {
  CommentStream stream("This text 12345 has no quality!!5432...", 0, "12345");
  std::string expected = "This text ";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_3) {
  CommentStream stream("This text 12345 has no quality!!543...", 0, "12345");
  std::string expected = "This text ";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_4_EOF) {
  CommentStream stream("This text 12345 has no quality!!5432", 0, "12345");
  std::string expected = "This text ";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}

BOOST_AUTO_TEST_CASE(testCommentLongUnfinishedEnd_5_3_EOF) {
  CommentStream stream("This text 12345 has no quality!!543", 0, "12345");
  std::string expected = "This text ";

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.inComment());
}


BOOST_AUTO_TEST_CASE(testNoQuotesIdentical) {
  CommentStream stream("This is a text without any quotes");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());

}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_NotAtEndOrStart) {
  CommentStream stream("This is a 'text with quotes'.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_AtEnd) {
  CommentStream stream("This is a 'text with quotes.'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_AtStart) {
  CommentStream stream("'This is a text', with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithQuotesIdentical_AtStartAndEnd) {
  CommentStream stream("'This is a text with quotes.'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedQuote) {
  CommentStream stream("This is a 'text with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL('\'', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\'', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedQuoteEscaped) {
  CommentStream stream("This is a 'text with quotes.\\");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL('\'', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\'', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedQuoteEscapedQuote) {
  CommentStream stream("This is a 'text with quotes.\\'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL('\'', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\'', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_NotAtEndOrStart) {
  CommentStream stream("This is a 'text with quotes'.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_AtEnd) {
  CommentStream stream("This is a 'text with quotes.'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_AtStart) {
  CommentStream stream("'This is a text', with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testWithSecondQuotesIdentical_AtStartAndEnd) {
  CommentStream stream("'This is a text with quotes.'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedSecondQuote) {
  CommentStream stream("This is a 'text with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL('\'', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\'', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedSecondQuoteEscaped) {
  CommentStream stream("This is a 'text with quotes.\\");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL('\'', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\'', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testUnclosedSecondQuoteEscapedSecondQuote) {
  CommentStream stream("This is a 'text with quotes.\\'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(true, stream.inQuote());
  BOOST_CHECK_EQUAL('\'', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\'', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testEscaped) {
  CommentStream stream("This is escaped:\\");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(true, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testEscapedQuote) {
  CommentStream stream("This is escaped:\\'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedQuotes_StartEnd) {
  CommentStream stream("'This is Peter\\'s text with quotes.'");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedQuotes) {
  CommentStream stream("This is 'Peter\\'s text' with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingSecondQuote) {
  CommentStream stream("This is 'Peter 9\" text' with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedSecondQuote) {
  CommentStream stream("This is 'Peter 9\\\" text' with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingFirstQuote) {
  CommentStream stream("This is \"Peter 9' text\" with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}

BOOST_AUTO_TEST_CASE(testQuotesContainingEscapedFirstQuote) {
  CommentStream stream("This is \"Peter 9\\' text\" with quotes.");
  std::string expected = stream.getInput();

  BOOST_CHECK_EQUAL(expected.length(), stream.length());
  BOOST_CHECK_EQUAL(expected, stream.getOutput());
  BOOST_CHECK_EQUAL(false, stream.isEscaped());
  BOOST_CHECK_EQUAL(false, stream.inQuote());
  BOOST_CHECK_EQUAL('\0', stream.getOpenQuote());
  BOOST_CHECK_EQUAL('\0', stream.getCloseQuote());
}


BOOST_AUTO_TEST_SUITE_END()
