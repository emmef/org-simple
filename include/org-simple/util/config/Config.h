#ifndef ORG_SIMPLE_UTIL_CONFIG_M_CONFIG_H
#define ORG_SIMPLE_UTIL_CONFIG_M_CONFIG_H
/*
 * org-simple/util/config/Config.h
 *
 * Added by michel on 2021-12-05
 * Copyright (C) 2015-2021 Michel Fleur.
 * Source https://github.com/emmef/org-simple
 * Email org-simple@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <exception>
#include <org-simple/util/config/ConfigReaders.h>
#include <org-simple/util/text/CommentStream.h>
#include <org-simple/util/text/InputStreams.h>
#include <org-simple/util/text/StreamPredicate.h>
#include <org-simple/util/text/TextFilePosition.h>
#include <sstream>
#include <string>

namespace org::simple::util::config {

class ParseError : public std::exception {
  std::size_t ln;
  std::size_t col;
  std::string message;

public:
  ParseError(const char *s, unsigned line, unsigned column)
      : std::exception(), ln(line), col(column) {
    std::stringstream stream(s);
    stream << "@" << ln << ":" << col << ": " << s;
    message = stream.str();
  }

  explicit ParseError(const char *s)
      : std::exception(), ln(0), col(0), message(s) {}

  const char *what() const noexcept override { return message.c_str(); }

  std::size_t getLine() const { return ln; }
  std::size_t getColumn() const { return col; }
};

template <typename T> struct KeyValueConfigTypes;

template <> struct KeyValueConfigTypes<char> {
  using classifierType = org::simple::util::text::Ascii;
  const classifierType classifier =
      org::simple::util::text::Classifiers::instance<classifierType>();
};

template <>
struct KeyValueConfigTypes<org::simple::util::text::Utf8Encoding::codePoint> {
  using classifierType = org::simple::util::text::Unicode;
  const classifierType classifier =
      org::simple::util::text::Classifiers::instance<classifierType>();
};

template <typename CP> class KeyValueConfig {
  enum class State {
    LineStart,
    UnQuotedKey,
    SkipToEndOfLine,
    SkipToAssignment,
    SkipToValue,
    EndOfKeyValuePair
  };
  static bool noAssignmentPredicate(const CP &c) { return c != '='; }
  static bool isAssignmentPredicate(const CP &c) { return c == '='; }

  KeyValueConfigTypes<char> configTypes;
  using positions = const org::simple::util::text::TextFilePositionData<CP>;
  typedef typename KeyValueConfigTypes<CP>::classifierType classifierType;
  typedef decltype(util::Predicates::of(
      util::text::GraphPredicate<CP, classifierType>::function,
      noAssignmentPredicate)) UnquotedKeyPredicate;
  UnquotedKeyPredicate unquotedKeyPredicate = util::Predicates::of(
      util::text::GraphPredicate<CP, classifierType>::function,
      noAssignmentPredicate);
  using NewLinePredicate = util::text::NewLinePredicate<CP, true>;
  const NewLinePredicate &newLinePredicate = NewLinePredicate ::instance();
  using QuoteBasedPredicate = util::text::QuoteStatePredicate<CP, true>;
  using EchoStream = util::text::EchoRepeatOneStream<CP>;
  using EndOfQuoteStream =
      util::text::PredicateVariableInputStream<CP, QuoteBasedPredicate, false,
                                               EchoStream>;
  using GraphOnlyStream =
      util::text::PredicateVariableInputStream<CP, UnquotedKeyPredicate, false,
                                               EchoStream>;
  using BeforeNewLineStream =
      util::text::PredicateVariableInputStream<CP, NewLinePredicate, false,
                                               EchoStream>;

  void handleKey(State &state, text::InputStream<CP> &stream,
                 KeyReader<CP> &keyReader, bool ignoreErrors,
                 const positions *pos, const CP &lastReadValue, const util::text::QuoteState<CP> &quoteState) {
    try {
      keyReader.read(stream);
    } catch (const ParseError &e) {
      if (ignoreErrors) {
        state = State::SkipToEndOfLine;
        return;
      }
      throw createError(e.what(), pos);
    }
    if (!keyReader.getKey()) {
      if (ignoreErrors) {
        state = State::SkipToEndOfLine;
        return;
      }
      throw createError("Error reading key value", pos);
    }
    if (state == State::UnQuotedKey) {
      if (quoteState.inQuote()) {
        throw createError("Unfinished quote inside key-name", pos);
      }
      if (lastReadValue == '=') {
        state = State::SkipToValue;
        return;
      }
    }
    state = State::SkipToAssignment;
  }

  void handleValue(State &state, text::InputStream<CP> &stream,
                   ValueReader<CP> &valueReader, const CP *keyName,
                   bool ignoreErrors, const positions *pos) {
    try {
      switch (valueReader.read(stream, keyName)) {
      case ReaderResult::Ok:
        state = State::EndOfKeyValuePair;
        break;
      case ReaderResult::NotFound:
        if (!ignoreErrors) {
          std::string message = "Unknown key: ";
          message += keyName;
          throw createError(message.c_str(), pos);
        }
        state = State::EndOfKeyValuePair;
        break;
      case ReaderResult::TooLong:
        if (!ignoreErrors) {
          std::string message = "Value length exceeded for key ";
          message += keyName;
          throw createError(message.c_str(), pos);
        }
        state = State::EndOfKeyValuePair;
        break;
      }
    } catch (const ParseError &e) {
      if (ignoreErrors) {
        state = State::EndOfKeyValuePair;
        return;
      }
      throw createError(e.what(), pos);
    }
  }

  ParseError createError(const char *msg, const positions *pos) {
    return ParseError(msg, pos ? pos->getLine() + 1 : 0,
                      pos ? pos->getColumn() + 1 : 0);
  }

public:
  void parse(util::text::CommentStream<CP> &commentStream, const positions *pos,
             bool ignoreErrors, KeyReader<CP> &keyReader,
             ValueReader<CP> &valueReader) {

    State state = State::LineStart;
    EchoStream echoStream(commentStream);
    QuoteBasedPredicate quoteBasedPredicate(commentStream.state());
    EndOfQuoteStream inQuoteStream(&echoStream, quoteBasedPredicate);
    GraphOnlyStream nonGraphTerminatedStream(&echoStream, unquotedKeyPredicate);
    BeforeNewLineStream newLineTerminatedStream(&echoStream, newLinePredicate);

    CP c;
    while (echoStream.get(c)) {
      switch (state) {

      case State::LineStart:
        if (configTypes.classifier.isWhiteSpace(c) || (c == '\n')) {
          continue;
        }
        if (commentStream.state().inQuote()) {
          handleKey(state, inQuoteStream, keyReader, ignoreErrors, pos,
                    echoStream.lastValue(), commentStream.state());
          continue;
        } else if (c != '=' && configTypes.classifier.isGraph(c)) {
          state = State::UnQuotedKey;
          echoStream.repeat();
          handleKey(state, nonGraphTerminatedStream, keyReader, ignoreErrors,
                    pos, echoStream.lastValue(), commentStream.state());
          continue;
        }
        if (ignoreErrors) {
          state = State::SkipToEndOfLine;
          break;
        }
        throw createError("Unexpected start of key-value pair", pos);
      case State::SkipToEndOfLine:
        if (c == '\n') {
          state = State::LineStart;
        }
        break;
      case State::SkipToAssignment:
        if (c == '=') {
          state = State::SkipToValue;
        } else if (configTypes.classifier.isWhiteSpace(c)) {
          break;
        } else if (c == '\n') {
          state = State::LineStart;
        }
        break;
      case State::SkipToValue:
        if (commentStream.state().inQuote()) {
          handleValue(state, inQuoteStream, valueReader, keyReader.getKey(),
                      ignoreErrors, pos);
        } else if (!configTypes.classifier.isWhiteSpace(c)) {
          echoStream.repeat();
          handleValue(state, newLineTerminatedStream, valueReader,
                      keyReader.getKey(), ignoreErrors, pos);
        }
        break;
      case State::EndOfKeyValuePair:
        if (!commentStream.state().inQuote()) {
          state = State::LineStart;
          echoStream.repeat();
          break;
        }
        throw createError("Unclosed quote in key-value pair", pos);
      default:
        throw createError("Unexpected state.", pos);
      }
    };
    if (state == State::LineStart) {
      return;
    }
    if (state == State::EndOfKeyValuePair) {
      if (!commentStream.state().inQuote()) {
        return;
      }
      throw createError("Unclosed quote in key-value pair", pos);
    }
    throw createError("Unexpected end of input.", pos);
  }
};

} // namespace org::simple::util::config

#endif // ORG_SIMPLE_UTIL_CONFIG_M_CONFIG_H
