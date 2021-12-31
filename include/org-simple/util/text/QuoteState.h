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

template <typename C>
class QuoteState : public StreamProbe<C>, public Predicate<C> {
  bool escaped = false;
  C openQuote = 0;
  C closeQuote = 0;

public:
  static constexpr char defaultEscapeCharacter = '\\';

  struct Config {
    typedef typename QuoteMatcher<C>::function matcher;
    C escapeChr;
    matcher quoteMatcher;

    Config(matcher function, C escapeCharacter)
        : escapeChr(escapeCharacter),
          quoteMatcher(function ? function : QuoteMatcher<C>::none) {}

    Config(matcher function) : Config(function, defaultEscapeCharacter) {}

    Config(const char *symmetricQuoteChars, C escapeCharacter)
        : Config(QuoteMatchers::getDefaultMatcherFor<C>(symmetricQuoteChars,
                                                        QuoteMatcher<C>::none),
                 escapeCharacter) {}

    Config(const char *symmetricQuoteChars)
        : Config(symmetricQuoteChars, defaultEscapeCharacter) {}
  } config;

  template <typename... A> QuoteState(A... args) : config(args...) {}

  explicit QuoteState(const Config &c) : config(c){};

  void reset() { *this = QuoteState(config); }

  C getOpenQuote() const { return openQuote; }
  C getCloseQuote() const { return closeQuote; }
  bool isEscaped() const { return escaped; }
  bool inQuote() const { return openQuote != 0; }
  C getEscapeCharacter() const { return config.escapeChr; }
  const Config getConfig() const { return config; }

  void probe(const C &c) final {
    if (openQuote != 0) {
      if (escaped) {
        escaped = false;
      } else if (c == config.escapeChr) {
        escaped = true;
      } else if (c == closeQuote) {
        openQuote = 0;
        closeQuote = 0;
      }
    } else if (escaped) {
      escaped = false;
    } else if (config.quoteMatcher(c, closeQuote)) {
      openQuote = c;
    } else if (c == config.escapeChr) {
      escaped = true;
    }
  }
  /**
   * Tests whether a character indicates a valid start of a quoted section, not
   * whether this \c QuoteState is inside or outside a quoted section.
   * @param c The character to test.
   * @return
   */
  bool test(const C &c) const final {
    C endQuote;
    return (closeQuote != 0 && c == closeQuote) ||
           config.quoteMatcher(c, endQuote);
  }
};

template <typename C, class S = InputStream<C>>
class QuotedStateStream : public ProbeInputStream<QuoteState<C>, C, S> {
public:
  QuotedStateStream(S &input, typename QuoteMatcher<C>::function function)
      : ProbeInputStream<QuoteState<C>, C, S>(input, function) {}
  QuotedStateStream(S &input, const char *symmetricQuoteChars)
      : ProbeInputStream<QuoteState<C>, C, S>(input, symmetricQuoteChars) {}

  using ProbeInputStream<QuoteState<C>, C, S>::isEscaped;
};

template <typename C, bool inSide>
class QuoteStatePredicate : public Predicate<C> {
  const QuoteState<C> &state;

public:
  bool test(const C &) const final {
    if constexpr (inSide) {
      return state.inQuote();
    } else {
      return !state.inQuote();
    }
  }

  QuoteStatePredicate(const QuoteState<C> &quoteState) : state(quoteState) {}
};

template <typename C>
class QuoteStateFilter : private QuoteState<C>,
                         public StreamFilter<C>,
                         public InputStream<C> {
  ReplayStream<C, 1> replay;
  bool inEscape = false;

public:
  template <typename... Args>
  QuoteStateFilter(Args... args) : QuoteState<C>(args...) {}

  const QuoteState<C> *operator->() const { return this; }

  InputFilterResult filter(C &result) override {
    bool inQuote = this->inQuote();
    this->probe(result);
    if (inEscape) {
      inEscape = false;
      if (this->test(result)) {
        return InputFilterResult::Ok;
      } else {
        replay << result;
        result = this->getEscapeCharacter();
        return InputFilterResult::Ok;
      }
    }
    if (inQuote != this->inQuote()) {
      return InputFilterResult::GetNext;
    } else if (this->isEscaped()) {
      inEscape = true;
      return InputFilterResult::GetNext;
    } else {
      return InputFilterResult::Ok;
    }
  }

  bool get(C &c) override { return replay.get(c); }
};

template <typename C, class S = InputStream<C>>
using QuoteFilteredStream = FilteredInputStream<C, QuoteStateFilter<C>, S>;

template <typename C, class S = InputStream<C>>
class QuoteStateTokenizedStream : public TokenizedInputStream<C> {
  ReplayStream<C, 1> replay;
  enum class State { SkipWhilePredicate, OutsideQuotes, InsideQuotes, Exhausted };
  State state = State::SkipWhilePredicate;

public:
  QuoteStateTokenizedStream(
      QuoteFilteredStream<C, S> &stream,
      const Predicate<C> *terminationPredicateOutsideQuote)
      : input(stream),
        outsideQuoteTerminationPredicate(terminationPredicateOutsideQuote) {}

  QuoteStateTokenizedStream(QuoteFilteredStream<C, S> &stream)
      : QuoteStateTokenizedStream(stream, nullptr) {}

  virtual bool isExhausted() const { return state == State::Exhausted; }

  bool get(C &result) {
    C c;
    if (replay.get(c)) {
      result = c;
      return true;
    }
    while (getWithReplay(c)) {
      if (state == State::SkipWhilePredicate) {
        if (input.getFilter()->inQuote()) {
          state = State::InsideQuotes;
        } else if (!meetsPredicate(c)) {
          state = State::OutsideQuotes;
        }
        else {
          continue;
        }
        replay << c;
        continue;
      }
      bool inQuote = input.getFilter()->inQuote();
      if (state == State::InsideQuotes) {
        if (inQuote) {
          result = c;
          return true;
        } else if (meetsPredicate(c)) {
          state = State::SkipWhilePredicate;
        } else {
          state = State::OutsideQuotes;
          replay << c;
        }
        return false;
      }
      if (state == State::OutsideQuotes) {
        if (inQuote) {
          state = State::InsideQuotes;
          replay << c;
          return false;
        }
        else if (meetsPredicate(c)) {
          state = State::SkipWhilePredicate;
          return false;
        } else {
          result = c;
          return true;
        }
      }
    }
    state = State::Exhausted;
    return false;
  }

  void resetExhausted() override {
    state = State::SkipWhilePredicate;
  }

private:
  QuoteFilteredStream<C, S> input;
  const Predicate<C> *outsideQuoteTerminationPredicate;

  bool meetsPredicate(const C &c) const {
    return outsideQuoteTerminationPredicate &&
           outsideQuoteTerminationPredicate->test(c);
  }

  bool getWithReplay(C &result) {
    C c;
    if (replay.get(c) || input.get(c)) {
      result = c;
      return true;
    }
    return false;
  }
};
} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_QUOTE_STATE_H
