#ifndef ORG_SIMPLE_ALIGNEDALLOCATOR_H
#define ORG_SIMPLE_ALIGNEDALLOCATOR_H
/*
 * org-simple/AlignedAllocator.h
 *
 * Added by michel on 2024-01-05
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

#include <org-simple/Alignment.h>

namespace org::simple {

template <typename Type,
          size_t Align = std::max(alignof(Type), Align::maxNatural)>
class BaseAlignedAllocator {
public:
  typedef Type value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  static constexpr size_type alignment = Align::fixWithMaxNatural<Type>(Align);

  template <typename OtherType> struct rebind {
    typedef BaseAlignedAllocator<OtherType, Align> other;
  };

  typedef std::true_type propagate_on_container_move_assignment;

  constexpr BaseAlignedAllocator() noexcept {}

  constexpr BaseAlignedAllocator(const BaseAlignedAllocator &) noexcept {}

  template <typename OtherType>
  constexpr BaseAlignedAllocator(
      const BaseAlignedAllocator<OtherType, alignment> &) noexcept {}

  [[nodiscard]] Type *allocate(size_type elements,
                               const void * = static_cast<const void *>(0)) {
    static_assert(sizeof(Type) != 0, "cannot allocate incomplete types");
    if (elements > Align::maxElements<Type>()) {
      if (elements > (std::numeric_limits<size_t>::max() / sizeof(Type))) {
        std::__throw_bad_array_new_length();
      }
      std::__throw_bad_alloc();
    }
    if constexpr (alignment > Align::maxNatural) {
      return static_cast<Type *>(
          ::operator new(elements * sizeof(Type), std::align_val_t(alignment)));
    }
    return static_cast<Type *>(::operator new(elements * sizeof(Type)));
  }

  void deallocate(Type *pointer,
                  size_type elements __attribute__((__unused__))) {
    if constexpr (alignment > Align::maxNatural) {
      ::operator delete(pointer,
                        std::align_val_t(alignment) /*, __n * sizeof(Type)*/);
      return;
    }
    ::operator delete(pointer /*, __n * sizeof(Type)*/);
  }

  template <typename OtherType, size_t A>
  friend constexpr bool
  operator==(const BaseAlignedAllocator &,
             const BaseAlignedAllocator<OtherType, A> &) noexcept {
    return alignment == A;
  }
  template <typename OtherType>
  friend constexpr bool
  operator==(const BaseAlignedAllocator &,
             const BaseAlignedAllocator<OtherType> &) noexcept {
    return alignment == BaseAlignedAllocator<OtherType>::alignment;
  }
  template <typename OtherType>
  friend constexpr bool operator==(const BaseAlignedAllocator &,
                                   const std::allocator<OtherType> &) noexcept {
    return alignment == Align::maxNatural;
  }
};

template <typename Type,
          size_t _Align = std::max(alignof(Type), Align::maxNatural)>
class AlignedAllocator
    : public BaseAlignedAllocator<Type,
                                  Align::fixWithMaxNatural<Type>(_Align)> {
public:
  typedef Type value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  using Base =
      BaseAlignedAllocator<Type, Align::fixWithMaxNatural<Type>(_Align)>;
  using Base::alignment;

  template <typename OtherType> struct rebind {
    typedef AlignedAllocator<OtherType, alignment> other;
  };

  using propagate_on_container_move_assignment [[maybe_unused]] =
      std::true_type;

  using is_always_equal [[maybe_unused]] = std::true_type;

  constexpr AlignedAllocator() noexcept {}

  constexpr AlignedAllocator(const AlignedAllocator &allocator) noexcept
      : Base(allocator) {}

  AlignedAllocator &operator=(const AlignedAllocator &) = default;

  template <typename OtherType>
  constexpr AlignedAllocator(
      const AlignedAllocator<OtherType, alignment> &) noexcept {}

  constexpr ~AlignedAllocator() noexcept {}

  [[nodiscard]] inline constexpr Type *allocate(size_t elements) {
    if (std::__is_constant_evaluated()) {
      if (__builtin_mul_overflow(elements, sizeof(Type), &elements))
        std::__throw_bad_array_new_length();
      if constexpr (alignment > Align::maxNatural) {
        return static_cast<Type *>(
            ::operator new(elements, std::align_val_t(alignment)));
      }
      return static_cast<Type *>(::operator new(elements));
    }

    return Base::allocate(elements, 0);
  }

  inline constexpr void deallocate(Type *pointer, size_t elements) {
    if (std::__is_constant_evaluated()) {
      ::operator delete(pointer);
      return;
    }
    Base::deallocate(pointer, elements);
  }

  friend constexpr bool operator==(const AlignedAllocator &,
                                   const AlignedAllocator &) noexcept {
    return true;
  }

  template <typename OtherType>
  friend constexpr bool
  operator==(const AlignedAllocator &,
             const AlignedAllocator<OtherType> &) noexcept {
    return alignment == AlignedAllocator<OtherType>::alignment;
  }
};

} // namespace org::simple

#endif // ORG_SIMPLE_ALIGNEDALLOCATOR_H
