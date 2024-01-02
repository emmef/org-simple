#ifndef ORG_SIMPLE_CONFIG_M_CONFIG_H
#define ORG_SIMPLE_CONFIG_M_CONFIG_H
/*
 * org-simple/config/Config.h
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
#include <functional>
#include <org-simple/config/ConfigReaders.h>
#include <org-simple/text/CommentStream.h>
#include <org-simple/text/InputStreams.h>
#include <org-simple/text/StreamPredicate.h>
#include <org-simple/text/TextFilePosition.h>
#include <org-simple/text/UnixNewLine.h>
#include <sstream>
#include <string>

namespace org::simple::config {

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

template <typename C> class KeyValueConfig {
  using Positions = const org::simple::text::TextFilePositionData<C>;

  class Parser {
    enum class State {
      LineStart,
      SkipToAssignment,
      SkipToValue,
      EndOfKeyValuePair
    };
    class Predicate : public org::simple::Predicate<C> {
    public:
      using Classifiers = org::simple::text::Classifiers;

      Classifiers ::defaultType<C> &classifier =
          org::simple::text::Classifiers::defaultInstance<C>();

      std::function<bool(const char &)> whiteSpace() {
        return [&](const char &c) { return classifier.isWhitespace(c); };
      }

      bool test(const C &c) const override {
        return assignment(c) || classifier.isWhiteSpace(c);
      }

      static bool assignment(const char &c) { return c == '=' || c == ':'; }
    } predicate;

    using NewlineTokenizedStream =
        text::NewlineTokenizedStream<C, text::CommentStream<C>>;
    using QuoteConfig = typename text::QuoteState<C>::Config;
    using QuoteStateFilter = text::QuoteStateFilter<C>;

    using QuoteFilteredStream =
        text::QuoteFilteredStream<C, NewlineTokenizedStream>;

    using QuoteTokenizedStream =
        text::QuoteStateTokenizedStream<C, NewlineTokenizedStream>;

    const Positions *pos;
    NewlineTokenizedStream newlineTokenizedStream;
    QuoteStateFilter quoteStateFilter;
    QuoteFilteredStream quoteFilteredStream;
    QuoteTokenizedStream quoteTokenizedStream;
    KeyReader<C> &keyReader;
    ValueReader<C> &valueReader;
    bool ignoreErrors;

    void skipToEol() {
      C c;
      while (quoteTokenizedStream.get(c))
        ;
    }

    bool handleKey() {
      ReaderResult result = keyReader.read(quoteTokenizedStream);
      if (result != ReaderResult::Ok) {
        if (!ignoreErrors) {
          std::string message = "Error while reading key: ";
          message += readerResultToString(result);
          throw createError(message.c_str());
        }
      }
      return keyReader.getKey() && *keyReader.getKey();
    }

    bool handleValue(const C *keyName) {
      bool success = false;
      switch (valueReader.read(quoteTokenizedStream, keyName)) {
      case ReaderResult::Ok:
        success = true;
        break;
      default:
        if (!ignoreErrors) {
          std::string message = keyName;
          message += ": ";
          message += readerResultToString(AbstractReader::getReaderResult());
          throw createError(message.c_str());
        }
        break;
      }
      return success;
    }

    ParseError createError(const char *msg) {
      return ParseError(msg, pos ? pos->getLine() + 1 : 0,
                        pos ? pos->getColumn() + 1 : 0);
    }

  public:
    Parser(text::CommentStream<C> &commentStream,
           const Positions *positions, bool ignoreErrors_,
           KeyReader<C> &keyReader_, ValueReader<C> &valueReader_)
        : pos(positions), newlineTokenizedStream(commentStream),
          quoteStateFilter(commentStream.state().getConfig()),
          quoteFilteredStream(quoteStateFilter, newlineTokenizedStream),
          quoteTokenizedStream(quoteFilteredStream, &predicate),
          keyReader(keyReader_), valueReader(valueReader_),
          ignoreErrors(ignoreErrors_) {}

    bool parse() {
      State state = State::LineStart;
      while (!newlineTokenizedStream.isExhausted()) {
        if (state == State::LineStart) {
          state = handleKey() ? State::SkipToValue : State::EndOfKeyValuePair;
        } else if (state == State::SkipToValue) {
          quoteTokenizedStream.resetExhausted();
          handleValue(keyReader.getKey());
          state = State::EndOfKeyValuePair;
        } else if (state == State::EndOfKeyValuePair) {
          if (!newlineTokenizedStream.isExhausted()) {
            skipToEol();
            state = State::LineStart;
            quoteTokenizedStream.resetExhausted();
          }
        }
      }
      if (state != State::LineStart && state != State::EndOfKeyValuePair) {
        if (!ignoreErrors) {
          throw createError("Unexpected end of input!");
        }
        return false;
      }
      else if (quoteFilteredStream.getFilter()->inQuote()) {
        throw createError("Unclosed quote at end of input");
      }
      return true;
    }
  };

public:
  void parse(text::CommentStream<C> &commentStream, const Positions *pos,
             bool ignoreErrors, KeyReader<C> &keyReader,
             ValueReader<C> &valueReader) {

    Parser parser(commentStream, pos, ignoreErrors, keyReader, valueReader);
    parser.parse();
  }
};

} // namespace org::simple::config

#endif // ORG_SIMPLE_CONFIG_M_CONFIG_H
