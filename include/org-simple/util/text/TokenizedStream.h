#ifndef ORG_SIMPLE_UTIL_TEXT_M_TOKENIZED_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT_M_TOKENIZED_STREAM_H
/*
 * org-simple/util/text/TokenizedStream.h
 *
 * Added by michel on 2022-01-01
 * Copyright (C) 2015-2022 Michel Fleur.
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

#include <org-simple/util/text/ReplayStream.h>
#include <functional>

namespace org::simple::util::text {

/**
 * Based on an underlying stream, which is implementation-specific, this
 * stream's \c get returns \c false if the end of a "token" is reached, which is
 * also implementation-specific. The stream can be used again for the next
 * token, unless the underlying stream is exhausted, as reported by the \c
 * exhausted function.
 * @tparam C The type of character the stream
 */
template <typename C> class TokenizedInputStream : public InputStream<C> {
public:
  virtual bool isExhausted() const = 0;
  /**
   * Resets the exhausted-state, which is useful if this token stream is based
   * upon yet another token stream.
   */
  virtual void resetExhausted() = 0;
};

template <typename C, class S = InputStream<C>>
class PredicateTokenStream : public TokenizedInputStream<C> {
  enum class State { Skip, Scan };
  std::function<bool(const C &)> tokenPredicate;
  std::function<bool(const C &)> skipPredicate;
  S &input;
  ReplayStream<C, 1> replay;
  State state = State::Skip;
  bool exhausted = false;

public:
  template <class P1, class P2>
  PredicateTokenStream(S &stream, P1 p1, P2 p2)
      : input(stream), tokenPredicate(p1), skipPredicate(p2) {}

  bool get(C &result) override {
    C c;
    if (replay.get(c)) {
      result = c;
      return true;
    }
    while (replay.get(c) || input.get(c)) {
      exhausted = false;
      switch (state) {
      case State::Skip:
        if (!skipPredicate(c)) {
          if (tokenPredicate(c)) {
            return false;
          }
          state = State::Scan;
          replay << c;
        }
        break;
      case State::Scan:
        if (tokenPredicate(c)) {
          state = State::Skip;
          return false;
        }
        if (skipPredicate(c)) {
          state = State::Skip;
        } else {
          result = c;
          return true;
        }
        break;
      }
    }
    exhausted = true;
    return false;
  }

  bool isExhausted() const override { return exhausted; }
  void resetExhausted() override { exhausted = false; state=State::Skip; }
};



} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_TOKENIZED_STREAM_H
