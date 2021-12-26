#ifndef ORG_SIMPLE_UTIL_TEXT__UNIX_NEW_LINE_H
#define ORG_SIMPLE_UTIL_TEXT__UNIX_NEW_LINE_H
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

#include <org-simple/util/text/InputFilter.h>

namespace org::simple::util::text {

template <typename C> class UnixNewLineFilter : public InputFilter<C> {
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

template <typename C> class UnixNewLineStream : public InputStream<C> {
  InputStream<C> &input;
  UnixNewLineFilter<C> filter;

public:
  explicit UnixNewLineStream(InputStream<C> &stream) : input(stream) {}

  const UnixNewLineFilter<C> &state() { return filter; }
  bool get(C &result) final { return applyInputFilter(filter, input, result); }
  void reset() { filter.reset(); }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT__UNIX_NEW_LINE_H
