#ifndef ORG_SIMPLE_INPUTSTREAM_H
#define ORG_SIMPLE_INPUTSTREAM_H
/*
 * org-simple/util/InputStream.h
 *
 * Added by michel on 2021-12-15
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

namespace org::simple::util {

enum class StreamState { OK, END };

template <typename T> class InputStream {
public:
  virtual bool get(T &) = 0;
  virtual ~InputStream() = default;
};

template <typename T>
class AbstractQuotedStateStream : public util::InputStream<T> {
public:
  virtual T inQuote() const { return false; }
  virtual bool isEscaped() const { return false; }
};

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

template <typename T>
class QuotedStateStream : public AbstractQuotedStateStream<T> {
  util::InputStream<T> &input;
  const T *const quotes;

  bool escaped = false;
  T quote = 0;

  bool isQuote(T c) const {
    for (const T *q = quotes; *q != 0; q++) {
      if (c == *q) {
        return true;
      }
    }
    return false;
  }

public:
  QuotedStateStream(util::InputStream<T> &stream, const T *quoteChars)
      : input(stream), quotes(quoteChars) {}

  T inQuote() const override { return quote; }
  bool isEscaped() const override { return escaped; }

  bool get(T &result) override {
    T c;
    if (!input.get(c)) {
      return false;
    }
    if (quote != 0) {
      if (escaped) {
        escaped = false;
      } else if (c == '\\') {
        escaped = true;
      } else if (c == quote) {
        quote = 0;
      }
    } else if (escaped) {
      escaped = false;
    } else if (isQuote(c)) {
      quote = c;
    } else if (c == '\\') {
      escaped = true;
    }
    result = c;
    return true;
  }
};

template <typename T>
class AbstractCommentStream : public AbstractQuotedStateStream<T> {
  int position;
  int replay;
  T nonComment;

protected:
  AbstractQuotedStateStream<T> &input;
  const T *const comment;
  bool inside;
  bool borrow_inside;

  virtual bool readUntilEnd(T &result) = 0;

public:
  AbstractCommentStream(AbstractQuotedStateStream<T> &stream,
                        const T *commentString)
      : position(-1), input(stream), comment(commentString), inside(false),
        borrow_inside(false) {}

  AbstractCommentStream(AbstractCommentStream<T> &stream,
                        const T *commentString)
      : position(-1), input(stream), comment(commentString), inside(false),
        borrow_inside(true) {}

  T inQuote() const override { return input.inQuote(); }
  bool isEscaped() const override { return input.isEscaped(); }
  bool inComment() const {
    return borrow_inside
               ? dynamic_cast<AbstractCommentStream &>(input).inComment() ||
                     inside
               : inside;
  }

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
    if (input.inQuote() || input.isEscaped()) {
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
      inside = true;
      return readUntilEnd(result);
    }
    replay = pos;
    nonComment = c;
    position = 1;
    result = comment[0];
    return true;
  }
};

template <typename T>
class LineCommentStream : public AbstractCommentStream<T> {
protected:
  using AbstractCommentStream<T>::input;
  using AbstractCommentStream<T>::inside;

  bool readUntilEnd(T &result) {
    T c;
    while (input.get(c)) {
      if (c == '\n') {
        result = '\n';
        inside = false;
        return true;
      }
    }
    return false;
  }

public:
  LineCommentStream(AbstractQuotedStateStream<T> &stream,
                    const T *commentString)
      : AbstractCommentStream<T>(stream, commentString) {}
};

template <typename T>
class BlockCommentStream : public AbstractCommentStream<T> {

protected:
  using AbstractCommentStream<T>::input;
  using AbstractCommentStream<T>::comment;
  using AbstractCommentStream<T>::inside;
  using AbstractCommentStream<T>::inQuote;
  using AbstractCommentStream<T>::isEscaped;


  bool readUntilEnd(T &result) {
    int commentEnd;
    for (commentEnd = 0; comment[commentEnd] != '\0'; commentEnd++)
      ;
    commentEnd--;
    int commentPos = commentEnd;
    T c;
    while (input.get(c)) {
      if (!inQuote() && !isEscaped() && c == comment[commentPos]) {
        commentPos--;
        if (commentPos < 0) {
          inside = false;
          if (!input.get(c)) {
            return false;
          }
          result = c;
          return true;
        }
      } else {
        commentPos = commentEnd;
      }
    }
    return false;
  }

public:
  BlockCommentStream(AbstractQuotedStateStream<T> &stream,
                     const T *commentString)
      : AbstractCommentStream<T>(stream, commentString) {}
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_INPUTSTREAM_H
