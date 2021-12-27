#ifndef ORG_SIMPLE_UTIL_TEXT_M_STREAM_PROBE_H
#define ORG_SIMPLE_UTIL_TEXT_M_STREAM_PROBE_H
/*
 * org-simple/util/text/StreamProbe.h
 *
 * Added by michel on 2021-12-27
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

#include <type_traits>

namespace org::simple::util::text {

template <typename C> class StreamProbe {
public:
  virtual void probe(const C &) = 0;
  virtual ~StreamProbe() = default;

  struct Traits {
    template <class X>
    requires(std::is_void_v<decltype(std::declval<X>().probe(
                 (const C &)std::declval<C &>()))>) //
        static constexpr bool substProbe(X *) {
      return true;
    }

    template <class X> static constexpr bool substProbe(...) { return false; }

    template <class X> static constexpr bool isA() {
      return substProbe<X>(static_cast<X *>(nullptr));
    }
  };

  static_assert(Traits::template isA<StreamProbe<C>>());
};

template <class P, typename C>
static constexpr bool
    hasStreamProbeSignature = StreamProbe<C>::Traits::template isA<P>();

template <class P, typename C, class S = InputStream<C>>
class ProbedInputStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  static_assert(hasStreamProbeSignature<P, C>);

  S &s;
  P &p;

public:
  ProbedInputStream(S &stream, P &probe) : s(stream), p(probe) {}

  bool get(C &result) final {
    if (s.get(result)) {
      p.probe(result);
      return true;
    }
    return false;
  }

  const P &getProbe() const { return p; }
  const S &getStream() const { return s; }
};

template <class P, typename C, class S = InputStream<C>>
class ProbeInputStream : public P, virtual public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  static_assert(hasStreamProbeSignature<P, C>);

  S &s;

public:
  ProbeInputStream(S &stream) : s(stream) {}

  template <typename... A>
  ProbeInputStream(S &stream, A... args) : P(args...), s(stream) {};

  bool get(C &result) final {
    if (s.get(result)) {
      P::probe(result);
      return true;
    }
    return false;
  }

  const S &getStream() const { return s; }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_STREAM_PROBE_H
