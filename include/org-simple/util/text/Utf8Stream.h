#ifndef ORG_SIMPLE_UTIL_TEXT__UTF8_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT__UTF8_STREAM_H
/*
 * org-simple/util/text/Utf8Stream.h
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
#include <org-simple/util/text/CharEncode.h>
#include <org-simple/util/text/Characters.h>
#include <org-simple/util/text/InputStream.h>

namespace org::simple::util::text {
template <typename T> class AsciiNewLineStream : public InputStream<T> {
  InputStream<T> &input;

public:
  AsciiNewLineStream(InputStream<T> &stream) : input(stream) {}

  bool get(T &result) override {
    T c;
    if (input.get(c)) {
      if (c != '\r' && Unicode::isLineBreak(c)) {
        result = '\n';
      } else {
        result = c;
      }
      return true;
    }
    return false;
  }
};

template <> class AsciiNewLineStream<char> : public InputStream<char> {
  InputStream<char> &input;

public:
  AsciiNewLineStream(InputStream<char> &stream) : input(stream) {}

  bool get(char &result) override { return input.get(result); }
};

class ValidatedUtf8Stream : public InputStream<char> {
  InputStream<char> &input;
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
  ValidatedUtf8Stream(InputStream<char> &stream)
      : input(stream), pos(-1) {}

  bool get(char &result) override {
    while (true) {
      if (pos >= 0) {
        char8_t replay = buffer[pos];
        if (replay != '\0') {
          result = replay;
          ++pos;
          return true;
        } else {
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

template <typename C>
class Utf8ToUnicodeStream : public InputStream<Utf8Encoding::codePoint> {
  static constexpr bool isSigned = std::is_same_v<char, C>;
  static_assert(isSigned || std::is_same_v<Utf8Encoding::byte, C>);
  InputStream<C> &input;
  Utf8Encoding::Reader reader;

public:
  Utf8ToUnicodeStream(InputStream<C> &source) : input(source) {}

  bool get(Utf8Encoding::codePoint &result) final {
    C byte;
    while (input.get(byte)) {
      if (reader.addGetState(byte) == DecodingReaderState::OK) {
        result = reader.getValueAndReset();
        return true;
      }
    }
    return false;
  }

  void reset() {
    reader.reset();
  }
};


template <typename C>
class UnicodeToUtf8Stream : public InputStream<C> {
  static constexpr bool isSigned = std::is_same_v<char, C>;
  static_assert(isSigned || std::is_same_v<Utf8Encoding::byte, C>);
  InputStream<Utf8Encoding::codePoint> &input;
  C buffer[5];
  int pos = -1;
  int end = 0;

public:
  UnicodeToUtf8Stream(InputStream<Utf8Encoding::codePoint> &source) : input(source) {}

  bool get(C &result) final {
    if (pos >= 0) {
      result = buffer[pos++];
      if (pos == end) {
        pos = -1;
      }
      return true;
    }
    Utf8Encoding::codePoint cp;
    if (!input.get(cp)) {
      return false;
    }
    end = Utf8Encoding ::unsafeEncode(cp, buffer) - buffer;
    result = buffer[0];
    if (end > 1) {
      pos = 1;
    }
    return true;
  }

  void reset() {
    pos = -1;
  }
};

typedef Utf8ToUnicodeStream<char> Utf8CharToUnicodeStream;
typedef UnicodeToUtf8Stream<char> UnicodeToUtf8CharStream;

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT__UTF8_STREAM_H
