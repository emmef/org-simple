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

#include <org-simple/util/Predicate.h>
#include <org-simple/util/text/InputStream.h>
#include <org-simple/util/text/StreamFilter.h>

namespace org::simple::util::text {

template <typename C, bool resetIfExhausted, class S = InputStream<C>>
class PredicateFunctionStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  VariableEchoRepeatOneStream<C, resetIfExhausted, S> stream;
  PredicateFunction<C> predicate;

public:
  PredicateFunctionStream(S *inputStream, PredicateFunction<C> function)
      : stream(inputStream),
        predicate(function ? function : falsePredicateFunction<C>) {}
  PredicateFunctionStream(PredicateFunction<C> function)
      : PredicateFunctionStream(nullptr, function) {}

  bool get(C &result) final { return stream.get(result) && function(result); }

  void assignStream(S *newStream) { stream.assignStream(newStream); }
  C getMostRecent() const { return stream.getMostRecent(); }
};

template <typename C, class P, bool resetIfExhausted, class S = InputStream<C>>
class PredicateStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  static_assert(hasPredicateSignature<P, C>);

  VariableEchoRepeatOneStream<C, resetIfExhausted, S> stream;
  const P &predicate;

public:
  PredicateStream(S *inputStream, const P &predicate_)
      : stream(inputStream), predicate(predicate_) {}

  bool get(C &result) final {
    return stream.get(result) && predicate.test(result);
  }

  void assignStream(S *newStream) { stream.assignStream(newStream); }
  C getMostRecent() const { return stream.getMostRecent(); }
};

template <typename C, bool resetInputOnStop, class S = InputStream<C>>
class PredicateFunctionVariableInputStream
    : public VariableEchoStream<C, resetInputOnStop, S> {
  static_assert(hasInputStreamSignature<S, C>);

  PredicateFunction<C> predicate;

public:
  PredicateFunctionVariableInputStream(S *stream,
                                       PredicateFunction<C> predicateFunction)
      : VariableEchoStream<C, resetInputOnStop, S>(stream),
        predicate(predicateFunction ? predicateFunction
                                    : falsePredicateFunction<C>) {}

  PredicateFunctionVariableInputStream(PredicateFunction<C> predicateFunction)
      : VariableEchoStream<C, resetInputOnStop, S>(),
        predicate(predicateFunction ? predicateFunction
                                    : falsePredicateFunction<C>) {}

  bool get(C &result) final {
    return VariableEchoStream<C, resetInputOnStop, S>::get(result) &&
           predicate(result);
  }

  void setPredicateFunction(PredicateFunction<C> predicateFunction) {
    predicate =
        predicateFunction ? predicateFunction : falsePredicateFunction<C>;
  }
};

template <typename C, class P, bool resetInputOnStop, class S = InputStream<C>>
class PredicateVariableInputStream
    : public VariableEchoStream<C, resetInputOnStop, S> {
  static_assert(hasInputStreamSignature<S, C>);

  const Predicate<C> &p;

public:
  PredicateVariableInputStream(S *stream, const Predicate<C> &predicate)
      : VariableEchoStream<C, resetInputOnStop, S>(stream), p(predicate) {}

  PredicateVariableInputStream(const Predicate<C> &predicate)
      : VariableEchoStream<C, resetInputOnStop, S>(), p(predicate) {}

  bool get(C &result) final {
    return VariableEchoStream<C, resetInputOnStop, S>::get(result) && p.test(result);
  }

  void setPredicateFunction(PredicateFunction<C> predicateFunction) {
    p = predicateFunction ? predicateFunction : falsePredicateFunction<C>;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_STREAM_PREDICATE_H
