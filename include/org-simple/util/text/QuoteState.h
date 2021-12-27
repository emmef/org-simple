#ifndef ORG_SIMPLE_UTIL_TEXT_M_QUOTE_STATE_H
#define ORG_SIMPLE_UTIL_TEXT_M_QUOTE_STATE_H
/*
 * org-simple/util/text/QuoteState.h
 *
 * Added by michel on 2021-12-22
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

#include <org-simple/util/text/StreamFilter.h>
#include <org-simple/util/text/StreamPredicate.h>
#include <org-simple/util/text/StreamProbe.h>

namespace org::simple::util::text {

template <typename C> class QuoteState : public StreamProbe<C> {
  bool escaped = false;
  C openQuote = 0;
  C closeQuote = 0;

public:
  typename QuoteMatcher<C>::function matcher;

  QuoteState(typename QuoteMatcher<C>::function function)
      : matcher(function ? function : QuoteMatcher<C>::none) {}

  QuoteState(const char *symmetricQuoteChars)
      : matcher(QuoteMatchers::getDefaultMatcherFor<C>(symmetricQuoteChars,
                                                       QuoteMatcher<C>::none)) {
  }

  void reset() { *this = {matcher}; }

  C getOpenQuote() const { return openQuote; }
  C getCloseQuote() const { return closeQuote; }
  bool isEscaped() const { return escaped; }
  bool inQuote() const { return openQuote != 0; }

  void probe(const C &c) final {
    if (openQuote != 0) {
      if (escaped) {
        escaped = false;
      } else if (c == '\\') {
        escaped = true;
      } else if (c == closeQuote) {
        openQuote = 0;
        closeQuote = 0;
      }
    } else if (escaped) {
      escaped = false;
    } else if (matcher && matcher(c, closeQuote)) {
      openQuote = c;
    } else if (c == '\\') {
      escaped = true;
    }
  }
};

template <typename C, class S = InputStream<C>>
class QuotedStateStream : public ProbeInputStream<QuoteState<C>, C, S> {
public:

  QuotedStateStream(S &input, typename QuoteMatcher<C>::function function) : ProbeInputStream<QuoteState<C>, C, S>(input, function) {}
  QuotedStateStream(S &input, const char *symmetricQuoteChars) : ProbeInputStream<QuoteState<C>, C, S>(input, symmetricQuoteChars) {}

  using ProbeInputStream<QuoteState<C>, C, S>::isEscaped;
};

template <typename C, bool inSide> class QuoteStatePredicate : public Predicate<C> {
  const QuoteState<C> &state;
public:
  bool test(const C&) const final {
    if constexpr (inSide) {
      return state.inQuote();
    }
    else {
      return !state.inQuote();
    }
  }

  QuoteStatePredicate(const QuoteState<C> & quoteState) : state(quoteState) {}
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_QUOTE_STATE_H
