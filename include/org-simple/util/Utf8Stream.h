#ifndef ORG_SIMPLE_UTF8STREAM_H
#define ORG_SIMPLE_UTF8STREAM_H
/*
 * org-simple/util/Utf8Stream.h
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
#include <org-simple/util/Characters.h>
#include <org-simple/util/CharEncode.h>
#include <org-simple/util/InputStream.h>

namespace org::simple::charEncode {
template <typename T>
class AsciiNewLineStream : public util::InputStream<T> {
  util::InputStream<T> &input;
public:
  AsciiNewLineStream(util::InputStream<T> &stream) : input(stream) {}

  bool get(T &result) override {
    T c;
    if (input.get(c)) {
      if (c != '\r' && charClass::Utf8::isLineBreak(c)) {
        result = '\n';
      }
      else {
        result = c;
      }
      return true;
    }
    return false;
  }

};

template <>
class AsciiNewLineStream<char> : public util::InputStream<char> {
  util::InputStream<char> &input;
public:
  AsciiNewLineStream(util::InputStream<char> &stream) : input(stream) {}

  bool get(char &result) override {
    return input.get(result);
  }

};

class ValidatedUtf8Stream : public util::InputStream<char> {
  util::InputStream<char> &input;
  char8_t buffer[5];
  signed pos;

  bool readCodePoint(char c) {
    Utf8Encoding ::codePoint v;
    int bytes = Utf8Encoding ::getBytesToReadSetInitialReaderValue(c, v);
    if (bytes == 0) {
      return true;
    }
    buffer[0] = c;
    int i;
    for (i = 1; i < bytes; i++) {
      if (!input.get(c)) {
        return false;
      }
      if (!Utf8Encoding::Continuation ::is(c)) {
        return true;
      }
      v <<= Utf8Encoding::Continuation::valueBits;
      v |= Utf8Encoding::Continuation::valueFrom(c);
      buffer[i] = c;
    }
    buffer[i] = 0;
    if (v <= Utf8Encoding::maximumCodePoint) {
      pos = 0;
    } // if invalid v, continue and skip invalid code point
    return true;
  }
public:
  ValidatedUtf8Stream(util::InputStream<char> &stream) : input(stream), pos(-1) {}
  
  bool get(char &result) override {
    while (true) {
      if (pos >= 0) {
        char8_t replay = buffer[pos];
        if (replay != '\0') {
          result = replay;
          ++pos;
          return true;
        }
        else {
          pos = -1;
        }
      }
      char c;
      if (!input.get(c)) {
        return false;
      }
      if ((0x7f & c) == 0) {
        result = c;
        return true;
      }
      if (!readCodePoint(c)) {
        return false;
      }
    }
  }
};

} // namespace org::simple

#endif // ORG_SIMPLE_UTF8STREAM_H
