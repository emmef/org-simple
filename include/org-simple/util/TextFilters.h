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

enum class EscapeHandlerResult { IGNORED, HANDLED, BUSY };
template <typename T> class QuoteAndEscapeState {
public:
  virtual bool inQuote() const = 0;
  virtual T getOpenQuote() const = 0;
  virtual T getCloseQuote() const = 0;
  virtual bool isEscaped() const = 0;
  virtual ~QuoteAndEscapeState() = default;
  virtual EscapeHandlerResult handleEscape(T, T &, InputStream<T> &) {
    return EscapeHandlerResult::IGNORED;
  }
};

template <typename T>
class QuoteAndEscapeHandler : public QuoteAndEscapeState<T> {
public:
  using matcherFunction = typename charClass::QuoteMatcher<T>::function;

  bool inQuote() const final { return openQuote != 0; }
  T getOpenQuote() const final { return openQuote; }
  T getCloseQuote() const final { return closeQuote; }
  bool isEscaped() const final { return escaped; }

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
  virtual EscapeHandlerResult handleEscape(T, T &, InputStream<T> &) {
    return EscapeHandlerResult::IGNORED;
  }

protected:
  void handle(T c, T &result, InputStream<T> &stream) {
    if (openQuote != 0) {
      if (escaped) {
        handleEscapeAndState(c, result, stream);
      } else if (c == '\\') {
        escaped = true;
      } else if (c == closeQuote) {
        openQuote = 0;
        closeQuote = 0;
      }
    } else if (escaped) {
      handleEscapeAndState(c, result, stream);
    } else if (matcher(c, closeQuote)) {
      openQuote = c;
    } else if (c == '\\') {
      escaped = true;
    }
    result = c;
  }

private:
  matcherFunction matcher;

  void handleEscapeAndState(T c, T &result, InputStream<T> &stream) {
    if (handleEscape(c, result, stream) != EscapeHandlerResult::BUSY) {
      escaped = false;
    }
  }

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

template <typename T>
class AbstractCommentStream : public QuoteAndEscapeState<T>,
                              public InputStream<T> {
  int position;
  int replay;
  T nonComment;
  unsigned level;
  const unsigned nesting;

protected:
  const T *const comment;
  InputStream<T> &input;

  virtual bool readUntilEnd(T &result, InputStream<T> &input) = 0;

  EscapeHandlerResult handleEscape(T c, T &result,
                                   InputStream<T> &stream) final {
    if (nestedState) {
      auto handled = inQuote() || inComment()
                         ? EscapeHandlerResult::IGNORED
                         : handleEscapeLocal(c, result, stream);
      if (handled != EscapeHandlerResult::IGNORED) {
        return handled;
      }
      return nestedState->handleEscape(c, result, stream);
    }
    return EscapeHandlerResult::IGNORED;
  }
  virtual EscapeHandlerResult handleEscapeLocal(T, T &, InputStream<T> &) {
    return EscapeHandlerResult::IGNORED;
  }

  bool nestingAllowed() { return level < nesting; }

  bool pushNestingLevel() {
    if (nestingAllowed()) {
      level++;
      return true;
    }
    return false;
  }

  bool popNestingLevelGetDone() { return level > 0 && --level == 0; }

public:
  template <class V>
  requires(std::is_base_of_v<InputStream<T>, V>)
      AbstractCommentStream(V &stream, const T *commentString,
                            unsigned nestingLevels = 0)
      : position(-1), replay(0), nonComment(0), level(0),
        nesting(nestingLevels + 1), comment(commentString), input(stream) {
    if constexpr (std::is_base_of_v<AbstractCommentStream<T>, V>) {
      nestedCommentStream = &stream;
    } else {
      nestedCommentStream = nullptr;
    }
    if constexpr (std::is_base_of_v<QuoteAndEscapeState<T>, V>) {
      nestedState = &stream;
    } else {
      nestedState = nullptr;
    }
  }

  bool inQuote() const final { return nestedState ? nestedState->inQuote() : false; }
  T getOpenQuote() const final { return nestedState ? nestedState->getOpenQuote() : 0; }
  T getCloseQuote() const final { return nestedState ? nestedState->getCloseQuote() : 0; }
  bool isEscaped() const final { return nestedState ? nestedState->isEscaped() : 0; }

  unsigned getLevel() const {
    return nestedCommentStream ? nestedCommentStream->getLevel() + level
                               : level;
  }
  bool inComment() const { return getLevel() != 0; }

  bool get(T &result) override {
    if (position >= 0) {
      if (position < replay) {
        result = comment[position++];
      } else {
        result = nonComment;
        position = -1;
      }
      return true;
    }
    T c;
    if (!input.get(c)) {
      return false;
    }
    if (inQuote() || isEscaped()) {
      result = c;
      return true;
    }
    int pos;
    for (pos = 0; comment[pos] != '\0' && c == comment[pos]; pos++) {
      if (!input.get(c)) {
        if (pos == 0) {
          result = comment[0];
          return true;
        } else if (comment[pos + 1] == '\0') {
          return false;
        } else {
          result = comment[0];
          nonComment = comment[pos];
          position = 1;
          replay = pos;
        }
        return true;
      }
    }
    if (pos == 0) {
      result = c;
      return true;
    }
    if (comment[pos] == '\0') {
      level = 1;
      return readUntilEnd(result, input);
    }
    replay = pos;
    nonComment = c;
    position = 1;
    result = comment[0];
    return true;
  }

private:
  QuoteAndEscapeState<T> *nestedState;
  const AbstractCommentStream<T> *nestedCommentStream;
};

template <typename T>
class LineCommentStream : public AbstractCommentStream<T> {
protected:
  using AbstractCommentStream<T>::popNestingLevelGetDone;

  bool readUntilEnd(T &result, InputStream<T> &input) override {
    T c;
    while (input.get(c)) {
      if (c == '\n') {
        result = '\n';
        popNestingLevelGetDone();
        return true;
      }
    }
    return false;
  }

public:
  template <class V>
  requires(std::is_base_of_v<InputStream<T>, V>)
      LineCommentStream(V &stream, const T *commentString)
      : AbstractCommentStream<T>(stream, commentString) {}
};

template <typename T>
class BlockCommentStream : public AbstractCommentStream<T> {

  int getCommentEnd() const {
    int commentEnd;
    for (commentEnd = 0; comment[commentEnd] != '\0'; commentEnd++)
      ;
    commentEnd--;
    return commentEnd;
  }

protected:
  using AbstractCommentStream<T>::comment;
  using AbstractCommentStream<T>::inQuote;
  using AbstractCommentStream<T>::isEscaped;
  using AbstractCommentStream<T>::nestingAllowed;
  using AbstractCommentStream<T>::pushNestingLevel;
  using AbstractCommentStream<T>::popNestingLevelGetDone;

  bool readUntilEnd(T &result, InputStream<T> &input) override {
    const int commentEnd = getCommentEnd();
    int commentPos = commentEnd;
    bool holdLast = false;
    T c;
    while (holdLast || input.get(c)) {
      holdLast = false;
      if (!inQuote() && !isEscaped()) {
        if (c == comment[commentPos]) {
          commentPos--;
          if (commentPos < 0) {
            bool done = popNestingLevelGetDone();
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
        } else if (nestingAllowed()) {
          int pos;
          for (pos = 0; comment[pos] != '\0' && c == comment[pos]; pos++) {
            if (!input.get(c)) {
              return false;
            }
          }
          if (pos > 0 && comment[pos] == '\0') {
            pushNestingLevel();
            commentPos = commentEnd;
          }
        }
      } else {
        commentPos = commentEnd;
      }
    }
    return false;
  }

public:
  template <class V>
  requires(std::is_base_of_v<InputStream<T>, V>)
      BlockCommentStream(V &stream, const T *commentString,
                         unsigned nestingLevels = 0)
      : AbstractCommentStream<T>(stream, commentString, nestingLevels) {}
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_TEXTFILTERS_H
