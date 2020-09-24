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

#include <org-simple/core/attributes.h>
#include <stdexcept>

namespace org::simple::core {
struct Index {

  template <typename S> org_nodiscard static S checked(S index, S size) {
    if (index < size) {
      return index;
    }
    throw std::out_of_range("IndexPolicy::index out of range");
  }

  template <typename S> org_nodiscard static S unchecked(S index, S) {
    return index;
  }

  /**
   * Returns checked index, or unchecked if
   * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
   */
  template <typename S> org_nodiscard static S safe(S index, S size) {
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
  template <typename S> org_nodiscard static S unsafe(S index, S size) {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
    return unchecked(index, size);
#else
    return checked(index, size);
#endif
  }

  struct Inclusive {
    template <typename S> org_nodiscard static S checked(S index, S size) {
      if (index <= size) {
        return index;
      }
      throw std::out_of_range("IndexPolicy::offset out of range");
    }

    template <typename S> org_nodiscard static S unchecked(S index, S) {
      return index;
    }

    /**
     * Returns checked index, or unchecked if
     * ORG_SIMPLE_INDEX_POLICY_FORCE_SAFE_UNCHECKED is defined.
     */
    template <typename S> org_nodiscard static S safe(S index, S size) {
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
    template <typename S> org_nodiscard static S unsafe(S index, S size) {
#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
      return unchecked(index, size);
#else
      return checked(index, size);
#endif
    }
  };
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_INDEX_H
