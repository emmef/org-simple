//
// Created by michel on 20-12-21.
//

#include "boost-unit-tests.h"
#include "org-simple/util/text/StringStream.h"
#include "org-simple/util/text/Utf8Stream.h"
#include "BitsHelper.h"

using Utf8 = org::simple::util::text::Utf8Encoding;

namespace {
class TeeTest : public org::simple::util::text::InputStream<char> {
  org::simple::util::text::InputStream<char> &input;
  char lastRead;
  std::string string;

public:
  TeeTest(org::simple::util::text::InputStream<char> &stream) : input(stream) {}

  bool get(char &result) override {
    if (input.get(lastRead)) {
      result = lastRead;
      string += lastRead;
      return true;
    }
    return false;
  }

  char getLastRead() const { return lastRead; }
  const std::string getString() const { return string; }
};
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_ValidateUtf8Stream_Tests)

BOOST_AUTO_TEST_CASE(testPureAsciiYieldsSameResult) {
  char charArray[127];
  int i;
  for (i = 0; i < 127; i++) {
    charArray[i] = i + 1;
  }
  charArray[i] = 0;
  std::string string = charArray;
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUniCodeYieldsSameResult) {
  struct Stream : public org::simple::util::text::InputStream<char> {
    char charArray[5] = { 0, 0, 0, 0, 0 };
    size_t pos = 0;

    bool get(char &value) {
      char x = charArray[pos];
      if (x) {
        value = x;
        pos++;
        return true;
      }
      return false;
    }

    void rewind() { pos = 0; }
  } sstream;

  int attempt = 0;
  for (Utf8::codePoint cp = 1; cp <= Utf8::maximumCodePoint; cp++) {
    char *afterEncodedPosition = Utf8::unsafeEncode(cp, sstream.charArray);
    if (afterEncodedPosition == nullptr) {
      BOOST_CHECK(afterEncodedPosition != nullptr);
      return;
    }
    *afterEncodedPosition = 0;
    int length = afterEncodedPosition - sstream.charArray;
    sstream.rewind();
    bool failed = false;
    org::simple::util::text::ValidatedUtf8Stream stream(sstream);
    for (int i = 0; i < length; i++) {
      char x;
      if (!stream.get(x) || x != sstream.charArray[i]) {
        failed = true;
      }
    }
    if (failed) {
      std::stringstream out;
      out << "Failure\n\tcp = " << renderBits(cp) << "\n\tbytes  =";
      for (int p = 0; p < length; p++) {
        out << " " << renderBits(sstream.charArray[p]);
      }
      out << "\n\tstream =";
      sstream.rewind();
      org::simple::util::text::ValidatedUtf8Stream stream(sstream);
      for (int i = 0; i < length; i++) {
        out << " ";
        char c;
        if (stream.get(c)) {
          out << renderBits(c);
        }
        else {
          out << "????????";
        }
      }
      BOOST_CHECK_MESSAGE(false, out.str());
      attempt++;
      if (attempt > 2) {
        return;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(tesUnicodeToUtf8ToUnicode) {
  class CodePointGenerator : public org::simple::util::text::InputStream<Utf8::codePoint > {
    Utf8::codePoint cp = 0;
    const Utf8::codePoint end = Utf8::maximumCodePoint;

  public:
    bool get(Utf8::codePoint &result) {
      if (cp <= end) {
        result = cp++;
        return true;
      }
      return false;
    }
  } generator;

  org::simple::util::text::UnicodeToUtf8CharStream charStream(generator);
  org::simple::util::text::Utf8CharToUnicodeStream uniStream(charStream);

  Utf8::codePoint expected;
  const Utf8::codePoint end = Utf8::maximumCodePoint;

  for (expected = 0; expected <= end; expected++) {
    Utf8::codePoint actual;
    const auto gotActual = uniStream.get(actual);
    if (!gotActual) {
      BOOST_CHECK(gotActual);
    }
    const bool toUtf8AndBackSame = expected == actual;
    if (!toUtf8AndBackSame) {
      BOOST_CHECK_EQUAL(expected, actual);
    }
  }
}

BOOST_AUTO_TEST_CASE(testInvalidStartByteOmitted) {
  std::string string = "01234\xff 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidIncluded) {
  std::string string = "01234\xc6\xb3 56";
  std::string expected = string;
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUncompletedOmitted_l2) {
  std::string string = "01234\xc6";
  std::string expected = "01234";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUncompletedOmitted_l3) {
  std::string string = "01234\xe6\xb3";
  std::string expected = "01234";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUncompletedOmitted_l4) {
  std::string string = "01234\xf6\xb3\xb3";
  std::string expected = "01234";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_NoMarker_l2) {
  std::string string = "01234\xc3\x03 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_HighMarker_l2) {
  std::string string = "01234\xc3\xc3 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_NoMarker_l3) {
  std::string string = "01234\xe3\xb3\x03 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_HighMarker_l3) {
  std::string string = "01234\xe3\xb3\xc3 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_NoMarker_l4) {
  std::string string = "01234\xf3\xb3\xb3\x03 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_HighMarker_l4) {
  std::string string = "01234\xf3\xb3\xb3\xc3 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testTooHighCodePointOmitted) {
  std::string string = "01234\xf7\xbf\xbf\xbf 56";
  std::string expected = "01234 56";
  org::simple::util::text::StringInputStream<char> input(string);
  org::simple::util::text::ValidatedUtf8Stream stream(input);
  org::simple::util::text::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}



BOOST_AUTO_TEST_SUITE_END()
