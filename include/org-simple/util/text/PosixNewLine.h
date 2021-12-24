#ifndef ORG_SIMPLE_POSIXNEWLINE_H
#define ORG_SIMPLE_POSIXNEWLINE_H
/*
 * org-simple/util/text/PosixNewLine.h
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

template <typename C> class ToPosixNewlineFilter {
  bool lastCR = false;

public:
  void reset() { *this = {}; }

  TextFilterResult directFilter(C &result) {
    if (result == '\n') {
      if (lastCR) {
        lastCR = false;
        return TextFilterResult::GetNext;
      } else {
        result = '\n';
        return TextFilterResult::Ok;
      }
    } else if (result == '\r') {
      lastCR = true;
      result = '\n';
      return TextFilterResult::Ok;
    } else {
      lastCR = false;
      return TextFilterResult::Ok;
    }
  }

  typedef AbstractInputFilter<ToPosixNewlineFilter, C> Interface;
};

template <typename C> class TextFilePositionData {
  std::size_t line = 0;
  std::size_t position = 0;
  std::size_t column = 0;

  void probe(const C &c) {
    position++;
    if (c == '\n') {
      line++;
      column = 0;
    } else {
      column++;
    }
  }
};

template <typename C>
using NewlinePositionProbe = AbstractInputProbe<C, TextFilePositionData<C>>;

template <typename C> class PosixNewlineStream : public util::InputStream<C> {
  util::InputStream<C> &input;
  ToPosixNewlineFilter<C> filter;

public:
  explicit PosixNewlineStream(util::InputStream<C> &stream) : input(stream) {}

  const ToPosixNewlineFilter<C> &state() { return filter; }
  bool get(C &result) final { return applyFilter(result, filter, input); }
  void reset() { filter.reset(); }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_POSIXNEWLINE_H
