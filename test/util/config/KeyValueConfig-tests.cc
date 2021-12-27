//
// Created by michel on 24-12-21.
//

#include <iostream>
#include <org-simple/util/config/Config.h>
#include <org-simple/util/text/StringStream.h>
#include <vector>

#include "boost-unit-tests.h"

namespace {
typedef org::simple::util::config::KeyValueConfig<char> KeyValueConfig;
typedef org::simple::util::config::ReaderResult ReaderResult;
typedef org::simple::util::config::KeyReader<char> KeyReader;
typedef org::simple::util::config::ValueReader<char> ValueReader;
typedef org::simple::util::text::InputStream<char> InputStream;
typedef org::simple::util::text::CommentStreamConfig<char> CommentStreamConfig;
typedef org::simple::util::text::CommentStream<char> CommentStream;
typedef org::simple::util::text::CStringInputStream<char> StringStream;

static CommentStreamConfig config("//", "/*", "'\"");

class SourceStream {
  StringStream source;
  CommentStream stream;

public:
  SourceStream(const char *string)
      : source(string), stream(source, config, 3) {}

  void reset() {
    source.rewind();
    stream.reset();
  }

  CommentStream &commentStream() { return stream; }
};

struct KeyValuePair {
  std::string key;
  std::string value;
};

class PrintingReader {
  SourceStream sourceStream;
  KeyValueConfig keyValueConfig;
  std::vector<KeyValuePair> results;

  class Key : public KeyReader {
    std::string key;

  public:
    ReaderResult read(InputStream &input) final {
      char c;
      while (input.get(c)) {
        key += c;
      }
//      std::cout << "Key=" << key << std::endl;
      return org::simple::util::config::ReaderResult::Ok;
    }

    const char *getKey() const final { return key.c_str(); }

    void reset() { key.clear(); }
  } keyReader;

  class Value : public ValueReader {
    Key &key;
    std::string value;
    std::vector<KeyValuePair> &results;

  public:
    ReaderResult read(InputStream &input, const char *keyName) final {
      char c;
      while (input.get(c)) {
        value += c;
      }
//      std::cout << "\tValue=" << value << std::endl;
      results.push_back({keyName, value});
      key.reset();
      return org::simple::util::config::ReaderResult::Ok;
    }

    Value(Key &keyReader, std::vector<KeyValuePair> &r)
        : key(keyReader), results(r) {}
  } valueReader;

public:
  PrintingReader(const char *input)
      : sourceStream(input), valueReader(keyReader, results) {}

  const std::vector<KeyValuePair> &parse() {
    keyValueConfig.parse(sourceStream.commentStream(), nullptr, false,
                         keyReader, valueReader);
    return results;
  }
};
} // namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_config_KeyvalueConfig)

BOOST_AUTO_TEST_CASE(testInitialize) {
  PrintingReader reader("");
}

BOOST_AUTO_TEST_CASE(testParseEmpty) {
  PrintingReader reader("");
  auto results = reader.parse();
  BOOST_CHECK_EQUAL(0, results.size());
}

BOOST_AUTO_TEST_CASE(testParseOneLine) {
  PrintingReader reader("key=value");
  auto results = reader.parse();
  BOOST_CHECK_EQUAL(1, results.size());
  BOOST_CHECK_EQUAL("key", results.at(0).key);
  BOOST_CHECK_EQUAL("value", results.at(0).value);
}

BOOST_AUTO_TEST_CASE(testParseOneLineQuotedKey) {
  PrintingReader reader("\"key\"=value");
  auto results = reader.parse();
  BOOST_CHECK_EQUAL(1, results.size());
  BOOST_CHECK_EQUAL("key", results.at(0).key);
  BOOST_CHECK_EQUAL("value", results.at(0).value);
}

BOOST_AUTO_TEST_CASE(testParseOneLineQuotedKeyAndValue) {
  PrintingReader reader("\"key\"=\"value\"");
  auto results = reader.parse();
  BOOST_CHECK_EQUAL(1, results.size());
  BOOST_CHECK_EQUAL("key", results.at(0).key);
  BOOST_CHECK_EQUAL("value", results.at(0).value);
}

BOOST_AUTO_TEST_CASE(testParseOneLineQuotedKeyAndValueWithQuoteLater) {
  PrintingReader reader("\"key\"=val\"lala");
  auto results = reader.parse();
  BOOST_CHECK_EQUAL(1, results.size());
  BOOST_CHECK_EQUAL("key", results.at(0).key);
  BOOST_CHECK_EQUAL("val\"lala", results.at(0).value);
}

BOOST_AUTO_TEST_CASE(testParseOneLineQuotedKeyAndValueUnclosed) {
  PrintingReader reader("\"key\"=\"value");
  BOOST_CHECK_THROW(reader.parse(), org::simple::util::config::ParseError);
}

BOOST_AUTO_TEST_SUITE_END()
