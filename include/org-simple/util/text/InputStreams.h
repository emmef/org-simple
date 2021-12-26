#ifndef ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAMS_H
#define ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAMS_H
/*
 * org-simple/util/text/InputStreams.h
 *
 * Added by michel on 2021-12-24
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

#include <org-simple/util/text/Characters.h>
#include <org-simple/util/text/StreamFilter.h>

namespace org::simple::util::text {

template <typename C, class D>
class GraphBasedStreamProbe : public StreamProbe<C> {
  static_assert(std::is_same_v<Ascii, D> || std::is_same_v<Unicode, D>);
  const D &classifier = Classifiers::instance<D>();
  bool graph;

public:
  void probe(const C &c) final { graph = classifier.isGraph(c); }

  bool isGraph() const { return graph; }

  static bool isGraphPredicateFunction(const GraphBasedStreamProbe &subject) {
    return subject.isGraph();
  }
  static bool noGraphPredicateFunction(const GraphBasedStreamProbe &subject) {
    return !subject.isGraph();
  }
};

template <class S, class D, bool resetStreamOnStop, typename C>
using GraphBasedStreamWithPredicate =
    PredicateVariableInputStream<GraphBasedStreamProbe<C, D>, S,
                                 resetStreamOnStop, C>;

template <class S, class D, bool resetStreamOnStop, typename C>
using GraphBasedStreamWithProbedPredicate =
    ProbeVariableInputStream<GraphBasedStreamProbe<C, D>, S, resetStreamOnStop,
                             C>;

template <typename C> class NewLineBasedStreamProbe : public StreamProbe<C> {
  bool nl;

public:
  void probe(const C &c) final { nl = c == '\n'; }

  bool isNewLine() const { return nl; }

  static bool
  isNewLinePredicateFunction(const NewLineBasedStreamProbe &subject) {
    return subject.isNewLine();
  }
  static bool
  noNewLinePredicateFunction(const NewLineBasedStreamProbe &subject) {
    return !subject.isNewLine();
  }
};

template <class S, bool resetStreamOnStop, typename C>
using NewLineBasedStreamWithPredicate =
    PredicateVariableInputStream<NewLineBasedStreamProbe<C>, S,
                                 resetStreamOnStop, C>;

template <class S, bool resetStreamOnStop, typename C>
using NewLineBasedStreamWithProbedPredicate =
    ProbeVariableInputStream<NewLineBasedStreamProbe<C>, S, resetStreamOnStop,
                             C>;

template <typename C>
class EchoRememberLastInputStream : public InputStream<C> {
  InputStream<C> &input;
  C v = 0;

public:
  EchoRememberLastInputStream(InputStream<C> &stream) : input(stream) {}

  bool get(C &result) final {
    if (input.get(result)) {
      v = result;
      return true;
    }
    return false;
  }

  C &lastValue() { return v; }
  const C &lastValue() const { return v; }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAMS_H
