#ifndef ORG_SIMPLE_INPUTFILTER_H
#define ORG_SIMPLE_INPUTFILTER_H
/*
 * org-simple/util/text/TextFilters.h
 *
 * Added by michel on 2021-12-21
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

#include <cstdint>
#include <org-simple/util/InputStream.h>
#include <org-simple/util/text/Characters.h>

namespace org::simple::util::text {

/**
 * Describes state of the filtering, that tells the caller of a filter what to
 * do.
 */
enum class InputFilterResult {
  /**
   * The filtered result can be treated as valid and, say, returned.
   */
  Ok,
  /**
   * There is no output, so another input must be fetched and filtering must be
   * applied again.
   */
  GetNext,
  /**
   * Stop reading from the input stream. This can be used to create input
   * streams that only read another stream partially.
   */
  Stop
};

template <typename C> class InputFilter {
public:
  /**
   * Apply the filter, where the input and the output reside in {@code result}.
   * Additional input can be read from the stream provided in {@code input}.
   *
   * @param result The result that can be written, which should only happen if
   * {@code TextFilterResult::True} is returned.
   * @param input The input stream that can be used to read ahead.
   * @return What the caller of the filter should do next.
   */
  virtual InputFilterResult filter(C &result) = 0;

  virtual ~InputFilter() = default;

  struct Traits {
    template <class X>
    requires(
        std::is_same_v<
            InputFilterResult,
            decltype(std::declval<X>().filter(
                std::declval<C &>()))>) static constexpr bool substFilter(X *) {
      return true;
    }
    template <class X> static constexpr bool substFilter(...) { return false; }

    template <class X> static constexpr bool isA() {
      return substFilter<X>(static_cast<X *>(nullptr));
    }
  };

  template <class D> class Wrapped : public InputFilter<C> {
    static_assert(Traits::template isA<D>());
    D *wrapped;

  public:
    Wrapped(D &filter) : wrapped(&filter) {}
    Wrapped(const Wrapped &) = default;
    Wrapped(Wrapped &&) = default;

    bool get(C &c) final { return wrapped->get(c); }
  };

  template <class D> class Interfaced : public virtual InputFilter, public D {
    static_assert(Traits::template isA<D>() &&
                  !std::is_base_of_v<InputFilter, D>);

  public:
    bool get(C &c) final { return D::filter(c); }
  };
};

template <class F, typename C>
static constexpr bool
    hasInputFilterSignature = InputFilter<C>::Traits::template isA<F>();

template <class F, typename C>
requires(!std::is_base_of_v<InputFilter<C>, F> && hasInputFilterSignature<F, C>)
    typename InputFilter<C>::template Wrapped<F> wrapAsInputFilter(F &wrapped) {
  return typename InputFilter<C>::template Wrapped<F>(wrapped);
}

template <class F, class S, typename C>
static constexpr bool canApplyInputFilterOnStream =
    hasInputFilterSignature<F, C> &&hasInputStreamSignature<S, C>;

template <class F, class S, typename C>
requires(canApplyInputFilterOnStream<F, S, C>) static bool applyInputFilter(
    F &filter, S &input, C &result) {
  do {
    /*
     * Some filters can also serve as an input stream, for example when
     * detecting a match takes several inputs and when the match does not occur,
     * the unmatched parts can be "replayed".
     */
    if constexpr (hasInputStreamSignature<F, C>) {
      if (!filter.get(result) && !input.get(result)) {
        return false;
      }
    } else {
      if (!input.get(result)) {
        return false;
      }
    }
    switch (filter.filter(result)) {
    case InputFilterResult::Ok:
      return true;
    case InputFilterResult::GetNext:
      break;
    default:
      return false;
    }
  } while (true);
}

template<class F, class S, typename C> class FilteredInputStream : public InputStream<C> {
  static_assert(canApplyInputFilterOnStream<F, S, C>);

  F &f;
  S &s;
public:
  FilteredInputStream(F &filter, S &stream) : f(filter), s(stream) {}

  bool get(C &c) final {
    return applyInputFilter(f, s, c);
  }
};

template <class F, class S, typename C, bool resetInputOnStop>
class FilteredVariableInputStream : public InputStream<C> {
  static_assert(canApplyInputFilterOnStream<F, S, C>);

  F &filter;
  InputStream<C> *input = nullptr;

public:
  FilteredVariableInputStream(F &inputFilter, InputStream<C> *stream)
      : filter(inputFilter), input(stream) {}

  bool get(C &result) final {
    if (input) {
      if (applyInputFilter(filter, *input, result)) {
        return true;
      }
      if constexpr (resetInputOnStop) {
        input = nullptr;
      }
    }
    return false;
  }

  void setStream(InputStream<C> *stream) {
    input = stream;
  }
};


template <typename C> class InputProbe {
public:
  virtual void probe(const C &) = 0;
  virtual ~InputProbe() = default;
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_INPUTFILTER_H
