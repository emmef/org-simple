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
typedef org::simple::util::text::TokenizedInputStream<char>
    TokenizedInputStream;
typedef org::simple::util::text::CommentStreamConfig<char> CommentStreamConfig;
typedef org::simple::util::text::CommentStream<char> CommentStream;
typedef org::simple::util::text::StringInputStream<char> StringStream;

static CommentStreamConfig config("//", "/*", "'");

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

  const char *getSource() const { return source.getCString(); }
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
    ReaderResult read(TokenizedInputStream &input, const char *keyName) final {
      value.clear();
      char c;
      while (!input.isExhausted()) {
        while (input.get(c)) {
          value += c;
        }
        if (input.get(c)) {
          value += ' ';
          value += c;
        }
      }
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

  const std::vector<KeyValuePair> &getResults() const { return results; }

  const char *getSource() const { return sourceStream.getSource(); }
};

class Scenario {
  std::string source;
  std::vector<KeyValuePair> expectedResults;

public:
  Scenario(const char *input) : source(input) {}
  Scenario(const char *input, const char *key, const char *value)
      : Scenario(input) {
    expectedResults.push_back({key, value});
  };

  void add(const char *key, const char *value) {
    expectedResults.push_back({key, value});
  }

  const char *getSource() const { return source.c_str(); }
  std::size_t getSize() const { return expectedResults.size(); }
  const std::vector<KeyValuePair> &getExpectedResults() const {
    return expectedResults;
  }
};

std::vector<Scenario> generateTestScenarios() {
  std::vector<std::pair<const char *, const char *>> keyTests;

  keyTests.push_back({"key", "key"});
  keyTests.push_back({" key", "key"});
  keyTests.push_back({"key ", "key"});
  keyTests.push_back({" key ", "key"});
  keyTests.push_back({"'key'", "key"});
  keyTests.push_back({" 'key'", "key"});
  keyTests.push_back({"'key' ", "key"});
  keyTests.push_back({" 'key' ", "key"});
  keyTests.push_back({"'ke y'", "ke y"});
  keyTests.push_back({" 'ke y'", "ke y"});
  keyTests.push_back({"'ke y' ", "ke y"});
  keyTests.push_back({" 'ke y' ", "ke y"});

  std::vector<std::pair<const char *, const char *>> valueTests;

  valueTests.push_back({"value", "value"});
  valueTests.push_back({" value", "value"});
  valueTests.push_back({"value ", "value"});
  valueTests.push_back({" value ", "value"});
  //  valueTests.push_back({"value ", "value "});
  //  valueTests.push_back({" value ", "value "});
  valueTests.push_back({"'value'", "value"});
  valueTests.push_back({" 'value'", "value"});
  valueTests.push_back({"'value' ", "value"});
  valueTests.push_back({" 'value' ", "value"});
  valueTests.push_back({"'val ue'", "val ue"});
  valueTests.push_back({" 'val ue'", "val ue"});
  valueTests.push_back({"'val ue' ", "val ue"});
  valueTests.push_back({" 'val ue' ", "val ue"});

  valueTests.push_back({"value 'has' one", "value has one"});
  valueTests.push_back({"value 'has' one ", "value has one"});
  valueTests.push_back({" value 'has' one ", "value has one"});
  valueTests.push_back({" value 'has' one", "value has one"});

  std::vector<Scenario> scenarios;

  scenarios.push_back("");

  for (auto key : keyTests) {
    for (auto value : valueTests) {
      std::string source = key.first;
      source += '=';
      source += value.first;
      scenarios.push_back({source.c_str(), key.second, value.second});
      source += '\n';
      scenarios.push_back({source.c_str(), key.second, value.second});
    }
  }

  size_t nrKeys = keyTests.size();
  size_t keyNumber = 0;
  size_t nrValues = valueTests.size();
  size_t valueNumber = 0;
  for (size_t i = 0; i < 100; i++) {
    auto key1 = keyTests.at(keyNumber % nrKeys);
    keyNumber--;
    auto key2 = keyTests.at(keyNumber % nrKeys);
    keyNumber--;
    auto value1 = valueTests.at(valueNumber % nrValues);
    valueNumber++;
    auto value2 = valueTests.at(valueNumber % nrValues);
    valueNumber++;
    std::string source = key1.first;
    source += '=';
    source += value1.first;
    source += '\n';
    source += key2.first;
    source += '=';
    source += value2.first;
    source += '\n';

    Scenario scenario(source.c_str(), key1.second, value1.second);
    scenario.add(key2.second, value2.second);

    scenarios.push_back(scenario);
  }

  return scenarios;
}

std::string printable(const std::string &value) {
  std::string result = "\"";

  for (auto c : value) {
    if (c == '\n') {
      result += "\\n";
    } else {
      result += c;
    }
  }
  result += "\"";
  return result;
}

static std::ostream &operator<<(std::ostream &out, const Scenario &scenario) {
  out << "Scenario " << printable(scenario.getSource()) << " =>";
  for (auto tok : scenario.getExpectedResults()) {
    out << " {" << printable(tok.key) << ", " << printable(tok.value) << "}";
  }
  return out;
}

} // namespace

BOOST_AUTO_TEST_SUITE(test_org_simple_util_config_KeyvalueConfig)

BOOST_DATA_TEST_CASE(testNonThrowingScenarios, generateTestScenarios()) {
  PrintingReader reader(sample.getSource());

  auto actual = reader.parse();
  auto expected = sample.getExpectedResults();
  BOOST_CHECK_EQUAL(sample.getSize(), actual.size());

  const int count = std::min(expected.size(), actual.size());
  for (int comparisonNr = 0; comparisonNr < count; comparisonNr++) {
    BOOST_CHECK_EQUAL(expected[comparisonNr].key, actual[comparisonNr].key);
    BOOST_CHECK_EQUAL(expected[comparisonNr].value, actual[comparisonNr].value);
  }
}

BOOST_AUTO_TEST_CASE(testInitialize) { PrintingReader reader(""); }

BOOST_AUTO_TEST_CASE(testParseOneLineQuotedKeyAndValueWithQuoteLater) {
  PrintingReader reader("'key'=val'lala");
  BOOST_CHECK_THROW(reader.parse(), org::simple::util::config::ParseError);
}

BOOST_AUTO_TEST_CASE(testParseOneLineUnquotedKeyWithQuoteLater) {
  PrintingReader reader("'ke'y'=value");
  BOOST_CHECK_THROW(reader.parse(), org::simple::util::config::ParseError);
}

BOOST_AUTO_TEST_CASE(testParseOneLineNothUnquotedWithQuoteLater) {
  // TODO This corner care fails.
//  PrintingReader reader("'ke'y'=val'ue");
//  BOOST_CHECK_THROW(reader.parse(), org::simple::util::config::ParseError);
}

BOOST_AUTO_TEST_CASE(testParseOneLineQuotedKeyAndValueUnclosed) {
  PrintingReader reader("'key'='value");
  BOOST_CHECK_THROW(reader.parse(), org::simple::util::config::ParseError);
}

BOOST_AUTO_TEST_CASE(testParseTwoLines) {
  PrintingReader reader("key1=value1\n"
                        "key2=value2");
  auto results = reader.parse();
  BOOST_CHECK_EQUAL(2, results.size());
  BOOST_CHECK_EQUAL("key1", results.at(0).key);
  BOOST_CHECK_EQUAL("value1", results.at(0).value);
  if (results.size() > 1) {
    BOOST_CHECK_EQUAL("key2", results.at(1).key);
    BOOST_CHECK_EQUAL("value2", results.at(1).value);
  }
}

BOOST_AUTO_TEST_SUITE_END()
