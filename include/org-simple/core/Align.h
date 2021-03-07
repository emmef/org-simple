#ifndef ORG_SIMPLE_ALIGN_H
#define ORG_SIMPLE_ALIGN_H
/*
 * org-simple/align.h
 *
 * Added by michel on 2021-03-05
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

#include <algorithm>
#include <cstddef>
#include <org-simple/core/Power2.h>
#include <org-simple/core/Size.h>

namespace org::simple::core {

static constexpr bool is_valid_alignment(size_t align) {
  return align < 2 || Power2::is(align);
}
static constexpr bool is_valid_positive_alignment(size_t align) {
  return align == 1 || Power2::is(align);
}

static constexpr size_t align_with_unchecked(size_t offset,
                                             size_t checked_align) {
  return 1 + ((offset - 1) | (checked_align - 1));
}

template <typename T> static constexpr size_t align_with(size_t offset) {
  return align_with_unchecked(offset, alignof(T));
}

static constexpr size_t align_with(size_t offset, size_t checked_align) {
  return is_valid_positive_alignment(checked_align)
             ? align_with_unchecked(offset, checked_align)
             : offset;
}

template <typename T> struct Alignment {
  /**
   * The alignment in bytes of the type parameter.
   */
  static constexpr size_t bytes = alignof(T);
  using type = T;

  /**
   * Returns if the specific alignment is a positive power of two and equal to
   * or greater than the alignment of the type parameter. The latter is always
   * a power of two.
   * @param align The specified alignment.
   * @return \c true when the the alignment is valid, \c false otherwise..
   */
  static constexpr bool is_valid(size_t align) {
    // alignof(T) is always a power of two, as is align after first check,
    // so it is justified to just check for same or bigger.
    return is_valid_positive_alignment(align) && align >= alignof(T);
  }

  /**
   * Returns a valid alignment, given the specified minimum alignment and the
   * type parameter. If the specified minimum is invalid or less than the type
   * alignment, the type alignment is returned. Otherwise, the valid minimum
   * alignment is returned.
   * @param min_alignment The specified minimum alignment.
   * @return a valid alignment meeting both type and minimym specified
   * alignments.
   */
  static constexpr size_t get_valid(size_t min_alignment) {
    return is_valid_positive_alignment(min_alignment)
               ? std::max(min_alignment, alignof(T))
               : alignof(T);
  }

  /**
   * Returns if the specified alignment is ideal for an array of elements of the
   * type parameter \c T.
   * This means that the specified alignment, a positive power of two, is equal
   * to or bigger than the alignment of \c T and smaller than the size of \c T,
   * which is always a multiple of the alignment of \c T. This way, the first
   * element will be aligned correctly as well as all consecutive elements in
   * the array.
   * @param align The specified alignment.
   * @return \c true if the specified alignment is correct
   */
  static constexpr bool is_ideal_for_array(size_t align) {
    return is_valid_positive_alignment(align) && align >= alignof(T) &&
           align <= sizeof(T);
  }

  static constexpr size_t aligned(size_t offset) {
    return align_with_unchecked(offset, alignof(T));
  }
};

template <typename T, size_t ALIGNAS> class AlignedAlloc {
  static constexpr size_t ALIGN = Alignment<T>::get_valid(ALIGNAS);

  T *alloc_;
  T *data_;
  size_t capacity_;

  void disown() {
    alloc_ = nullptr;
    data_ = nullptr;
    capacity_ = 0;
  }

public:
  AlignedAlloc(size_t count) {
    typedef typename SizeValue::Elements<sizeof(T)>::Valid Valid;
    static constexpr size_t MAX_ALIGN = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
    if constexpr (ALIGN == 0) {
      alloc_ = new T[Valid::value(count)];
      data_ = alloc_;
    } else if constexpr (ALIGN <= MAX_ALIGN) {
      alloc_ = new T[Valid::value(count), ALIGN];
      data_ = alloc_;
    } else {
      alloc_ = new T[Valid::sum(count, ALIGN - MAX_ALIGN), MAX_ALIGN];
      size_t alloc_offs = alloc_ - nullptr;
      size_t diff = align_with(alloc_offs, ALIGN) - alloc_offs;
      data_ = alloc_ + diff;
    }
    capacity_ = Valid::value(count);
  }

  AlignedAlloc(AlignedAlloc &&source)
      : alloc_(source.alloc_), data_(source.data_),
        capacity_(source.capacity_) {
    source.disown();
  }

  T *data() const noexcept { return data_; }
  size_t capacity() const noexcept { return capacity_; }

  ~AlignedAlloc() {
    if (alloc_) {
      delete[] alloc_;
      disown();
    }
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_ALIGN_H
