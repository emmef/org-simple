#ifndef ORG_SIMPLE_STRINGSTREAM_H
#define ORG_SIMPLE_STRINGSTREAM_H
/*
 * org-simple/util/StringStream.h
 *
 * Added by michel on 2021-12-20
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
#include <org-simple/util/InputStream.h>
#include <algorithm>
#include <string>

namespace org::simple::util {

template <typename T>
class CStringInputStream : public org::simple::util::InputStream<T> {
  const T *input;
  int pos;

public:
  explicit CStringInputStream(const T *string) : input(string), pos(0) {}

  bool get(T &result) override {
    T c = input[pos];
    if (c != '\0') {
      result = c;
      ++pos;
      return true;
    }
    return false;
  }

  void rewind() { pos = 0; }

  void set(const T *string) {
    input = string;
    rewind();
  }

  const T *getCString() const { return input; }
};

template <typename T>
class StringInputStream : public org::simple::util::InputStream<T> {
  std::basic_string<T> input;
  size_t pos;

public:
  explicit StringInputStream(const std::basic_string<T> &string)
      : input(string), pos(0) {}
  explicit StringInputStream(const T *string) : input(string), pos(0) {}
  StringInputStream(StringInputStream &&) = default;

  bool get(T &result) override {
    if (pos < input.length()) {
      result = input[pos];
      ++pos;
      return true;
    }
    return false;
  }

  void rewind() { pos = 0; }

  void set(const T *string) {
    input = string;
    rewind();
  }

  void set(const std::basic_string<T> &string) {
    input = string;
    rewind();
  }

  const T *getCString() const { return input.c_str(); }

  const std::basic_string<T> &getString() const { return input; }
};

template <typename T> class InputCollector {
  std::basic_string<T> output;
  const std::size_t maxLength;

public:
  InputCollector(std::size_t maximumLength)
      : maxLength(std::max(maximumLength, 2lu)) {}

  void reset() { output.clear(); }

  size_t consume(InputStream<T> &stream) {
    size_t count = 0;
    while (output.length() < maxLength) {
      T c;
      if (stream.get(c)) {
        output += c;
        count++;
      } else {
        break;
      }
    }
    return count;
  }

  const T *getCString() const { return output.c_str(); }

  const std::basic_string<T> &getString() const { return output; }

  bool isFull() const { return maxLength == output.length(); }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_STRINGSTREAM_H
