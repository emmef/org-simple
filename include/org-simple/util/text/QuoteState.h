#ifndef ORG_SIMPLE_QUOTESTATE_H
#define ORG_SIMPLE_QUOTESTATE_H
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
#include <org-simple/util/text/TextFilters.h>

namespace org::simple::util::text {

template <typename C> class QuoteState {
  bool escaped = false;
  C openQuote = 0;
  C closeQuote = 0;

public:
  typename QuoteMatcher<C>::function matcher;

  QuoteState(typename QuoteMatcher<C>::function function)
      : matcher(function ? function : QuoteMatcher<C>::none) {}

  QuoteState(const char *symmetricQuoteChars)
      : matcher(QuoteMatchers::getDefaultMatcherFor<C>(
            symmetricQuoteChars, QuoteMatcher<C>::none)) {}

  void reset() { *this = {matcher}; }

  C getOpenQuote() const { return openQuote; }
  C getCloseQuote() const { return closeQuote; }
  bool isEscaped() const { return escaped; }
  bool inQuote() const { return openQuote != 0; }

  void probe(C c) {
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

template <typename T>
class QuotedStateStream : public QuoteState<T>, public util::InputStream<T> {

public:
  using matcherFunction = typename QuoteMatcher<T>::function;

  template <typename Q>
  QuotedStateStream(util::InputStream<T> &stream, Q functionOrQuotes)
      : QuoteState<T>(functionOrQuotes), input(stream) {}

  bool get(T &result) final {
    if (!input.get(result)) {
      return false;
    }
    QuoteState<T>::probe(result);
    return true;
  }

private:
  util::InputStream<T> &input;
};

template <typename T> class InQuoteStream : public util::InputStream<T> {
  const QuoteState<T> &state;
  util::InputStream<T> *input;

public:
  InQuoteStream(const QuoteState<T> &quoteState, util::InputStream<T> &source)
      : state(quoteState) {}

  bool get(T &result) final {
    if (input != nullptr && input->get(result) && state.inQuote()) {
      return true;
    }
    input = nullptr;
  }

  void set(util::InputStream<T> *source) {
    input = source;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_QUOTESTATE_H
