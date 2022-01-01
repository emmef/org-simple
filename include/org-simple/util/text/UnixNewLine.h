#ifndef ORG_SIMPLE_UTIL_TEXT_M_UNIX_NEW_LINE_H
#define ORG_SIMPLE_UTIL_TEXT_M_UNIX_NEW_LINE_H
/*
 * org-simple/util/text/UnixNewLine.h
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

#include <org-simple/util/text/TokenizedStream.h>
#include <org-simple/util/text/StreamFilter.h>
#include <org-simple/util/text/ReplayStream.h>

namespace org::simple::util::text {

template <typename C> class UnixNewLineFilter : public StreamFilter<C> {
  bool lastCR = false;

public:
  void reset() { *this = {}; }

  InputFilterResult filter(C &result) final {
    if (result == '\n') {
      if (lastCR) {
        lastCR = false;
        return InputFilterResult::GetNext;
      } else {
        result = '\n';
        return InputFilterResult::Ok;
      }
    } else if (result == '\r') {
      lastCR = true;
      result = '\n';
      return InputFilterResult::Ok;
    } else {
      lastCR = false;
      return InputFilterResult::Ok;
    }
  }
};

template <typename C, class S = InputStream<C>>
class UnixNewLineStream : public InputStream<C> {
  S &input;
  UnixNewLineFilter<C> filter;

public:
  explicit UnixNewLineStream(S &stream) : input(stream) {}

  const UnixNewLineFilter<C> &state() { return filter; }
  bool get(C &result) final { return applyInputFilter(filter, input, result); }
  void reset() { filter.reset(); }
};

template <typename C, class S = InputStream<C>>
class NewlineTokenizedStream : public TokenizedInputStream<C> {
  S &input;
  ReplayStream<C, 1> replay;
  bool exhausted = false;

public:
  explicit NewlineTokenizedStream(S &stream) : input(stream) {}

  bool get(C &result) final {
    C c;
    bool skipping = false;
    while (replay.get(c) || input.get(c)) {
      if (skipping) {
        if (c != '\n') {
          replay << c;
          return false;
        }
      } else if (c != '\n') {
        result = c;
        return true;
      } else {
        skipping = true;
      }
    }
    exhausted = true;
    return false;
  }

  bool isExhausted() const final { return exhausted; }

  void resetExhausted() {
    exhausted = false;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_UNIX_NEW_LINE_H
