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
#include <org-simple/util/text/Characters.h>
#include <org-simple/util/text/CommentStream.h>
#include <org-simple/util/InputStream.h>
#include <org-simple/util/text/LineContinuation.h>
#include <org-simple/util/text/PosixNewLine.h>
#include <org-simple/util/text/Utf8Stream.h>
#include <sstream>
#include <string>

namespace org::simple::util::config {

class ParseError : public std::exception {
  unsigned ln;
  unsigned col;
  std::stringstream message;

public:
  explicit ParseError(const char *s, unsigned line, unsigned column)
      : std::exception(), ln(line), col(column) {
    message << "[l" << line << ":" << col << "]: " << s;
  }

  explicit ParseError(const char *s) : std::exception(), ln(0), col(0) {
    message << s;
  }

  template <typename T> ParseError &operator<<(const T &v) { message << v; }

  const char *what() const noexcept override { return message.str().c_str(); }
};

template <typename T> class AbstractReader {
public:
  /**
   * Read a key name from the input stream.
   * @param input The input stream to read from.
   * @throws ParserError When key could not be read successfully.
   */
  virtual void read(util::InputStream<T> &input) = 0;

  virtual ~AbstractReader() = default;
};

template <typename T> class AbstractKeyReader : public AbstractReader<T> {
public:
  /**
   * Returns the key name if that was successfully read, or throws a
   * std::runtime_error() is it was not.
   * @return The key name.
   */
  virtual const T *getKey() const = 0;
};

template <typename T> struct KeyValueConfigTypes;

template <> struct KeyValueConfigTypes<char> {
  using classifierType = org::simple::util::text::Ascii;
  using inputStreamType = util::text::ValidatedUtf8Stream;
  using renderStreamType = util::EchoStream<char>;
  const classifierType classifier =
      org::simple::util::text::Classifiers::instance<classifierType>();
};

template <> struct KeyValueConfigTypes<org::simple::util::text::Utf8Encoding::codePoint> {
  using classifierType = org::simple::util::text::Unicode;
  using inputStreamType = org::simple::util::text::Utf8CharToUnicodeStream;
  using renderStreamType = org::simple::util::text::UnicodeToUtf8CharStream;
  const classifierType classifier =
      org::simple::util::text::Classifiers::instance<classifierType>();
};

template <typename T>
struct KeyValueConfigClassifiers : public KeyValueConfigTypes<T> {
  using KeyValueConfigTypes<T>::classifier;

  template <typename codePoint>
  bool isKeyCharacter(codePoint c, codePoint assignment) const {
    return c != assignment && classifier.isGraph(c);
  }
  template <typename codePoint> bool isKeyCharacter(codePoint c) const {
    return isKeyCharacter(c, '=');
  }
  template <typename codePoint> bool isValueCharacter(codePoint c) const {
    return (!classifier.isControl(c) || classifier.isBlank(c));
  }
  template <typename codePoint>
  bool isValueCharacter(codePoint c, codePoint commentStart) const {
    return c != commentStart && isValueCharacter(c);
  }

  class UntilEndOfUnquotedKeyStream : public util::InputStream<T> {
    util::InputStream<T> *input;
    KeyValueConfigClassifiers &owner;
    T lastRead;
    T replayed;

  public:
    UntilEndOfUnquotedKeyStream(KeyValueConfigClassifiers &owner_)
        : owner(owner_){};

    bool get(T &result) {
      if (input != nullptr) {
        if (replayed) {
          lastRead = replayed;
          replayed = 0;
        } else if (!input->get(lastRead)) {
          return false;
        }
        if (!owner.classifier.isWhiteSpace(lastRead)) {
          result = lastRead;
          return true;
        }
        input = nullptr;
      }
      return false;
    }

    T getLastRead() const { return lastRead; }

    void set(util::InputStream<T> *source, T initialChar) {
      input = source;
      replayed = initialChar;
    }
  };

  class UntilEndOfLineStream : public util::InputStream<T> {
    util::InputStream<T> *input;
    T replayed;

  public:
    UntilEndOfLineStream() {}

    bool get(T &result) {
      if (input != nullptr) {
        if (replayed) {
          result = replayed;
          replayed = 0;
        } else if (!input->get(result)) {
          return false;
        }
        if (result != '\n') {
          return true;
        }
        input = nullptr;
      }
      return false;
    }

    void set(util::InputStream<T> *source, T initialChar) {
      input = source;
      replayed = initialChar;
    }
  };
};

template <typename CP>
class KeyValueConfig : public KeyValueConfigClassifiers<CP> {
  enum class State {
    LineStart,
    QuotedKey,
    UnQuotedKey,
    SkipToEndOfLine,
    SkipToAssignment,
    SkipToValue
  };
  using inputStreamType = typename KeyValueConfigTypes<CP>::inputStreamType;
  using KeyValueConfigTypes<CP>::renderStreamType;
  using KeyValueConfigTypes<CP>::classifier;
  using KeyValueConfigTypes<CP>::isKeyCharacter;

  org::simple::util::ReplayStream<char> inputStream;
  org::simple::util::text::PosixNewlineStream<char> posixNewlineStream;
  org::simple::util::text::LineContinuationStream<char> lineContinuationStream;
  org::simple::util::text::CommentStream<char> commentStream;
  inputStreamType inputConversion;
  State state;

  org::simple::util::text::InQuoteStream<CP> inQuoteStream;
  typename KeyValueConfigClassifiers<CP>::UntilEndOfUnquotedKeyStream
      untilEndOfUnquotedKeyStream;
  typename KeyValueConfigClassifiers<CP>::UntilEndOfLineStream
      untilEndOfLineStream;

  void handleKey(util::InputStream<CP> &stream,
                 AbstractKeyReader<CP> &keyReader, bool ignoreErrors) {
    try {
      keyReader.read(stream, keyReader);
    } catch (const ParseError &e) {
      if (ignoreErrors) {
        state = State::SkipToEndOfLine;
        return;
      }
      throw createError(e.what());
    }
    if (!keyReader.getKey()) {
      if (ignoreErrors) {
        state = State::SkipToEndOfLine;
        return;
      }
      throw createError("Error reading key value");
    }

    state = state == State::UnQuotedKey &&
                    untilEndOfUnquotedKeyStream.getLastRead() == '='
                ? state == State::SkipToValue
                : State::SkipToAssignment;
  }

  void handleValue(util::InputStream<CP> &stream,
                   AbstractReader<CP> &valueReader, bool ignoreErrors) {
    try {
      valueReader.read(stream);
    } catch (const ParseError &e) {
      if (ignoreErrors) {
        state = State::SkipToEndOfLine;
        return;
      }
      throw createError(e.what());
    }
  }

  void init(util::InputStream<char> *stream = nullptr) {
    inputStream.assignedStream(stream);
    posixNewlineStream.reset();
    lineContinuationStream.reset();
    commentStream.reset();
    inputConversion.reset();
    state = State::LineStart;
  }

  ParseError createError(const char *msg) {
    return ParseError(msg, posixNewlineStream.state().getLine() + 1,
                      posixNewlineStream.state().getColumn() + 1);
  }

public:
  template <typename V>
  KeyValueConfig(const char *lineCommentString, const char *blockCommentString,
                 unsigned nestingLevels, V quoteMatchers)
      : posixNewlineStream(inputStream),
        lineContinuationStream(posixNewlineStream),
        commentStream(lineContinuationStream, lineCommentString,
                      blockCommentString, nestingLevels, quoteMatchers),
        inputConversion(commentStream), untilEndOfUnquotedKeyStream(*this) {}

  void reset() { init(nullptr); }

  void parse(util::InputStream<char> &input, bool ignoreErrors,
             AbstractKeyReader<CP> &keyReader,
             AbstractReader<CP> &valueReader) {
    init(&input);
    CP c;
    while (input.get(c)) {
      switch (state) {

      case State::LineStart:
        if (classifier.isWhiteSpace(c) || classifier.isNewLine(c)) {
          continue;
        }
        if (commentStream.inQuote()) {
          state = State::QuotedKey;
          continue;
        } else if (KeyValueConfigClassifiers<CP>::isKeyCharacter(c, '=')) {
          state = State::UnQuotedKey;
          continue;
        }
        if (ignoreErrors) {
          state = State::SkipToEndOfLine;
          break;
        }
        throw createError("Unexpected start of key-value pair");
      case State::SkipToEndOfLine:
        if (c == '\n') {
          state = State::LineStart;
        }
        break;
      case State::QuotedKey:
        inQuoteStream.set(&input);
        handleKey(inQuoteStream, keyReader, ignoreErrors);
        break;
      case State::UnQuotedKey:
        untilEndOfUnquotedKeyStream.set(&input, c);
        handleKey(untilEndOfUnquotedKeyStream.keyReader, ignoreErrors);
        break;
      case State::SkipToAssignment:
        if (c == '=') {
          state = State::SkipToValue;
        }
        else if (classifier.isWhiteSpace(c)) {
          break;
        }
        else if (c == '\n') {
          state = State::LineStart;
        }
        break;
      case State::SkipToValue:
        if (!classifier.isWhiteSpace(c)) {
          untilEndOfLineStream.set(&input, c);
          handleValue(untilEndOfLineStream, valueReader, ignoreErrors);
        }
        break;
      }
    };
    if (state != State::LineStart) {
      ParseError error = createError("Unexpected end of file");
      error << ": Unexpected state " << state;
      throw error;
    }
  }
};

} // namespace org::simple::util::config

#endif // ORG_SIMPLE_CONFIG_H
