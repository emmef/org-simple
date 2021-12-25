#ifndef ORG_SIMPLE_CONFIG_H
#define ORG_SIMPLE_CONFIG_H
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
    QuotedKey,
    UnQuotedKey,
    SkipToEndOfLine,
    SkipToAssignment,
    SkipToValue
  };
  KeyValueConfigTypes<char> configTypes;
  using positions = const org::simple::util::text::TextFilePositionData<CP>;
  typedef typename KeyValueConfigTypes<CP>::classifierType classifierType;
  // Filters to indicate specific areas of a key value pair
  using EndOfQuoteFilter = util::text::EndOfQuotedTerminationFilter<CP>;
  using NonGraphFilter =
      util::text::NonGraphTerminatedFilter<CP, classifierType>;
  using NewLineFilter = util::text::NewLineTerminatedFilter<CP>;
  using EchoStream = util::text::EchoRememberLastInputStream<CP>;
  // Streams that terminate when specific areas (see filters) are ended
  using InquoteStream =
      util::text::FilteredInputStream<EndOfQuoteFilter, EchoStream, CP>;
  using GraphOnlyStream =
      util::text::FilteredVariableInputStream<NonGraphFilter, EchoStream, CP,
                                              false>;
  using BeforeNewLineStream =
      util::text::FilteredVariableInputStream<NewLineFilter, EchoStream, CP,
                                              false>;

  NonGraphFilter nonGraphTerminatedFilter;
  NewLineFilter newLineTerminatedFilter;

  template <class S>
  requires(hasInputStreamSignature<S, CP>)
  void handleKey(State &state, S &stream,
                 KeyReader<CP> &keyReader, bool ignoreErrors,
                 const positions *pos, const CP &lastReadValue) {
    try {
      auto x = wrapAsInputStream<S, CP>(stream);
      keyReader.read(x);
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

    state = state == State::UnQuotedKey && lastReadValue == '='
                ? State::SkipToValue
                : State::SkipToAssignment;
  }

  void handleValue(State &state, util::InputStream<CP> &stream,
                   ValueReader<CP> &valueReader, const CP *keyName,
                   bool ignoreErrors, const positions *pos) {
    try {
      valueReader.read(stream, keyName);
    } catch (const ParseError &e) {
      if (ignoreErrors) {
        state = State::SkipToEndOfLine;
        return;
      }
      throw createError(e.what(), pos);
    }
  }

  ParseError createError(const char *msg, const positions *pos) {
    return ParseError(msg, pos ? pos->getLine() + 1 : 0, pos ? pos->getColumn() + 1 : 0);
  }

public:
  void parse(util::text::CommentStream<CP> &commentStream, const positions *pos,
             bool ignoreErrors, KeyReader<CP> &keyReader,
             ValueReader<CP> &valueReader) {

    State state = State::LineStart;
    EndOfQuoteFilter endOfQuoteFilter(commentStream.state());
    EchoStream echoStream(commentStream);
    InquoteStream inQuoteStream(endOfQuoteFilter, echoStream);
    GraphOnlyStream nonGraphTerminatedStream(nonGraphTerminatedFilter,
                                             &echoStream);
    BeforeNewLineStream newLineTerminatedStream(newLineTerminatedFilter,
                                                &echoStream);

    CP c;
    while (echoStream.get(c)) {
      switch (state) {

      case State::LineStart:
        if (configTypes.classifier.isWhiteSpace(c) ||
            (c == '\n')) {
          continue;
        }
        if (commentStream.state().inQuote()) {
          state = State::QuotedKey;
          continue;
        } else if (c != '=' && configTypes.classifier.isGraph(c)) {
          state = State::UnQuotedKey;
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
      case State::QuotedKey:
        handleKey(state, inQuoteStream, keyReader, ignoreErrors, pos,
                  echoStream.lastValue());
        break;
      case State::UnQuotedKey:
        handleKey(state, nonGraphTerminatedStream, keyReader, ignoreErrors, pos,
                  echoStream.lastValue());
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
        if (!configTypes.classifier.isWhiteSpace(c)) {
          handleValue(state, newLineTerminatedStream, valueReader,
                      keyReader.getKey(), ignoreErrors, pos);
        }
        break;
      }
    };
    if (state != State::LineStart) {
      throw createError("Unexpected end of input.", pos);
    }
  }
};

} // namespace org::simple::util::config

#endif // ORG_SIMPLE_CONFIG_H
