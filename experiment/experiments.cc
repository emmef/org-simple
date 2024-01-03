//
// Created by michel on 27-03-21.
//

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <span>

struct Align {
  static constexpr size_t maxAlign =
      std::bit_floor(std::numeric_limits<unsigned short>::max());

  template <class type> static constexpr bool isValid(size_t align) noexcept {
    return !align || (align < maxAlign && std::has_single_bit(align) &&
                      align >= alignof(type));
  }

  template <class type> static constexpr size_t valid(size_t align) noexcept {
    return align && isValid<type>(align) ? align : alignof(type);
  }

  template <class type> static constexpr size_t maxCount() noexcept {
    return std::numeric_limits<size_t>::max() / sizeof(type);
  }

  template <class type>
  static constexpr size_t validCount(size_t count) noexcept {
    return std::min(count, maxCount<type>());
  }

  template <class type>
  static constexpr size_t alignOffset(size_t count, size_t align) noexcept {
    const size_t c = validCount<type>(count);
    const size_t a = valid<type>(align);
    const size_t b = sizeof(type) * c;
    return b <= a ? a : a * (1 + (b + a - 1) / a);
  }

  template <class type>
  static constexpr bool isAlignedPtr(const type *const ptr, size_t alignment) {
    return ptr != nullptr &&
           static_cast<uintptr_t>(ptr) % valid<type>(alignment) == 0;
  }

  static constexpr bool isAlignedOffset(size_t offset, size_t alignment) {
    return !offset || offset % alignment == 0;
  }
};

struct Empty {};

template <typename T, size_t S, size_t A> struct AlignedInfo {
  using type = T;
  static_assert(Align::isValid<T>(A));
  static constexpr size_t align = Align::valid<T>(A);
  static_assert(align % sizeof(T) == 0);
  static constexpr size_t capacity = Align::validCount<T>(S);
  struct BaseContainer {
    using type = std::array<T, capacity>;
    alignas(align) type array;
  };

  static constexpr size_t alignedArrayBytes = sizeof(BaseContainer);
  static_assert(alignedArrayBytes % sizeof(T) == 0);
  static constexpr size_t alignedCount = alignedArrayBytes / sizeof(T);

  template <size_t sliceSize> struct slice {
    static constexpr bool valid = true;
    static_assert(sliceSize != 0 && capacity != 0 &&
                  (capacity % sliceSize) == 0);

    static constexpr size_t sliceCount = capacity / sliceSize;

    template <size_t sliceIndex> static constexpr bool isValidSliceIndex() {
      return sliceIndex < sliceCount;
    }

    static constexpr bool isValidSliceIndex(size_t sliceIndex) {
      return sliceIndex < sliceCount;
    }

    static constexpr size_t validSliceIndex(size_t sliceIndex) {
      if (isValidSliceIndex(sliceIndex)) {
        return sliceIndex;
      }
      throw std::invalid_argument("Invalid slice-index");
    }
  };
};

template <typename T, size_t S, size_t A, bool isConst = false>
struct AlignedReference;

template <typename T, size_t S, size_t A, class Sub> class AlignedObject {
public:
  static constexpr auto align = AlignedInfo<T, S, A>::align;
  static constexpr auto capacity = AlignedInfo<T, S, A>::capacity;

  template <size_t sliceSize>
  static constexpr bool isValidSliceSize =
      AlignedInfo<T, S, A>::template slice<sliceSize>::valid;

  template <size_t sliceSize, size_t sliceIndex>
  static constexpr bool isValidSliceIndex =
      isValidSliceSize<sliceSize> &&
      AlignedInfo<T, S, A>::template slice<
          sliceSize>::template isValidSliceIndex<sliceIndex>();

  constexpr const T *data() const noexcept {
    return std::assume_aligned<align>(
        static_cast<const Sub *>(this)->wrapper().array.data());
  }
  constexpr T *data() noexcept {
    auto *const ptr = static_cast<Sub *>(this)->wrapper().array.data();
    return std::assume_aligned<align>(ptr);
  }
  constexpr const T &operator[](size_t i) const noexcept {
    return std::assume_aligned<align>(
        static_cast<const Sub *>(this)->wrapper().array.data())[std::min(
        i, capacity - 1)];
  }
  constexpr T &operator[](size_t i) noexcept {
    return std::assume_aligned<align>(
        static_cast<Sub *>(this)->wrapper().array.data())[std::min(i, capacity -
                                                                          1)];
  }
  constexpr auto &container() noexcept {
    return static_cast<Sub *>(this)->wrapper().array;
  }
  constexpr const auto &container() const noexcept {
    return static_cast<const Sub *>(this)->wrapper().array;
  }

//  template <size_t sliceSize, size_t sliceIndex>
//    requires(isValidSliceIndex<sliceSize, sliceIndex>)
//  auto getSlice() const noexcept {
//    return AlignedReference<T, sliceSize, A, true>(
//        *static_cast<const Sub *>(this));
//  }
//
//  template <size_t sliceSize, size_t sliceIndex>
//    requires(isValidSliceIndex<sliceSize, sliceIndex>)
//  AlignedReference<T, sliceSize, A, false> getSlice() noexcept {
//    return { data(), sliceIndex };
//  }
//
//  template <size_t sliceSize>
//    requires(isValidSliceSize<sliceSize>)
//  auto getSlice(size_t sliceIndex) const {
//    return template <sliceSize>{ false, data()};
//  }
//
//  template <size_t sliceSize>
//    requires(isValidSliceSize<sliceSize>)
//  auto getSlice(size_t sliceIndex) {
//    return AlignedReference<T, sliceSize, A, false>(
//        *static_cast<Sub *>(this, sliceIndex));
//  }
};

template <typename T, size_t A, class Sub> struct AlignedObject<T, 0, A, Sub> {
  constexpr const T &operator[](size_t) const noexcept {
    return *static_cast<const Sub *>(this)->wrapper().array.begin();
  }
  constexpr T &operator[](size_t) noexcept {
    return *static_cast<Sub *>(this)->wrapper().array.begin();
  }
};

template <typename T, size_t S, size_t A>
struct AlignedArray : public AlignedObject<T, S, A, AlignedArray<T, S, A>>,
                      public AlignedInfo<T, S, A> {
  using typename AlignedInfo<T, S, A>::type;
  using AlignedInfo<T, S, A>::align;
  using AlignedInfo<T, S, A>::capacity;
  using AlignedInfo<T, S, A>::alignedArrayBytes;
  using AlignedInfo<T, S, A>::alignedCount;
  using Container = typename AlignedInfo<T, S, A>::BaseContainer;
  friend class AlignedObject<T, S, A, AlignedArray<T, S, A>>;

private:
  Container container_;

  constexpr const Container &wrapper() const noexcept { return container_; }
  constexpr Container &wrapper() noexcept { return container_; }
};

template <typename T, size_t S, size_t A>
struct AlignedPointer : public AlignedObject<T, S, A, AlignedPointer<T, S, A>>,
                        public AlignedInfo<T, S, A> {
  using typename AlignedInfo<T, S, A>::type;
  using AlignedInfo<T, S, A>::align;
  using AlignedInfo<T, S, A>::capacity;
  using AlignedInfo<T, S, A>::alignedArrayBytes;
  using AlignedInfo<T, S, A>::alignedCount;
  using Container = typename AlignedInfo<T, S, A>::BaseContainer;
  friend class AlignedObject<T, S, A, AlignedPointer<T, S, A>>;

  AlignedPointer(const AlignedPointer &) = delete;
  AlignedPointer(AlignedPointer &&s) : container_(s.container_) {
    s.container_ = nullptr;
  }
  AlignedPointer() = default;
  ~AlignedPointer() {
    if (container_) {
      delete container_;
      container_ = nullptr;
    }
  }

private:
  Container *container_ = new Container;

  constexpr const Container &wrapper() const noexcept { return *container_; }
  constexpr Container &wrapper() noexcept { return *container_; }
};

template <class T, size_t S, size_t A, bool isConst> struct ReferenceContainer {
  using type = std::span<T, AlignedInfo<T, S, A>::capacity>;
  alignas(AlignedInfo<T, S, A>::align) type array;
  ReferenceContainer(T *source) : array(source, S) {}
};

template <class T, size_t S, size_t A>
struct ReferenceContainer<T, S, A, true> {
  using type = std::span<const T, AlignedInfo<const T, S, A>::capacity>;
  alignas(AlignedInfo<T, S, A>::align) type array;
  ReferenceContainer(const T *const source) : array(source, S) {}
};

template <class T, size_t S, size_t A, bool isConst> struct ConstWrapper {
  T *begin_;
  T *end_;
  using containerType = ReferenceContainer<T, S, A, false>;
  containerType container_;

  ConstWrapper(T *begin, size_t offset)
      : begin_(begin), end_(begin + AlignedInfo<const T, S, A>::capacity),
        container_(begin + offset * AlignedInfo<const T, S, A>::alignedCount) {}
};

template <class T, size_t S, size_t A> struct ConstWrapper<T, S, A, true> {
  const T *begin_;
  const T *end_;
  using containerType = ReferenceContainer<T, S, A, true>;
  containerType container_;

  ConstWrapper(T *begin, size_t offset)
      : begin_(begin), end_(begin + AlignedInfo<const T, S, A>::capacity),
        container_(begin + offset * AlignedInfo<const T, S, A>::alignedCount) {}
  ConstWrapper(const T *begin, size_t offset)
      : begin_(begin), end_(begin + AlignedInfo<const T, S, A>::capacity),
        container_(begin + offset * AlignedInfo<const T, S, A>::alignedCount) {}
};

template <typename T, size_t S, size_t A, bool isConst>
struct AlignedReference
    : public AlignedObject<T, S, A, AlignedReference<T, S, A>>,
      public AlignedInfo<T, S, A> {
  static_assert(S > 0);
  using typename AlignedInfo<T, S, A>::type;
  using AlignedInfo<T, S, A>::align;
  using AlignedInfo<T, S, A>::capacity;
  using AlignedInfo<T, S, A>::alignedArrayBytes;
  using AlignedInfo<T, S, A>::alignedCount;
  using ConstWrapper = ConstWrapper<T, S, A, isConst>;
  using Container = typename ConstWrapper::containerType;
  friend class AlignedObject<T, S, A, AlignedReference<T, S, A>>;

  template <size_t ownerSize>
  using sliceInfo = typename AlignedInfo<T, ownerSize, A>::template slice<S>;

  template <size_t ownerSize>
  static constexpr bool isValidSliceSize = sliceInfo<ownerSize>::valid;

  template <size_t ownerSize, size_t sliceIndex>
  static constexpr bool isValidSliceIndex =
      isValidSliceSize<ownerSize> &&
      sliceInfo<ownerSize>::template isValidSliceIndex<sliceIndex>();

  template <size_t ownerSize>
    requires(isValidSliceSize<ownerSize> && isConst)
  AlignedReference(const AlignedArray<T, ownerSize, A> &input,
                   size_t sliceIndex)
      : sw_(input.data(), sliceInfo<ownerSize>::validSliceIndex(sliceIndex)) {}

  template <size_t ownerSize>
    requires(isValidSliceSize<ownerSize>)
  AlignedReference(AlignedPointer<T, ownerSize, A> &input, size_t sliceIndex)
      : sw_(input.data(), sliceInfo<ownerSize>::validSliceIndex(sliceIndex)) {}

  template <size_t ownerSize>
    requires(isValidSliceSize<ownerSize> && isConst)
  AlignedReference(const AlignedPointer<T, ownerSize, A> &input,
                   size_t sliceIndex) noexcept
      : sw_(input.data(), sliceInfo<ownerSize>::validSliceIndex(sliceIndex)) {}

  template <size_t ownerSize, size_t sliceIndex>
    requires(isValidSliceIndex<ownerSize, sliceIndex>)
  AlignedReference(AlignedArray<T, ownerSize, A> &input) noexcept
      : sw_(input.data(), sliceIndex) {}

  template <size_t ownerSize, size_t sliceIndex>
    requires(isValidSliceIndex<ownerSize, sliceIndex> && isConst)
  AlignedReference(const AlignedArray<T, ownerSize, A> &input)
      : sw_(input.data(), sliceIndex) {}

  template <size_t ownerSize, size_t sliceIndex>
    requires(isValidSliceIndex<ownerSize, sliceIndex>)
  AlignedReference(AlignedPointer<T, ownerSize, A> &input) noexcept
      : sw_(input.data(), sliceIndex) {}

  template <size_t ownerSize, size_t sliceIndex>
    requires(isValidSliceIndex<ownerSize, sliceIndex> && isConst)
  AlignedReference(const AlignedPointer<T, ownerSize, A> &input) noexcept
      : sw_(input.data(), sliceIndex) {}

  template <size_t ownerSize, size_t sliceIndex, bool check = true>
    requires(isValidSliceIndex<ownerSize, sliceIndex> && isConst)
  AlignedReference(const T *owner)
      : sw_(validPointerValue(owner), sliceIndex) {}

  template <size_t ownerSize, size_t sliceIndex>
    requires(isValidSliceIndex<ownerSize, sliceIndex>)
  AlignedReference(T *owner) : sw_(validPointerValue(owner), sliceIndex) {}

  template <size_t ownerSize>
    requires(isValidSliceSize<ownerSize> && isConst)
  AlignedReference(const T *owner, size_t sliceIndex)
      : sw_(validPointerValue(owner), sliceIndex) {}

  template <size_t ownerSize>
    requires(isValidSliceSize<ownerSize>)
  AlignedReference(T *owner, size_t sliceIndex)
      : sw_(validPointerValue(owner),
            sliceInfo<ownerSize>::validSliceIndex(sliceIndex)) {}

  size_t offset() const noexcept {
    return (static_cast<uintptr_t>(
                AlignedObject<T, S, A, AlignedReference<T, S, A>>::data()) -
            static_cast<uintptr_t>(sw_.begin_)) /
           alignedArrayBytes;
  }

  AlignedReference(AlignedReference &&source) = default;

private:

  ConstWrapper sw_;

  constexpr const auto &wrapper() const noexcept { return sw_.container_; }
  constexpr auto &wrapper() noexcept { return sw_.container_; }

  constexpr size_t sourceCount() const noexcept {
    return sw_.end_ - sw_.begin_;
  }

  template <typename Z>
    requires(std::is_same_v<Z, T *> || std::is_same_v<Z, const T *>)
  constexpr bool isValidPointerValue(Z ptr) {
    return (reinterpret_cast<uintptr_t>(ptr) % alignedArrayBytes) == 0;
  }

  template <typename Z>
    requires(std::is_same_v<Z, T *> || std::is_same_v<Z, const T *>)
  constexpr Z validPointerValue(Z ptr) {
    if (isValidPointerValue(ptr)) {
      return ptr;
    }
    throw std::invalid_argument("Pointer is not aligned");
  }

  constexpr size_t sourceArrays() const noexcept {
    return sourceCount() / alignedCount;
  }

  size_t validInitialOffset(size_t len, size_t offset) const noexcept {
    const auto frames = len / alignedCount;
    if (offset < frames) {
      return offset * alignedCount;
    }
    return 0;
  }
};

AlignedArray<double, 3, 32> double_array_3_32;
AlignedArray<double, 13, 32> double_array_13_32;
AlignedArray<double, 16, 32> double_array_16_32;
AlignedArray<double, 17, 32> double_array_17_32;

AlignedPointer<double, 3, 32> double_pointer_3_32;
AlignedPointer<double, 13, 32> double_pointer_13_32;
AlignedPointer<double, 16, 32> double_pointer_16_32;
AlignedPointer<double, 17, 32> double_pointer_17_32;
std::array<double, 4> array4;

template <class type> void fillConsecutive(type &value) {
  int i = 0;
  for (auto &v : value.container()) {
    v = i++;
  }
}

template <class type>
void printvalues(const type &value, const char *description) {
  printf("%10s {", description);
  for (auto &v : value.container()) {
    printf(" %lf", v);
  }
  printf(" }\n");
}

int main(int, char **) {
  printf("Align::maxAlign           = %zu\n", Align::maxAlign);
  printf("alignof(Empty)            = %zu\n", alignof(Empty));
  printf("Align::valid<Empty>(0)    = %zu\n", Align::valid<Empty>(0));

  auto &ref = double_array_16_32;
  fillConsecutive(ref);
//  ref.getSlice<4, 1>();
    AlignedReference<double, 4, 32, true> slice(ref, 1);

  //  printvalues(ref, "owner");
  //  printvalues(slice, "slice");
  //
  //  printf("&reference.data() - &ref.data() = %zu\n",
  //         (reinterpret_cast<uintptr_t>(slice.data()) -
  //          reinterpret_cast<uintptr_t>(ref.data())));
}
