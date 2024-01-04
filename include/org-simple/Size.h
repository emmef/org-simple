#ifndef ORG_SIMPLE_SIZE_H
#define ORG_SIMPLE_SIZE_H
/*
 * org-simple/Align.h
 *
 * Added by michel on 2024-01-02
 * Copyright (C) 2015-2024 Michel Fleur.
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

#include "Alignment.h"
#include <limits>
#include <stdexcept>

namespace org::simple {

template <class T, bool allowZero = false> struct Size {
  using Alignment = AlignedType<T>;
  static constexpr size_t maxElements = std::numeric_limits<size_t>::max() /
                                        sizeof(T);
  static constexpr size_t size = sizeof(T);
  static constexpr bool isValidElements(const size_t elements) {
    return elements ? elements <= maxElements : allowZero;
  }

  static constexpr bool isValidElementProduct(const size_t elements1,
                                              const size_t elements2) {
    return isValidElements(elements1) && isValidElements(elements2) && elements1
               ? maxElements / elements1 >= elements2
               : elements2 ? maxElements / elements2 >= elements1
                           : true;
  }

  static constexpr bool isValidElementProduct(const size_t elements1,
                                              const size_t elements2,
                                              const size_t elements3) {
    return isValidElementProduct(elements1, elements2) &&
                   isValidElements(elements3) && elements1
               ? maxElements / elements1 >= elements2 * elements3
               : elements2 ? maxElements / elements2 >= elements1 * elements3
                 : elements3 ? maxElements / elements3 >= elements1 * elements2
                             : true;
  }

  constexpr bool isValid() const noexcept { return size_ || allowZero; }

  constexpr size_t get() const noexcept { return size_; }

  constexpr operator size_t() const noexcept { return get(); }

  size_t getValidValue() const {
    if (isValid()) {
      return size_;
    }
    throw std::invalid_argument("Size is zero or too large");
  }

  size_t checkedIndex(size_t i) { return Index::checked(i, size_); }
  size_t checkedEndIndex(size_t i) { return Index::checked(i, size_); }

#ifndef ORG_SIMPLE_INDEX_POLICY_FORCE_UNSAFE_CHECKED
  size_t index(size_t i) const noexcept { return i; }
  size_t endIndex(size_t i) const noexcept { return i; }
#else
  size_t index(size_t i) const { return Index::checked(i, size_); }
  size_t endIndex(size_t i) const { return Index::checked(i, size_); }
#endif

  void set(size_t elements) noexcept {
    size_ = isValidElements(elements) ? elements : 0;
  }

  void operator=(size_t value) noexcept { set(value); }
  void set(size_t elements1, size_t elements2) noexcept {
    size_ =
        isValidElementProduct(elements1, elements2) ? elements1 * elements2 : 0;
  }
  void set(size_t elements1, size_t elements2, size_t elements3) noexcept {
    size_ = isValidElementProduct(elements1, elements2, elements3)
                ? elements1 * elements2 * elements3
                : 0;
  }

  constexpr Size() = default;
  constexpr Size(size_t elements) noexcept
      : size_(isValidElements(elements) ? elements : 0) {}
  constexpr Size(size_t elements1, size_t elements2) noexcept
      : size_(isValidElementProduct(elements1, elements2)
                  ? elements1 * elements2
                  : 0) {}
  constexpr Size(size_t elements1, size_t elements2, size_t elements3) noexcept
      : size_(isValidElementProduct(elements1, elements2, elements3)
                  ? elements1 * elements2 * elements3
                  : 0) {}

private:
  size_t size_ = 0;
};

template <typename T>
static constexpr bool isAlignedWith(const T *ptr, size_t elements = 0) {
  return Size<T>::pointer(ptr, elements);
}

template <typename T> using SizeIncludingZero = Size<T, true>;


} // namespace org::simple

#endif // ORG_SIMPLE_SIZE_H
