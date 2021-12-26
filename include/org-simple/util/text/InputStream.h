#ifndef ORG_SIMPLE_UTIL_TEXT__INPUT_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT__INPUT_STREAM_H
/*
 * org-simple/util/text/InputStream.h
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

#include <limits>
#include <type_traits>

namespace org::simple::util::text {

template <typename C> class InputStream {

public:
  virtual bool get(C &) = 0;
  virtual ~InputStream() = default;

  struct Traits {
    template <class X>
    requires(std::is_same_v<
             bool,
             decltype(std::declval<X>().get(
                 std::declval<C &>()))>) static constexpr bool substGet(X *) {
      return true;
    }
    template <class X> static constexpr bool substGet(...) { return false; }

    template <class X> static constexpr bool isA() {
      return substGet<X>(static_cast<X *>(nullptr));
    }
  };

  static_assert(Traits::template isA<InputStream<C>>());

  template <class D> class Wrapped : public InputStream<C> {
    static_assert(Traits::template isA<D>());
    D *wrapped;

  public:
    Wrapped(D &stream) : wrapped(&stream) {}
    Wrapped(const Wrapped &) = default;
    Wrapped(Wrapped &&) = default;

    bool get(C &c) final { return wrapped->get(c); }
  };

  template <class D> class Interfaced : public virtual InputStream, public D {
    static_assert(Traits::template isA<D>() &&
                  !std::is_base_of_v<InputStream, D>);

  public:
    bool get(C &c) final { return D::get(c); }
  };
};

template <class S, typename C>
static constexpr bool
    hasInputStreamSignature = InputStream<C>::Traits::template isA<S>();

template <class S, typename C>
requires(!std::is_base_of_v<InputStream<C>, S> && hasInputStreamSignature<S, C>)
    typename InputStream<C>::template Wrapped<S> wrapAsInputStream(S &wrapped) {
  return typename InputStream<C>::template Wrapped<S>(wrapped);
}
template <class S, typename C>
requires(std::is_base_of_v<InputStream<C>, S>)
    S &wrapAsInputStream(S &wrapped) {
  return wrapped;
}

template <typename T> class EmptyInputStream : public InputStream<T> {
public:
  static InputStream<T> *instance() {
    static EmptyInputStream<T> instance;
    return &instance;
  }

  bool get(T &) final { return false; }
};

template <typename T> class DeadPillStream : public EmptyInputStream<T> {

public:
  static InputStream<T> *instance() {
    static DeadPillStream<T> instance;
    return &instance;
  }
};

template <typename T> class EchoStream : public InputStream<T> {
  InputStream<T> &input;

public:
  bool get(T &result) final { return input.get(result); }
  EchoStream(InputStream<T> &source) : input(source) {}
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

  bool assignedStream(InputStream<T> *stream) {
    input = stream;
    return stream != DeadPillStream<T>::instance();
  }
};

template <typename T, unsigned N>
class ReplayCharacterStream : public InputStream<T> {
  static_assert(N > 0 && N < std::numeric_limits<unsigned>::max() / sizeof(T));

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

  bool available() final { return replayCount > 0; }

  ReplayCharacterStream &operator<<(T value) {
    if (replayCount < N) {
      v[replayCount++] = value;
    }
    return *this;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT__INPUT_STREAM_H
