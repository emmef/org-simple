#ifndef ORG_SIMPLE_INDEX_H
#define ORG_SIMPLE_INDEX_H
/*
 * org-simple/Index.h
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

#ifdef ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED
#define ORG_SIMPLE_INDEX_SAFE_NOEXCEPT noexcept
#else
#define ORG_SIMPLE_INDEX_SAFE_NOEXCEPT
#endif

#ifdef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
#define ORG_SIMPLE_INDEX_UNSAFE_NOEXCEPT
#else
#define ORG_SIMPLE_INDEX_UNSAFE_NOEXCEPT noexcept
#endif

#ifdef ORG_SIMPLE_DEREFERENCE_SAFE_UNCHECKED
#define ORG_SIMPLE_DEREFERENCE_SAFE_NOEXCEPT noexcept
#else
#define ORG_SIMPLE_DEREFERENCE_SAFE_NOEXCEPT
#endif

#ifdef ORG_SIMPLE_DEREFERENCE_UNSAFE_CHECKED
#define ORG_SIMPLE_DEREFERENCE_UNSAFE_NOEXCEPT
#else
#define ORG_SIMPLE_DEREFERENCE_UNSAFE_NOEXCEPT noexcept
#endif

namespace org::simple::core {

struct Index {

  template <typename S> [[nodiscard]] static S checked(S index, S size) {
    if (index < size) {
      return index;
    }
    throw std::out_of_range("Index::checked(index): index out of range");
  }

  template <typename S> [[nodiscard]] static S unchecked(S index, S) noexcept {
    return index;
  }

  /**
   * Returns checked index, or unchecked if
   * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
   */
  template <typename S>
  [[nodiscard]] static S safe(S index, S size) ORG_SIMPLE_INDEX_SAFE_NOEXCEPT {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED
    return checked(index, size);
#else
    return unchecked(index, size);
#endif
  }

  /**
   * Returns unchecked index, or checked if
   * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
   */
  template <typename S>
  [[nodiscard]] static S unsafe(S index,
                                S size) ORG_SIMPLE_INDEX_UNSAFE_NOEXCEPT {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
    return unchecked(index, size);
#else
    return checked(index, size);
#endif
  }

  struct Inclusive {
    template <typename S> [[nodiscard]] static S checked(S index, S size) {
      if (index <= size) {
        return index;
      }
      throw std::out_of_range(
          "Index::Inclusive::checked(index): index out of range");
    }

    template <typename S>
    [[nodiscard]] static S unchecked(S index, S) noexcept {
      return index;
    }

    /**
     * Returns checked index, or unchecked if
     * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
     */
    template <typename S>
    [[nodiscard]] static S safe(S index,
                                S size) ORG_SIMPLE_INDEX_SAFE_NOEXCEPT {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED
      return checked(index, size);
#else
      return unchecked(index, size);
#endif
    }

    /**
     * Returns unchecked index, or checked if
     * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
     */
    template <typename S>
    [[nodiscard]] static S unsafe(S index,
                                  S size) ORG_SIMPLE_INDEX_UNSAFE_NOEXCEPT {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
      return unchecked(index, size);
#else
      return checked(index, size);
#endif
    }
  };
};

struct Dereference {

  template <typename T> [[nodiscard]] static T *checked(T *ptr) {
    if (ptr) {
      return ptr;
    }
    throw std::invalid_argument("Dereferece::checked(ptr): nullptr.");
  }

  template <typename T> [[nodiscard]] static T *unchecked(T *ptr) noexcept {
    if (ptr) {
      return ptr;
    }
    throw std::invalid_argument("Dereferece::checked(ptr): nullptr.");
  }

  template <typename T>
  [[nodiscard]] static T *safe(T *ptr) ORG_SIMPLE_DEREFERENCE_SAFE_NOEXCEPT {
#ifdef ORG_SIMPLE_DEREFERENCE_SAFE_UNCHECKED
    return unchecked(ptr);
#else
    return checked(ptr);
#endif
  }

  template <typename T>
  [[nodiscard]] static T *
  unsafe(T *ptr) ORG_SIMPLE_DEREFERENCE_UNSAFE_NOEXCEPT {
#ifdef ORG_SIMPLE_DEREFERENCE_UNSAFE_CHECKED
    return checked(ptr);
#else
    return unchecked(ptr);
#endif
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_INDEX_H
