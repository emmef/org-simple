#ifndef ORG_SIMPLE_ALIGN_H
#define ORG_SIMPLE_ALIGN_H
/*
 * org-simple/core/align.h
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

/**
 * Returns is the specified alignment \c alignment is valid. This is the case if
 * it's a positive power of two.
 * @param alignment The alignment to test.
 * @return \c true if \c alignment is a positive power of two, false otherwise.
 */
static constexpr bool alignment_is_valid(size_t alignment) {
  return alignment == 1 || Power2::is(alignment);
}

/**
 * Returns is the specified alignment \c alignment is valid. This is the case if
 * it's a positive power of two and not smaller than the alignment of \c T.
 * @tparam T The type for which the validity of \c alignment must be checked.
 * @param alignment The alignment to test.
 * @return \c true if \c alignment is a positive power of two, false otherwise.
 */
template <typename T>
static constexpr bool alignment_is_valid(size_t alignment) {
  return alignment_is_valid(alignment) && alignment >= alignof(T);
}

/**
 * Returns whether specified \c offset is aligned with the, assumed valid,
 * alignment \c valid_align.
 * @param offset The offset to check.
 * @param valid_align The, assumed valid, alignment to use.
 * @return \c true if the offset is aligned with \c valid_align, \c false
 * otherwise.
 */
static constexpr bool alignment_unchecked_matches(size_t offset,
                                                  size_t valid_align) {
  return (offset & (valid_align - 1)) == 0;
}

/**
 * Returns whether \c offset is aligned with \c ALIGNAS
 * Compilation fails if \c ALIGNAS is not a valid alignment.
 * @tparam ALIGNAS The alignment to apply to \c offset.
 * @param offset The offset to test.
 * @return \c true if the offset is aligned with \c ALIGNAS, \c false otherwise.
 */
template <size_t ALIGNAS>
static constexpr bool alignment_matches(size_t offset) {
  static_assert(alignment_is_valid(ALIGNAS));
  return alignment_unchecked_matches(offset, ALIGNAS);
}

/**
 * Returns whether \c offset is aligned with the natural alignment of type \c
 * T.
 * @tparam T The type whose natural alignment to test \c offset with.
 * @param offset The offset to test.
 * @return \c true if the offset is aligned with \c T's natural alignment, \c
 * false otherwise.
 */
template <typename T> static constexpr bool alignment_matches(size_t offset) {
  return alignment_unchecked_matches(offset, alignof(T));
}

/**
 * Returns whether \c offset is aligned with \c alignment or \c false if \c
 * alignment is not a valid alignment..
 * @param offset The offset to test.
 * @param alignment The alignment to use.
 * @return \c true if \c alignment is a valid alignment and the offset is
 * aligned with it, \c false otherwise.
 */
static constexpr bool alignment_matches(size_t offset, size_t alignment) {
  return alignment_is_valid(alignment) &&
         alignment_unchecked_matches(offset, alignment);
}

/**
 * Returns the offset with the, assumed valid, alignment \c valid_align applied.
 * The result will be \c offset if it's already aligned and the next multiple of
 * \c valid_align otherwise.
 * @param offset The offset whose aligned value must be returned.
 * @param valid_align The, assumed valid, alignment to apply to \c offset.
 * @return the aligned value of \c offset.
 */
static constexpr size_t alignment_unchecked_apply(size_t offset,
                                                  size_t valid_align) {
  return 1 + ((offset - 1) | (valid_align - 1));
}

/**
 * Returns the offset with the alignment \c ALIGNAS applied.
 * Compilation fails if \c ALIGNAS is not a valid alignment.
 * @tparam ALIGNAS The alignment to apply to \c offset.
 * @param offset The offset whose aligned value must be returned.
 * @return the aligned value of \c offset.
 */
template <size_t ALIGNAS>
static constexpr size_t alignment_apply(size_t offset) {
  static_assert(alignment_is_valid(ALIGNAS));
  return alignment_unchecked_apply(offset, ALIGNAS);
}

/**
 * Returns the offset with the natural alignment of type \c T applied.
 * @tparam T The type whose natural alignment to apply to \c offset.
 * @param offset The offset whose aligned value must be returned.
 * @return the aligned value of \c offset.
 */
template <typename T> static constexpr size_t alignment_apply(size_t offset) {
  return alignment_unchecked_apply(offset, alignof(T));
}

/**
 * Returns the offset with the alignment \c alignment applied or \c offset if \c
 * alignment is not a valid alignment.
 * @param offset The offset whose aligned value must be returned.
 * @param alignment The, assumed valid, alignment to apply to \c offset.
 * @return the aligned value of \c offset.
 */
static constexpr size_t alignment_apply(size_t offset, size_t alignment) {
  return alignment_is_valid(alignment)
             ? alignment_unchecked_apply(offset, alignment)
             : offset;
}

/**
 * Returns correct alignment for the specified type \c T based on the suggested
 * alignment \c suggestion. If \c suggestion is both a valid alignment and
 * it is bigger than the natural alignment of \c T, \c suggestion is returned.
 * Otherwise, the natural alignment of \c T is returned.
 * @tparam T The type that the alignment should apply to.
 * @param suggestion The suggested alignment.
 * @return the correct alignment value, equal to or larger than \c alignof(T).
 */
template <typename T>
static constexpr size_t alignment_get_correct(size_t suggestion) {
  return alignment_is_valid(suggestion) && suggestion > alignof(T) ? suggestion
                                                                   : alignof(T);
}

/**
 * Applies correct alignment to \c ptr for the specified type \c T based on the
 * suggested alignment \c suggestion.
 * @tparam T The type that the alignment should apply to.
 * @param suggestion The suggested alignment.
 * @param ptr The pointer to align
 * @return the aligned pointer.
 */
template <typename T>
static constexpr T *alignment_apply_correct(size_t suggestion, T *ptr) {
  return (T *)alignment_unchecked_apply((uintptr_t)ptr,
                                        alignment_get_correct<T>(suggestion));
}

template <typename T, size_t ALIGNAS> class AlignedAlloc {
  static constexpr size_t ALIGN = alignment_get_correct<T>(ALIGNAS);
  static constexpr bool ALIGNED_ALLOC =
      ALIGN > __STDCPP_DEFAULT_NEW_ALIGNMENT__;

  T *data_;
  size_t capacity_;

  void disown() {
    data_ = nullptr;
    capacity_ = 0;
  }

public:
  explicit AlignedAlloc(size_t count) {
    capacity_ = SizeValue::Elements<sizeof(T)>::Valid::value(count);
    if constexpr (ALIGNED_ALLOC) {
      data_ = new (std::align_val_t{ALIGN}) T[capacity_];
    } else {
      data_ = new T[capacity_];
    }
  }

  AlignedAlloc(AlignedAlloc &&source) noexcept
      : data_(source.data_), capacity_(source.capacity_) {
    source.disown();
  }

  [[nodiscard]] T *data() const { return data_; }
  [[nodiscard]] size_t capacity() const { return capacity_; }

  ~AlignedAlloc() {
    if (data_) {
      delete[] data_;
      disown();
    }
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_ALIGN_H
