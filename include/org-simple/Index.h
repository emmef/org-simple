#ifndef ORG_SIMPLE_CORE_M_INDEX_H
#define ORG_SIMPLE_CORE_M_INDEX_H
/*
 * org-simple/core/Index.h
 *
 * Added by michel on 2020-09-21
 * Copyright (C) 2015-2020 Michel Fleur.
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

#include <stdexcept>

namespace org::simple::core {

/**
 * Validates indexes within a buffer of a given size. A valid index is in the
 * inclusive range zero to size minus one.
 */
struct Index {

  /**
   * Returns the index if it valid and throws out of range otherwise. This
   * checked access is generally used for higher level collections.
   *
   * @tparam S The type of index value.
   * @param index The index.
   * @param size The size of the buffer.
   * @return The valid index.
   */
  template <typename S>
  [[nodiscard]] static constexpr inline S checked(S index, S size) {
    if (index < size) {
      return index;
    }
    throw std::out_of_range("Index::checked(index): index out of range");
  }

  /**
   * Returns the index unchecked, for lowe level access to buffers.
   *
   * Using this method consistently allows to recompile with range checking to
   * facilitate troubleshooting.
   *
   * @tparam S The type of index value.
   * @param index The index.
   * @param size The size of the buffer.
   * @return The valid index.
   */
  template <typename S>
  [[nodiscard]] static constexpr inline S unchecked(S index,
                                                    [[maybe_unused]] S size) {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
    return index;
#else
    return checked(index, size);
#endif
  }
};

struct EndIndex {
  template <typename S>
  [[nodiscard]] static constexpr inline S checked(S index, S size) {
    if (index <= size) {
      return index;
    }
    throw std::out_of_range(
        "Index::Inclusive::checked(index): index out of range");
  }

  template <typename S>
  [[nodiscard]] static constexpr inline S unchecked(S index,
                                                    [[maybe_unused]] S size) {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
    return index;
#else
    return checked(index, size);
#endif
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_CORE_M_INDEX_H
