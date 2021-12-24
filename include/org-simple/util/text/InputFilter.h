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
enum class TextFilterResult {
  /**
   * The filtered result can be treated as valid and, say, returned.
   */
  Ok,
  /**
   * There is no output, so another input must be fetched and filtering must be
   * applied again.
   */
  GetNext
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
  virtual TextFilterResult filter(C &result) = 0;

  virtual ~InputFilter() = default;
};

template <typename C> class InputFilterWithBuffer : public InputFilter<C> {

public:
  virtual bool available() const = 0;

  /**
   * Provides an implementation Implements a filtered form of {@code
   * input.get(result)}, that can omit and add characters. This function can be
   * used to translate the filter right-away into a filtered stream with the
   * same characteristics.
   * @param result The result character.
   * @param input The input stream.
   * @return Whether result contains the valid next character.
   */
  //  bool apply(C &result, InputStream<C> &input) override;
};

template <typename C> class InputProbe {
public:
  virtual void probe(const C &) = 0;
  virtual ~InputProbe() = default;
};

template <class F, typename C> struct FilterConcept {

  class TestHasFilterFunction {
    template <class X>
    requires(std::is_same_v<
             TextFilterResult,
             decltype(X().filter(
                 std::declval<C &>()))>) static constexpr bool subst(X *) {
      return true;
    }
    template <class X> static constexpr bool subst(...) { return false; }

    static constexpr bool test() {
      if constexpr (std::is_base_of_v<InputFilter<C>, F>) {
        return true;
      } else {
        return subst<F>(static_cast<F *>(nullptr));
      }
    }

  public:
    static constexpr bool has = test();
  };

  class TestHasAvailableFunction {
    template <class X>
    requires(std::is_same_v<
             bool,
             decltype(std::add_const_t<X>()
                          .available())>) static constexpr bool subst(X *) {
      return true;
    }
    template <class X> static constexpr bool subst(...) { return false; }

    static constexpr bool test() {
      if constexpr (std::is_base_of_v<InputFilterWithBuffer<C>, F>) {
        return true;
      } else {
        return subst<F>(static_cast<F *>(nullptr));
      }
    }

  public:
    static constexpr bool has = test();
  };

  static constexpr bool validAsInputFilter = TestHasFilterFunction::has;

  static constexpr bool validAsInputFilterWithBuffer =
      TestHasFilterFunction::has && TestHasAvailableFunction::has;

  static constexpr bool validAsInputFilterOnly =
      TestHasFilterFunction::has && !TestHasAvailableFunction::has;

  static constexpr bool implementsInputFilterOnly() {
    return std::is_base_of_v<InputFilter<C>, F> &&
           !std::is_base_of_v<InputFilterWithBuffer<C>, F>;
  }

  class TestHasDirectFilterFunction {
    template <class X>
    requires(std::is_same_v<
             TextFilterResult,
             decltype(X().directFilter(
                 std::declval<C &>()))>) static constexpr bool subst(X *) {
      return true;
    }
    template <class X> static constexpr bool subst(...) { return false; }

  public:
    static constexpr bool has = subst<F>(static_cast<F *>(nullptr));
  };

  class TestHasDirectAvailableFunction {
    template <class X>
    requires(
        std::is_same_v<
            bool,
            decltype(std::add_const_t<X>()
                         .directAvailable())>) static constexpr bool subst(const X
                                                                               *) {
      return true;
    }
    template <class X> static constexpr bool subst(...) { return false; }

  public:
    static constexpr bool has = subst<F>(static_cast<F *>(nullptr));
  };

  static constexpr bool validAsDirectInputFilter =
      TestHasDirectFilterFunction::has;

  static constexpr bool validAsDirectInputFilterWithBuffer =
      TestHasDirectFilterFunction::has && TestHasDirectAvailableFunction::has;

  static constexpr bool validAsDirectInputFilterOnly =
      TestHasDirectFilterFunction::has && !TestHasDirectAvailableFunction::has;

  static constexpr bool validToApplyAsFilter =
      validAsInputFilter || validAsInputFilterWithBuffer ||
      validAsDirectInputFilter || validAsDirectInputFilterWithBuffer;
};

template <class F, typename C>
requires(FilterConcept<F, C>::
             validToApplyAsFilter) static inline bool applyFilter(C &result,
                                                                  F &filter,
                                                                  InputStream<C>
                                                                      &input) {
  if constexpr (FilterConcept<F, C>::validAsDirectInputFilter) {
    do {
      if constexpr (FilterConcept<F, C>::validAsDirectInputFilterWithBuffer) {
        if (!filter.directAvailable()) {
          if (!input.get(result)) {
            return false;
          }
        }
      } else {
        if (!input.get(result)) {
          return false;
        }
      }
      switch (filter.directFilter(result)) {
      case TextFilterResult::Ok:
        return true;
      case TextFilterResult::GetNext:
        break;
      default:
        return false;
      }
    } while (true);
  }
  else if constexpr (FilterConcept<F, C>::validAsInputFilter) {
    do {
      if constexpr (FilterConcept<F, C>::validAsInputFilterWithBuffer) {
        if (!filter.available()) {
          if (!input.get(result)) {
            return false;
          }
        }
      } else {
        if (!input.get(result)) {
          return false;
        }
      }
      switch (filter.filter(result)) {
      case TextFilterResult::Ok:
        return true;
      case TextFilterResult::GetNext:
        break;
      default:
        return false;
      }
    } while (true);
  }
}

template <class D, typename C>
class AbstractInputFilter : public InputFilter<C>, public D {
  static_assert(!std::is_base_of_v<InputFilter<C>, D> &&
                FilterConcept<D, C>::TestHasFilterFunction::has);

public:
  virtual TextFilterResult filter(C &result) final {
    return static_cast<D *>(this)->directFilter(result);
  }
};

template <class D, typename C>
class AbstractInputFilterWithBuffer : public virtual InputFilterWithBuffer<C>,
                                      public D {
  static_assert(!std::is_base_of_v<InputFilter<C>, D> &&
                !std::is_base_of_v<InputFilterWithBuffer<C>, D> &&
                FilterConcept<D, C>::TestHasDirectFilterFunction::has &&
                FilterConcept<D, C>::TestHasDirectAvailableFunction::has);

public:
  virtual bool available() const final {
    return static_cast<const D *>(this)->directAvailable();
  }
  virtual TextFilterResult filter(C &result) final {
    return static_cast<D *>(this)->directFilter(result);
  }
};

template <typename C, class D> class AbstractInputProbe : public InputProbe<C> {
  static_assert(
      std::is_same_v<void, decltype(D().probe(std::declval<const C &>()))>);

  D data;

public:
  void doProbe(const C &c) { data.probe(c); }

  void probe(const C &c) final { return doProbe(c); }

  const D &getData() const { return data; }
  const D &operator->() const { return getData(); }

  void reset() { data = D{}; }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_INPUTFILTER_H
