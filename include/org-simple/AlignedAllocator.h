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

#include <org-simple/Align.h>

namespace org::simple {

template <typename Type, size_t Align> class AlignedAllocator {
  static_assert(Align::isValid<Type>(Align));

public:
  typedef Type value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  static constexpr size_type alignment = Align::fixed<Type>(Align);

  [[maybe_unused]] typedef std::true_type
      propagate_on_container_move_assignment;
  using is_always_equal [[maybe_unused]] = std::true_type;

  constexpr AlignedAllocator() noexcept = default;
  constexpr AlignedAllocator(const AlignedAllocator &) noexcept = default;
  template <typename OtherType>
  constexpr AlignedAllocator(
      const AlignedAllocator<OtherType, alignment> &) noexcept {};
  constexpr ~AlignedAllocator() noexcept = default;

  AlignedAllocator &operator=(const AlignedAllocator &) = default;

  template <typename OtherType> struct [[maybe_unused]] rebind {
    typedef AlignedAllocator<OtherType, Align> other;
  };

  // Comparison

  template <typename OtherType, size_t A>
  friend constexpr bool
  operator==(const AlignedAllocator &,
             const AlignedAllocator<OtherType, A> &) noexcept {
    return alignment == A;
  }
  template <typename OtherType>
  friend constexpr bool operator==(const AlignedAllocator &,
                                   const std::allocator<OtherType> &) noexcept {
    return alignment == Align::maxNatural;
  }

  // The actual allocator implementation ;-)

  [[nodiscard]] inline constexpr Type *allocate(size_t elements) {
    static_assert(sizeof(Type) != 0, "cannot allocate incomplete types");

    if (elements > Align::maxElements<Type>()) {
      if (elements > std::numeric_limits<size_t>::max() / sizeof(Type)) {
        throw std::bad_array_new_length();
      }
      throw std::bad_alloc();
    }
    const size_t bytes = elements * sizeof(Type);
    if (alignment > Align::maxNatural) {
      return static_cast<Type *>(
          ::operator new(bytes, std::align_val_t(alignment)));
    }
    return static_cast<Type *>(::operator new(bytes));
  }

  void deallocate(Type *pointer,
                  size_type elements __attribute__((__unused__))) {
    if constexpr (alignment > Align::maxNatural) {
      ::operator delete(pointer, std::align_val_t(alignment));
    } else {
      ::operator delete(pointer);
    }
  }

  template<class Allocator>
  static constexpr size_t alignmentOf() {
    return subst(static_cast<const Allocator *>(nullptr));
  }
private:
  static constexpr size_t subst(...) {
    return 0;
  }
  template <typename T>
  static constexpr size_t subst(const std::allocator<T> * const) {
    return Align::maxNatural;
  }
  template <typename T, size_t A>
  static constexpr size_t subst(const AlignedAllocator<T,A> * const) {
    return A;
  }


};


} // namespace org::simple

#endif // ORG_SIMPLE_ALIGNEDALLOCATOR_H
