#ifndef ORG_SIMPLE_UTIL_TEXT_M_LINE_CONTINUATION_H
#define ORG_SIMPLE_UTIL_TEXT_M_LINE_CONTINUATION_H
/*
 * org-simple/util/text/LineContinuation.h
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
#include <org-simple/util/text/StreamFilter.h>

namespace org::simple::util::text {

template <typename C>
class LineContinuationFilter : public StreamFilter<C> {
  enum class State { Normal, Marked, ReturnNext };
  State state = State::Normal;
  C next = 0;

public:
  void reset() { *this = {}; }

  bool get(C &c) {
    if (state == State::ReturnNext) {
      c = next;
      return true;
    }
    return false;
  }

  InputFilterResult filter(C &c) final {
    if (state == State::ReturnNext) {
      state = State::Normal;
      return InputFilterResult::Ok;
    }
    if (state == State::Normal) {
      if (c == '\\') {
        state = State::Marked;
        return InputFilterResult::GetNext;
      } else {
        return InputFilterResult::Ok;
      }
    }
    if (state == State::Marked) {
      if (c == '\n') {
        state = State::Normal;
        return InputFilterResult::GetNext;
      } else {
        next = c;
        state = State::ReturnNext;
        c = '\\';
        return InputFilterResult::Ok;
      }
    }
    return InputFilterResult::Ok;
  }
};

template <typename C>
class LineContinuationStream : public InputStream<C> {
  InputStream<C> &input;
  LineContinuationFilter<C> filter;

public:
  explicit LineContinuationStream(InputStream<C> &stream)
      : input(stream) {}

  bool get(C &result) final { return applyInputFilter(filter, input, result); }
  void reset() { filter.reset(); }
};


} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_LINE_CONTINUATION_H
