#ifndef ORG_SIMPLE_TEXTFILTERS_H
#define ORG_SIMPLE_TEXTFILTERS_H
/*
 * org-simple/TextFilters.h
 *
 * Added by michel on 2021-12-21
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

#include <cstdint>
#include <org-simple/util/Characters.h>
#include <org-simple/util/InputStream.h>

namespace org::simple::util {

template <typename T> class PosixNewlineStream : public util::InputStream<T> {
  util::InputStream<T> &input;
  std::size_t line;
  std::size_t position;
  std::size_t column;
  bool lastCR;

public:
  explicit PosixNewlineStream(util::InputStream<T> &stream)
      : input(stream), line(0), position(0), column(0), lastCR(false) {}

  void reset() {
    line = 0;
    position = 0;
    column = 0;
    lastCR = false;
  }

  std::size_t getLine() const { return line; }
  std::size_t getPosition() const { return position; }
  std::size_t getColumn() const { return column; }

  bool get(T &result) override {
    T c;
    while (input.get(c)) {
      position++;
      if (c == '\n') {
        if (lastCR) {
          lastCR = false;
        } else {
          line++;
          column = 0;
          result = '\n';
          return true;
        }
      } else if (c == '\r') {
        line++;
        column = 0;
        lastCR = true;
        result = '\n';
        return true;
      } else {
        lastCR = false;
        result = c;
        column++;
        return true;
      }
    }
    return false;
  }
};

template <typename T>
class LineContinuationStream : public util::InputStream<T> {
  util::InputStream<T> &input;

  enum class State { NORMAL, MARKED, RETURN_NEXT };
  State state;
  T next;

public:
  explicit LineContinuationStream(util::InputStream<T> &stream)
      : input(stream), state(State::NORMAL), next(0) {}

  bool get(T &result) override {
    while (true) {
      if (state == State::RETURN_NEXT) {
        state = State::NORMAL;
        result = next;
        return true;
      }
      T c;
      if (!input.get(c)) {
        if (state == State::MARKED) {
          state = State::NORMAL;
          result = '\\';
          return true;
        }
        return false;
      }
      if (state == State::NORMAL) {
        if (c == '\\') {
          state = State::MARKED;
        } else {
          result = c;
          return true;
        }
      } else if (state == State::MARKED) {
        if (c == '\n') {
          state = State::NORMAL;
          continue;
        }
        next = c;
        state = State::RETURN_NEXT;
        result = '\\';
        return true;
      }
    }
  }
};

template <typename T> class QuoteAndEscapeState {
public:
  virtual bool inQuote() const = 0;
  virtual T getOpenQuote() const = 0;
  virtual T getCloseQuote() const = 0;
  virtual bool isEscaped() const = 0;
  virtual ~QuoteAndEscapeState() = default;
};

template <typename T>
class QuoteAndEscapeHandler : public QuoteAndEscapeState<T> {
public:
  using matcherFunction = typename charClass::QuoteMatcher<T>::function;

  bool inQuote() const final { return openQuote != 0; }
  T getOpenQuote() const final { return openQuote; }
  T getCloseQuote() const final { return closeQuote; }
  bool isEscaped() const final { return escaped; }
  virtual void handleEscape(T, T &, InputStream<T> &) {}

  void reset() {
    escaped = false;
    openQuote = 0;
    closeQuote = 0;
  }

  QuoteAndEscapeHandler(matcherFunction function)
      : matcher(validMatcherFunction(function)), escaped(false), openQuote(0),
        closeQuote(0) {}

  QuoteAndEscapeHandler(const T *quotes)
      : QuoteAndEscapeHandler(
            charClass::QuoteMatchers::getDefaultMatcherFor(quotes, nullptr)) {}

protected:
  void handle(T c, T &result, InputStream<T> &stream) {
    if (openQuote != 0) {
      if (escaped) {
        handleEscape(c, result, stream);
        escaped = false;
      } else if (c == '\\') {
        escaped = true;
      } else if (c == closeQuote) {
        openQuote = 0;
        closeQuote = 0;
      }
    } else if (escaped) {
      handleEscape(c, result, stream);
      escaped = false;
    } else if (matcher(c, closeQuote)) {
      openQuote = c;
    } else if (c == '\\') {
      escaped = true;
    }
    result = c;
  }

private:
  matcherFunction matcher;

  bool escaped = false;
  T openQuote = 0;
  T closeQuote = 0;

  static matcherFunction validMatcherFunction(matcherFunction f) {
    if (f != nullptr) {
      return f;
    }
    throw std::invalid_argument(
        "QuoteAndEscapeHandler: require quote matcher function.");
  }
};

template <typename T>
class QuotedStateStream : public QuoteAndEscapeHandler<T>,
                          public util::InputStream<T> {

public:
  using matcherFunction = typename charClass::QuoteMatcher<T>::function;

  QuotedStateStream(util::InputStream<T> &stream, matcherFunction function)
      : QuoteAndEscapeHandler<T>(function), input(stream) {}

  QuotedStateStream(util::InputStream<T> &stream, const T *quotes)
      : QuoteAndEscapeHandler<T>(quotes), input(stream) {}

  bool get(T &result) override {
    T c;
    if (!input.get(c)) {
      return false;
    }
    QuoteAndEscapeHandler<T>::handle(c, result, input);
    return true;
  }

private:
  util::InputStream<T> &input;
};

enum class MatchCommentStartResult { True, False, Continue };

template <typename T>
class CommentStream : public InputStream<T>, public QuoteAndEscapeHandler<T> {
  class Replay {
    int position = -1;
    int replayPos = 0;
    T nonComment = 0;
    const T *comment = nullptr;

  public:
    bool next(T &result) {
      if (position >= 0) {
        if (position < replayPos) {
          result = comment[position++];
        } else {
          result = nonComment;
          reset();
        }
        return true;
      }
      return false;
    }

    void start(const T *commentString, int failPosition,
               T firstNonCommentCharacter) {
      if (commentString == nullptr) {
        *this = {};
      } else {
        comment = commentString;
        replayPos = failPosition;
        position = 1;
        nonComment = firstNonCommentCharacter ? firstNonCommentCharacter
                                              : commentString[replayPos];
      }
    }

    void reset() { *this = {}; }
  } replay;

  class Nesting {
    const unsigned nesting;
    unsigned level;
    unsigned inLine;

  public:
    Nesting(unsigned nestingLevels)
        : nesting(nestingLevels + 1), level(0), inLine(0) {}

    bool nestingAllowed() { return level < nesting; }

    void startBlockComment() { level = 1; }

    void startLineComment() { inLine = 1; }

    void endLineComment() { inLine = 0; }

    bool pushNestingLevel() {
      if (nestingAllowed()) {
        level++;
        return true;
      }
      return false;
    }

    unsigned getLevel() const { return level + inLine; }

    bool popNestingLevelGetDone() { return level > 0 && --level == 0; }
  } nesting;

  const T *const lineComment;
  const T *const blockComment;
  const int commentEnd;
  InputStream<T> &input;

  static int getCommentEnd(const T *comment) {
    int commentEnd;
    for (commentEnd = 0; comment[commentEnd] != '\0'; commentEnd++)
      ;
    return commentEnd - 1;
  }

  static const T *validCommentString(const T *value, const T *lineComment) {
    if (value != nullptr && lineComment != nullptr) {
      int i;
      for (i = 0; value[i] != '\0' && lineComment[i] != '\0'; i++) {
        if (value[i] != lineComment[i]) {
          return value;
        }
      }
      if (lineComment[i] == '\0' && value[i] != '\0') {
        throw std::invalid_argument("CommentStream: Block-comment that starts "
                                    "with line-comment will never be used.");
      }
    }
    return value;
  }

  bool readUntilEndOfLineComment(T &result) {
    nesting.startLineComment();
    T c;
    while (input.get(c)) {
      if (c == '\n') {
        result = '\n';
        nesting.endLineComment();
        return true;
      }
    }
    nesting.endLineComment();
    return false;
  }

  bool readUntilEndOfBlock(T &result) {
    nesting.startBlockComment();
    int commentPos = commentEnd;
    bool holdLast = false;
    T c;
    while (holdLast || input.get(c)) {
      holdLast = false;
      if (c == blockComment[commentPos]) {
        commentPos--;
        if (commentPos < 0) {
          bool done = nesting.popNestingLevelGetDone();
          if (!input.get(c)) {
            return false;
          }
          if (done) {
            result = c;
            return true;
          }
          commentPos = commentEnd;
          holdLast = true;
        }
      } else if (nesting.nestingAllowed()) {
        int pos;
        for (pos = 0; blockComment[pos] != '\0' && c == blockComment[pos];
             pos++) {
          if (!input.get(c)) {
            return false;
          }
        }
        if (pos > 0 && blockComment[pos] == '\0') {
          nesting.pushNestingLevel();
          commentPos = commentEnd;
        }
      }
    }
    return false;
  }

  MatchCommentStartResult matchCommentStart(T chr, T &result) {
    T c = chr;
    const T *comment = nullptr;
    bool matchLine = lineComment && lineComment[0] && lineComment[0] == c;
    bool matchBlock = blockComment && blockComment[0] && blockComment[0] == c;
    int pos;
    if (!(matchLine || matchBlock)) {
      pos = 0;
    }
    else {
      pos = 1;
      while (true) {
        comment = matchLine ? lineComment : blockComment;
        if (input.get(c)) {
          if (matchLine) {
            if ((matchLine = lineComment[pos] && lineComment[pos] == c)) {
              if (lineComment[pos +1] == '\0') {
                return readUntilEndOfLineComment(result)
                           ? MatchCommentStartResult::True
                           : MatchCommentStartResult::False;
              }
            }
          }
          if (matchBlock) {
            if ((matchBlock = blockComment[pos] && blockComment[pos] == c)) {
              if (blockComment[pos + 1] == '\0') {
                return readUntilEndOfBlock(result)
                           ? MatchCommentStartResult::True
                           : MatchCommentStartResult::False;
              }
            }
          }
          if (!(matchLine || matchBlock)) {
            break;
          }
        } else {
          if (pos == 0) {
            result = comment[0];
            return MatchCommentStartResult::True;
          } else if (matchLine || matchBlock) {
            result = comment[pos];
            return MatchCommentStartResult::True;
          } else if (comment[pos + 1] == '\0') {
            return MatchCommentStartResult::False;
          } else {
            result = comment[0];
            replay.start(comment, pos, 0);
          }
          return MatchCommentStartResult::True;
        }
        pos++;
      }
      replay.start(comment, pos, c);
      result = comment[0];
      return MatchCommentStartResult::True;
    }
    return MatchCommentStartResult::Continue;
  }

public:
  CommentStream(InputStream<T> &stream, const T *lineCommentString,
                const T *blockCommentString, unsigned nestingLevels,
                typename charClass::QuoteMatcher<T>::function quoteMatcher)
      : QuoteAndEscapeHandler<T>(quoteMatcher), nesting(nestingLevels),
        lineComment(lineCommentString),
        blockComment(validCommentString(blockCommentString, lineComment)),
        commentEnd(getCommentEnd(blockComment)), input(stream) {}

  CommentStream(InputStream<T> &stream, const T *lineCommentString,
                const T *blockCommentString, unsigned nestingLevels,
                const T *symmetricQuotesString)
      : CommentStream(stream, lineCommentString, blockCommentString,
                      nestingLevels,
                      charClass::QuoteMatchers::getDefaultMatcherFor(
                          symmetricQuotesString, nullptr)) {}

  unsigned getLevel() const { return nesting.getLevel(); }
  bool inComment() const { return getLevel() != 0; }

  bool get(T &result) override {
    if (replay.next(result)) {
      return true;
    }
    T c;
    if (!input.get(c)) {
      return false;
    }
    QuoteAndEscapeHandler<T>::handle(c, result, input);
    if (QuoteAndEscapeHandler<T>::isEscaped() ||
        QuoteAndEscapeHandler<T>::inQuote()) {
      return true;
    }
    switch (matchCommentStart(c, result)) {
    case MatchCommentStartResult::True:
      return true;
    case MatchCommentStartResult::False:
      return false;
    default:
      result = c;
      return true;
    }
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_TEXTFILTERS_H
