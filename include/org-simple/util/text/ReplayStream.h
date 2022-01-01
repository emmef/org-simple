#ifndef ORG_SIMPLE_UTIL_TEXT_M_REPLAY_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT_M_REPLAY_STREAM_H
/*
 * org-simple/util/text/ReplayStream.h
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

#include <cstddef>
#include <org-simple/util/text/InputStream.h>

namespace org::simple::util::text {

/**
 * Defines an input stream that replays a maximum of \N characters of type \c C,
 * that can be added via the << operator or the append() function.
 * @tparam C The type of character to replay.
 * @tparam N The maximum number of characters that can be replayed.
 */
template <typename C, std::size_t N = 1> class ReplayStream;

template <typename C> class ReplayStream<C, 1> : public InputStream<C> {
  bool replays = false;
  C v = 0;

public:
  bool get(C &result) override {
    if (replays) {
      replays = false;
      result = v;
      return true;
    }
    return false;
  }

  void add(C value) {
    replays = true;
    v = value;
  }

  void operator<<(C value) { add(value); }
};

template <typename C, std::size_t N>
class ReplayStream : public InputStream<C> {
  static_assert(N > 1 &&
                N < std::numeric_limits<std::size_t>::max() / sizeof(C));

  std::size_t wr = 0;
  std::size_t sz = 0;
  C v[N];

public:
  bool get(C &result) override {
    if (sz) {
      std::size_t i = (wr + N - sz) % N;
      result = v[i];
      sz--;
      return true;
    }
    return false;
  }

  ReplayStream &add(C value) {
    if (sz < N) {
      v[wr] = value;
      wr = (++wr % N);
      sz++;
    }
    return *this;
  }

  ReplayStream &operator<<(C value) { return add(value); }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_REPLAY_STREAM_H
