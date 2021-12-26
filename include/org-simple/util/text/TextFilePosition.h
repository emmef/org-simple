#ifndef ORG_SIMPLE_UTIL_TEXT_M_TEXT_FILE_POSITION_H
#define ORG_SIMPLE_UTIL_TEXT_M_TEXT_FILE_POSITION_H
/*
 * org-simple/util/text/TextFilePosition.h
 *
 * Added by michel on 2021-12-24
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
template <typename C> class TextFilePositionData {
  std::size_t line = 0;
  std::size_t position = 0;
  std::size_t column = 0;

public:
  void probe(const C &c) {
    position++;
    if (c == '\n') {
      line++;
      column = 0;
    } else {
      column++;
    }
  }

  void reset() {
    *this = {};
  }

  std::size_t getPosition() const { return position; }
  std::size_t getLine() const { return line; }
  std::size_t getColumn() const { return column; }
};


} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_TEXT_FILE_POSITION_H
