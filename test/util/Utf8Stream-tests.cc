//
// Created by michel on 20-12-21.
//

#include "boost-unit-tests.h"
#include <org-simple/util/StringStream.h>
#include <org-simple/util/Utf8Stream.h>

using Utf8 = org::simple::charEncode::Utf8Encoding;

namespace {
class TeeTest : public org::simple::util::InputStream<char> {
  org::simple::util::InputStream<char> &input;
  char lastRead;
  std::string string;

public:
  TeeTest(org::simple::util::InputStream<char> &stream) : input(stream) {}

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

template <typename number, bool separators = false>
static const char *binary(number num) {
  static constexpr size_t bits = sizeof(num) * 8;
  static char buffer[bits + 1 + bits / 3];
  number test = number(1) << (sizeof(num) * 8 - 1);
  if constexpr (separators) {
    int i;
    int j;
    for (i = 0, j = 0; i < bits; i++, test >>= 1) {
      if (i != 0 && (i % 4) == 0) {
        buffer[j++] = '_';
      }
      buffer[j++] = test & num ? '1' : '0';
    }
    buffer[j] = '\0';
  } else {
    int i;
    for (i = 0; i < bits; i++, test >>= 1) {
      buffer[i] = test & num ? '1' : '0';
    }
    buffer[i] = '\0';
  }
  return buffer;
}

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
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(string.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(string, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUniCodeYieldsSameResult) {
  char charArray[5];
  org::simple::util::CStringInputStream<char> sstream(charArray);
  int attempt = 0;
  for (Utf8::codePoint cp = 1; cp <= Utf8::maximumCodePoint; cp++) {
    char *afterEncodedPosition = Utf8::unsafeEncode(cp, charArray);
    if (afterEncodedPosition == nullptr) {
      BOOST_CHECK(afterEncodedPosition != nullptr);
      return;
    }
    *afterEncodedPosition = 0;
    int length = afterEncodedPosition - charArray;
    sstream.rewind();
    bool failed = false;
    org::simple::charEncode::ValidatedUtf8Stream stream(sstream);
    for (int i = 0; i < length; i++) {
      char x;
      if (!stream.get(x) || x != charArray[i]) {
        failed = true;
      }
    }
    if (failed) {
      std::stringstream out;
      out << "Failure\n\tcp = " << binary(cp) << "\n\tbytes  =";
      for (int p = 0; p < length; p++) {
        out << " " << binary(charArray[p]);
      }
      out << "\n\tstream =";
      sstream.rewind();
      org::simple::charEncode::ValidatedUtf8Stream stream(sstream);
      for (int i = 0; i < length; i++) {
        out << " ";
        char c;
        if (stream.get(c)) {
          out << binary(c);
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

BOOST_AUTO_TEST_CASE(testInvalidStartByteOmitted) {
  std::string string = "01234\xff 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidIncluded) {
  std::string string = "01234\xc6\xb3 56";
  std::string expected = string;
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUncompletedOmitted_l2) {
  std::string string = "01234\xc6";
  std::string expected = "01234";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUncompletedOmitted_l3) {
  std::string string = "01234\xe6\xb3";
  std::string expected = "01234";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testValidUncompletedOmitted_l4) {
  std::string string = "01234\xf6\xb3\xb3";
  std::string expected = "01234";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_NoMarker_l2) {
  std::string string = "01234\xc3\x03 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_HighMarker_l2) {
  std::string string = "01234\xc3\xc3 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_NoMarker_l3) {
  std::string string = "01234\xe3\xb3\x03 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_HighMarker_l3) {
  std::string string = "01234\xe3\xb3\xc3 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_NoMarker_l4) {
  std::string string = "01234\xf3\xb3\xb3\x03 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testInvalidContinuationByteOmitted_HighMarker_l4) {
  std::string string = "01234\xf3\xb3\xb3\xc3 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}

BOOST_AUTO_TEST_CASE(testTooHighCodePointOmitted) {
  std::string string = "01234\xf7\xbf\xbf\xbf 56";
  std::string expected = "01234 56";
  org::simple::util::StringInputStream<char> input(string);
  org::simple::charEncode::ValidatedUtf8Stream stream(input);
  org::simple::util::InputCollector<char> collector(string.length());
  BOOST_CHECK_EQUAL(expected.length(), collector.consume(stream));
  BOOST_CHECK_EQUAL(expected, collector.getString());
}



BOOST_AUTO_TEST_SUITE_END()
