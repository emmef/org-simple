#ifndef ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAM_H
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
requires(std::is_base_of_v<InputStream<C>, S>) S &wrapAsInputStream(
    S &wrapped) {
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

template <typename C, class S = InputStream<C>> class EchoStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  S &input;
  C v = 0;

public:
  EchoStream(S &stream) : input(stream) {}

  bool get(C &result) final {
    if (input.get(v)) {
      result = v;
      return true;
    }
    return false;
  }

  C getMostRecent() const { return v; }
};

template <typename C, class S = InputStream<C>> class EchoRepeatOneStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  S &input;
  bool mustRepeat = false;
  C v = 0;

public:
  EchoRepeatOneStream(S &stream) : input(stream) {}

  bool get(C &result) override {
    if (mustRepeat) {
      result = v;
      mustRepeat = false;
      return true;
    }
    if (input.get(result)) {
      v = result;
      return true;
    }
    return false;
  }

  C &lastValue() { return v; }
  C getMostRecent() const { return v; }
  void repeat() { mustRepeat = true; }
};

template <typename C, bool resetWhenExhausted, class S = InputStream<C>>
class VariableEchoStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  S *input = nullptr;

public:
  VariableEchoStream() {}
  VariableEchoStream(S * stream) { input = stream; }

  bool get(C &c) override {
    if (input) {
      if (input->get(c)) {
        return true;
      }
      if constexpr (resetWhenExhausted) {
        input = nullptr;
      }
    }
    return false;
  }

  void assignStream(S *stream) { input = stream; }
};

template <typename C, bool resetWhenExhausted, class S = InputStream<C>>
class VariableEchoRepeatOneStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  S *input = nullptr;
  bool mustRepeat = false;
  C v = 0;

public:
  VariableEchoRepeatOneStream() {}
  VariableEchoRepeatOneStream(S * stream) { input = stream; }

  bool get(C &result) override {
    if (mustRepeat) {
      result = v;
      mustRepeat = false;
      return true;
    }
    if (input) {
      if (input->get(v)) {
        result = v;
        return true;
      }
      if constexpr (resetWhenExhausted) {
        input = nullptr;
      }
    }
    return false;
  }

  void assignStream(S *stream) { input = stream; }
  C getMostRecent() const { return v; }
  void repeat() { mustRepeat = true; }
};

template <typename C, unsigned N>
class ReplayStream : public InputStream<C> {
  static_assert(N > 0 && N < std::numeric_limits<unsigned>::max() / sizeof(C));

  unsigned replayCount = 0;
  C v[N];

public:
  bool get(C &result) override {
    if (replayCount) {
      replayCount--;
      result = v[N - 1 - replayCount];
      return true;
    }
    return false;
  }

  ReplayStream &operator<<(C value) {
    if (replayCount < N) {
      v[replayCount++] = value;
    }
    return *this;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAM_H
