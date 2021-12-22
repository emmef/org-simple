#ifndef ORG_SIMPLE_COMMENTSTREAM_H
#define ORG_SIMPLE_COMMENTSTREAM_H
/*
 * org-simple/util/CommentStream.h
 *
 * Added by michel on 2021-12-22
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
#include <org-simple/util/QuoteState.h>

namespace org::simple::util {
template <typename T>
class CommentStream : public QuoteState<T>, public InputStream<T> {
  class Replay {
    int position = -1;
    int replayPos = 0;
    T nonComment = 0;
    const T *comment = nullptr;

    void startBasic(const T *commentString, int failPosition,
                    T firstNonCommentCharacter) {
      if (commentString == nullptr) {
        *this = {};
      } else {
        comment = commentString;
        replayPos = failPosition;
        position = 1;
        nonComment = firstNonCommentCharacter;
      }
    }


  public:
    bool next(T &result) {
      if (available()) {
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

    bool available() { return position >= 0; }

    void start(const T *commentString, int failPosition,
               T firstNonCommentCharacter) {
      comment = commentString;
      replayPos = failPosition;
      position = 1;
      nonComment = firstNonCommentCharacter;
    }

    void start(const T *commentString, int failPosition) {
      start(commentString, failPosition, commentString[replayPos]);
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

  TextFilterResult matchCommentStart(T &result) {
    T c = result;
    const T *comment = nullptr;
    bool matchLine = lineComment && lineComment[0] && lineComment[0] == c;
    bool matchBlock = blockComment && blockComment[0] && blockComment[0] == c;
    int pos;
    if (!(matchLine || matchBlock)) {
      pos = 0;
    } else {
      pos = 1;
      while (true) {
        comment = matchLine ? lineComment : blockComment;
        if (input.get(c)) {
          if (matchLine) {
            if ((matchLine = lineComment[pos] && lineComment[pos] == c)) {
              if (lineComment[pos + 1] == '\0') {
                return readUntilEndOfLineComment(result)
                           ? TextFilterResult::True
                           : TextFilterResult::False;
              }
            }
          }
          if (matchBlock) {
            if ((matchBlock = blockComment[pos] && blockComment[pos] == c)) {
              if (blockComment[pos + 1] == '\0') {
                return readUntilEndOfBlock(result)
                           ? TextFilterResult::True
                           : TextFilterResult::False;
              }
            }
          }
          if (!(matchLine || matchBlock)) {
            break;
          }
        } else {
          if (pos == 0) {
            result = comment[0];
            return TextFilterResult::True;
          } else if (matchLine || matchBlock) {
            result = comment[pos];
            return TextFilterResult::True;
          } else if (comment[pos + 1] == '\0') {
            return TextFilterResult::False;
          } else {
            result = comment[0];
            replay.start(comment, pos);
          }
          return TextFilterResult::True;
        }
        pos++;
      }
      replay.start(comment, pos, c);
      result = comment[0];
      return TextFilterResult::True;
    }
    return TextFilterResult::Continue;
  }

public:
  template <typename Q>
  CommentStream(InputStream<T> &stream, const T *lineCommentString,
                const T *blockCommentString, unsigned nestingLevels, Q quotes)
      : QuoteState<T>(quotes), nesting(nestingLevels),
        lineComment(lineCommentString),
        blockComment(validCommentString(blockCommentString, lineComment)),
        commentEnd(getCommentEnd(blockComment)), input(stream) {}

  unsigned getLevel() const { return nesting.getLevel(); }
  bool inComment() const { return getLevel() != 0; }

  bool get(T &result) override {
    if (replay.next(result)) {
      return true;
    }
    if (!input.get(result)) {
      return false;
    }
    QuoteState<T>::probe(result);
    if (QuoteState<T>::isEscaped() || QuoteState<T>::inQuote()) {
      return true;
    }
    switch (matchCommentStart(result)) {
    case TextFilterResult::False:
      return false;
    default:
      return true;
    }
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_COMMENTSTREAM_H
