#ifndef ORG_SIMPLE_TEXTFILTERS_H
#define ORG_SIMPLE_TEXTFILTERS_H
/*
 * org-simple/TextFilters.h
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
#include <org-simple/util/Characters.h>
#include <org-simple/util/InputStream.h>

namespace org::simple::util {

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

template <typename C, class F>
static inline bool getFiltered(C &result, F &filter, InputStream<C> &input) requires(
    std::is_base_of_v<InputFilter<C>, F>) {
  do {
    if constexpr (std::is_base_of_v<InputFilterWithBuffer<C>, F>) {
      if (!filter.available()) {
        if (!input.get(result)) {
          return false;
        }
      }
    }
    else {
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
  } while(true);
}

template <typename C>
bool InputFilter<C>::get(C &result, InputStream<C> &input) {
  return getFiltered(result, *this, input);
}

template <typename C>
bool InputFilterWithBuffer<C>::get(C &result, InputStream<C> &input) {
  return getFiltered(result, *this, input);
}

} // namespace org::simple::util

#endif // ORG_SIMPLE_TEXTFILTERS_H
