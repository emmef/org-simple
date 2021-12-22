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
 * Describes what action the caller of a filter step should take after the
 * filtering.
 */
enum class TextFilterResult {
  /**
   * The receiver of the result should return {@code true}.
   */
  True,
  /**
   * The receiver of the result should return {@code false}.
   */
  False,
  /**
   * The receiver of the result should proceed processing.
   */
  Continue,
  /**
   * The receiver of the result should swallow and seek more input
   */
  Swallow
};

template <typename C, class S = InputStream<C>> class AbstractTextFilter {
  static_assert(std::is_base_of_v<InputStream<C>, S>);

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
  virtual TextFilterResult filter(C &result, S &input) = 0;
  /**
   * Returns whether there are characters available without further input from
   * the input stream.
   */
  virtual bool available() { return false; }

  /**
   * Implements a filtered form of {@code input.get(result)}, that can both add
   * end omit characters. This function can be used to translate this filter
   * right-away into a filtered stream with the same characteristics.
   * @param result The result character.
   * @param input The input stream.
   * @return Whether result contains the valid next character.
   */
  bool get(C &result, S &input) {
    while (true) {
      if (!available()) {
        if (!input.get(result)) {
          return false;
        }
      }
      switch (filter(result, input)) {
      case TextFilterResult::True:
        return true;
      case TextFilterResult::Swallow:
        break;
      default:
        return false;
      }
    }
  }
  virtual ~AbstractTextFilter() = default;
};



} // namespace org::simple::util

#endif // ORG_SIMPLE_TEXTFILTERS_H
