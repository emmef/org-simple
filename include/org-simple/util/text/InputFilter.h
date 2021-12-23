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

  /**
   * Implements a filtered form of {@code input.get(result)}, that can both add
   * end omit characters. This function can be used to translate this filter
   * right-away into a filtered stream with the same characteristics.
   * @param result The result character.
   * @param input The input stream.
   * @return Whether result contains the valid next character.
   */
  virtual bool get(C &result, InputStream<C> &input);
  virtual ~InputFilter() = default;
};

template <typename C> class InputFilterWithBuffer : public InputFilter<C> {

public:
  virtual bool available() = 0;

  /**
   * Implements a filtered form of {@code input.get(result)}, that can both add
   * end omit characters. This function can be used to translate this filter
   * right-away into a filtered stream with the same characteristics.
   * @param result The result character.
   * @param input The input stream.
   * @return Whether result contains the valid next character.
   */
  bool get(C &result, InputStream<C> &input) override;
  virtual ~InputFilterWithBuffer() = default;
};

template <class X, typename C>
concept ConceptIsInputFilter = std::is_base_of_v<InputFilter<C>, X> ||
    std::is_same_v<TextFilterResult, decltype(X().filter(std::declval<C &>()))>;

template <class X, typename C>
concept ConceptIsInputFilterWithBuffer = ConceptIsInputFilter<X, C> &&
    (std::is_base_of_v<InputFilterWithBuffer<C>, X> ||
     std::is_same_v<bool, decltype(std::add_const_t<X>().available())>);

template <typename F, class C>
requires(ConceptIsInputFilter<F, C>) static inline bool getFiltered(
    C &result, F &filter,
    InputStream<C> &input) requires(ConceptIsInputFilter<F, C>) {
  do {
    if constexpr (ConceptIsInputFilterWithBuffer<F, C>) {
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

template <typename C>
bool InputFilter<C>::get(C &result, InputStream<C> &input) {
  return getFiltered<InputFilter<C>, C>(result, *this, input);
}

template <typename C>
bool InputFilterWithBuffer<C>::get(C &result, InputStream<C> &input) {
  return getFiltered<InputFilterWithBuffer<C>, C>(result, *this, input);
}

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_INPUTFILTER_H
