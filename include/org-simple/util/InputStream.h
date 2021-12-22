#ifndef ORG_SIMPLE_INPUTSTREAM_H
#define ORG_SIMPLE_INPUTSTREAM_H
/*
 * org-simple/util/InputStream.h
 *
 * Added by michel on 2021-12-15
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

#include<limits>

namespace org::simple::util {

enum class StreamState { OK, END };

template <typename T> class InputStream {
public:
  virtual bool get(T &) = 0;
  virtual ~InputStream() = default;

};

template<typename T>
class EmptyInputStream : public InputStream<T>{
public:
  static InputStream<T> *instance() {
    static EmptyInputStream<T> instance;
    return &instance;
  }

  bool get(T &) final { return false; }
};

template <typename T>
class DeadPillStream : public EmptyInputStream<T> {

public:
  static InputStream<T> *instance() {
    static DeadPillStream<T> instance;
    return &instance;
  }
};


template <typename T> class ReplayStream : public InputStream<T> {
  InputStream<T> *input = nullptr;

public:
  bool get(T &c) final {
    if (input == nullptr) {
      return false;
    }
    if (input->get(c)) {
      return true;
    }
    input = nullptr;
    return false;
  }

  bool available() final {
    return input && input->available();
  }

  bool assignedStream(InputStream<T> *stream) {
    input = stream;
    return stream != DeadPillStream<T>::instance();
  }
};

template <typename T, unsigned N>
class ReplayCharacterStream : public InputStream<T> {
  static_assert(N > 0 &&
                N < std::numeric_limits<unsigned>::max() / sizeof(T));

  unsigned replayCount = 0;
  T v[N];

public:
  bool get(T &result) final {
    if (replayCount) {
      replayCount--;
      result = v[N - 1 - replayCount];
      return true;
    }
    return false;
  }

  bool available() final {
    return replayCount > 0;
  }

  ReplayCharacterStream &operator << (T value) {
    if (replayCount < N) {
      v[replayCount++] = value;
    }
    return *this;
  }
};



} // namespace org::simple::util

#endif // ORG_SIMPLE_INPUTSTREAM_H
