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

template <typename C, class D, bool negate = false>
class GraphPredicate : public Predicate<C> {
  static_assert(std::is_same_v<Ascii, D> || std::is_same_v<Unicode, D>);
  const D &classifier = Classifiers::instance<D>();

public:
  bool test(const C &c) const final { return negate ^ classifier.isGraph(c); }

  static const GraphPredicate &instance() {
    static GraphPredicate pred;
    return pred;
  }

  static bool function(const C &c) { return instance().test(c); }
};


template <typename C, bool negate = false>
class NewLinePredicate : public Predicate<C> {

public:
  bool test(const C &c) const final { return function(c); }

  static constexpr bool function(const C &c) {
    return negate ^ (c == '\n');
  }

  static const NewLinePredicate &instance() {
    static NewLinePredicate pred;
    return pred;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAMS_H
