#ifndef ORG_SIMPLE_UTIL_TEXT_M_STREAM_PREDICATE_H
#define ORG_SIMPLE_UTIL_TEXT_M_STREAM_PREDICATE_H
/*
 * org-simple/util/text/StreamPredicate.h
 *
 * Added by michel on 2021-12-26
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

#include <org-simple/util/text/InputStream.h>
#include <org-simple/util/text/StreamFilter.h>

namespace org::simple::util::text {

template <class P> using StreamPredicateFunction = bool (*)(const P &);

template <class P> static bool trueStreamPredicateFunction(const P &) {
  return true;
}
template <class P> static bool falseStreamPredicateFunction(const P &) {
  return false;
}

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

template <class P, class S, typename C>
static constexpr bool canApplyPredicateAfterProbe =
    hasInputStreamSignature<S, C> &&hasStreamProbeSignature<P, C>;

template <class P, class S, typename C>
requires(canApplyPredicateAfterProbe<P, S, C>)
    // Prevent unreadable requires formatting
    static bool applyPredicateAfterProbe(
        P &predicateSubject, S &input, C &result,
        StreamPredicateFunction<P> predicateFunction) {
  if (!input.get(result)) {
    return false;
  }
  predicateSubject.probe(result);
  if (!predicateFunction(predicateSubject)) {
    return false;
  }
  return true;
}

template <class P, class S, typename C>
requires(hasInputStreamSignature<S, C>)
    // Prevent unreadable requires formatting
    static bool applyPredicate(const P &predicateSubject, S &input, C &result,
                               StreamPredicateFunction<P> predicateFunction) {
  if (!input.get(result)) {
    return false;
  }
  if (!predicateFunction(predicateSubject)) {
    return false;
  }
  return true;
}

template <class P, class S, bool resetInputOnStop, typename C>
class PredicateVariableInputStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);

  const P &subject;
  S *input = nullptr;
  StreamPredicateFunction<P> predicate;

public:
  PredicateVariableInputStream(const P &predicateSubject, S *stream,
                               StreamPredicateFunction<P> predicateFunction)
      : subject(predicateSubject), input(stream),
        predicate(predicateFunction ? predicateFunction
                                    : falseStreamPredicateFunction<P>) {}

  bool get(C &result) final {
    if (input) {
      if (applyPredicate(subject, *input, result, predicate)) {
        return true;
      }
      if constexpr (resetInputOnStop) {
        input = nullptr;
      }
    }
    return false;
  }

  void setStream(InputStream<C> *stream) { input = stream; }

  void setPredicateFunction(StreamPredicateFunction<P> predicateFunction) {
    predicate =
        predicateFunction ? predicateFunction : falseStreamPredicateFunction<P>;
  }
};

template <class P, class S, bool resetInputOnStop, typename C>
class ProbeVariableInputStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  static_assert(canApplyPredicateAfterProbe<P, S, C>);

  P &subject;
  S *input = nullptr;
  StreamPredicateFunction<P> predicate;

public:
  ProbeVariableInputStream(P &predicateSubject, S *stream,
                           StreamPredicateFunction<P> predicateFunction)
      : subject(predicateSubject), input(stream),
        predicate(predicateFunction ? predicateFunction
                                    : falseStreamPredicateFunction<P>) {}

  bool get(C &result) final {
    if (input) {
      if (applyPredicateAfterProbe(subject, *input, result, predicate)) {
        return true;
      }
      if constexpr (resetInputOnStop) {
        input = nullptr;
      }
    }
    return false;
  }

  void setStream(InputStream<C> *stream) { input = stream; }

  void setPredicateFunction(StreamPredicateFunction<P> predicateFunction) {
    predicate =
        predicateFunction ? predicateFunction : falseStreamPredicateFunction<P>;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_STREAM_PREDICATE_H
