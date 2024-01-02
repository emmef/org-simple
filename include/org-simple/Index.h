#ifndef ORG_SIMPLE_M_INDEX_H
#define ORG_SIMPLE_M_INDEX_H
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
#include <bit>
#include <cstddef>
#include <stdexcept>

namespace org::simple {

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

template <class T> struct Align {
  static constexpr size_t alignment = alignof(T);
  static constexpr size_t maxElements =
      std::numeric_limits<size_t>::max() / alignment;
  static constexpr size_t maxAlignmentBytes =
      std::bit_ceil(std::numeric_limits<size_t>::max() / 2);
  static constexpr size_t maxAlignmentElements = std::bit_floor(maxElements);

  struct valid {
    static constexpr bool bytes(const size_t count) {
      return count == 0 ||
             (((count % alignof(T)) == 0) && std::has_single_bit(count));
    }
    static constexpr bool elements(const size_t count) {
      return count == 0 || count <= maxAlignmentElements;
    }

  };

  struct fix {
    static constexpr size_t bytes(const size_t count) {
      return (count && valid::bytes(count)) ? count : alignment;
    }

    static constexpr size_t elements(const size_t count) {
      const size_t c =
          valid::elements(count) && count != 0 ? std::bit_ceil(count) : 1;
      return c * alignment;
    }
  };

  static constexpr bool pointer(const T *ptr, size_t elements = 0) {
    return (reinterpret_cast<uintptr_t>(ptr) & fix::elements(elements)) == 0;
  }

  constexpr Align() = default;

  static constexpr Align ofBytes(size_t bytes) { return Align(bytes, true); }

  static constexpr Align ofElements(size_t elements) {
    return Align(elements, false);
  }

  constexpr size_t allocAlignment() const noexcept {
    const size_t value =
        std::bit_ceil(alignment_ < maxAlignmentBytes ? alignment_ : alignment);
    return value;
  }

  constexpr std::align_val_t align_val() const noexcept {
    return static_cast<std::align_val_t>(allocAlignment());
  }

  constexpr size_t get() const { return alignment_; }

  constexpr operator size_t() const noexcept { return get(); }

private:
  constexpr Align(size_t a, bool inBytes)
      : alignment_(inBytes ? fix::bytes(a) : fix::elements(a)) {}
  size_t alignment_ = alignment;
};

template <class T, bool allowZero = false> struct Size {
  using Alignment = Align<T>;
  static constexpr size_t maxElements = Align<T>::maxElements;
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

#endif // ORG_SIMPLE_M_INDEX_H
