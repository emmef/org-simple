//
// Created by michel on 27-03-21.
//

#include <iostream>
#include <org-simple/AlignedAllocator.h>
#include <org-simple/AlignedData.h>
#include <vector>

// SFINAE test
template <typename T> class FloatingPointOps {
  typedef bool YesType;
  typedef long NoType;

  template <class X, class Sub, bool isArithmetic = std::is_arithmetic_v<X>>
  struct Helper1;

  template <class X, class Sub> struct Helper1<X, Sub, true> {
    static constexpr bool value = true;
  };

  template <class X, class Sub> struct Helper1<X, Sub, false> {

    static constexpr bool value =
        std::is_arithmetic_v<X> ||
        std::is_same_v<decltype(Sub::template test<T>(std::declval<T *>())),
                       YesType>;
  };

public:
  struct Multiply {
    struct Assign : public Helper1<T, Assign> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator*=(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    struct Binary : public Helper1<T, Binary> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator*(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    static constexpr bool value = Binary::value && Assign::value;
  };
  struct Divide {
    struct Assign : public Helper1<T, Assign> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator/=(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    struct Binary : public Helper1<T, Binary> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator/(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    static constexpr bool value = Binary::value && Assign::value;
  };
  struct Add {
    struct Assign : public Helper1<T, Assign> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator+=(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    struct Binary : public Helper1<T, Binary> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator+(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    static constexpr bool value = Binary::value && Assign::value;
  };
  struct Subtract {
    struct Assign : public Helper1<T, Assign> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator-=(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    struct Binary : public Helper1<T, Binary> {
      template <typename C>
      static constexpr YesType
      test(decltype(&std::declval<C>().operator-(std::declval<double>())));

      template <typename C> static constexpr NoType test(...);
    };
    static constexpr bool value = Binary::value && Assign::value;
  };

  template <bool isArithmetic = std::is_arithmetic_v<T>, typename Bogus = bool>
  struct All {
    static constexpr bool value = true;
  };

  template <typename Bogus> struct All<false, Bogus> {
    static constexpr bool value =
        Multiply::value && Divide::value && Add::value && Subtract::value;
  };

  static constexpr bool value = All<std::is_arithmetic_v<T>, bool>::value;
};

struct Empty {};
struct HasOpMul {
  template <typename F>
    requires(std::is_arithmetic_v<F>)
  HasOpMul &operator*=(F v) {
    return *this;
  }
};
//
// using namespace org::simple;
//
// template <typename T, size_t elements, size_t alignment>
// using AlignedArray =
//    AlignedData<T, elements, alignment, AlignedDataType::ARRAY, false>;
// template <typename T, size_t elements, size_t alignment>
// using AlignedPointer = AlignedData<T, elements, alignment,
//                                   AlignedDataType::ALLOCATED_ARRAY, false>;
// template <typename T, size_t elements, size_t alignment>
// using AlignedReference =
//    AlignedData<T, elements, alignment, AlignedDataType::REFERENCE, false>;
//
// template <class type> void fillConsecutive(type &value) {
//  int i = 0;
//  for (auto &v : value.getInternalStorage()) {
//    v = i++;
//  }
//}
//
// std::ostream &operator<<(std::ostream &out, const AlignedDataInfo &info) {
//  if (!info.isAlignedData) {
//    out << "Not AlignedData!";
//    return out;
//  }
//  out << "AlignedDataInfo<";
//  out << "elements=" << info.elements;
//  out << ",alignment=" << info.alignment;
//  out << ",type=";
//  switch (info.type) {
//  case org::simple::AlignedDataType::ARRAY:
//    out << "ARRAY";
//    break;
//  case org::simple::AlignedDataType::ALLOCATED_ARRAY:
//    out << "ALLOCATED_ARRAY";
//    break;
//  case org::simple::AlignedDataType::REFERENCE:
//    out << "REFERENCE";
//    break;
//  default:
//    out << "NONE";
//  }
//  out << ",isConst=" << (info.isConst ? "true" : "false") << ">";
//  return out;
//}
//
// template <class T>
// void printAlignedDataInfo(const T &data, const char *message) {
//  constexpr auto info = AlignedDataInfo::get<T>();
//  std::cout << std::endl
//            << message << " (" << typeid(T).name() << ")" << std::endl;
//  std::cout << "\t" << info << std::endl;
//  if constexpr (info.isAlignedData) {
//    std::cout << "\tData location " << data.data() << std::endl;
//    std::cout << "\t" << message << ":" << std::endl << "\t\t{";
//    for (auto &v : data) {
//      std::cout << " " << v;
//    }
//    std::cout << " }" << std::endl;
//  }
//}
//
// AlignedArray<double, 3, 32> double_array_3_32;
// AlignedArray<double, 13, 32> double_array_13_32;
// AlignedArray<double, 16, 32> double_array_16_32;
// AlignedArray<double, 17, 32> double_array_17_32;
//
// AlignedPointer<double, 3, 32> double_pointer_3_32;
// AlignedPointer<double, 13, 32> double_pointer_13_32;
// AlignedPointer<double, 16, 32> double_pointer_16_32;
// AlignedPointer<double, 17, 32> double_pointer_17_32;

template <class C> void multiply(double by, C &vector) {
  for (auto &v : vector) {
    v *= by;
  }
}

template <class I> void multiply(double by, I begin, I end) {
  I x = std::assume_aligned<32>(begin);
  for (I p = x; p < end; p++) {
    *p *= by;
  }
}

template <class C> void fillRandom(C &vector) {
  for (auto &v : vector) {
    v = std::rand();
  }
}

template <typename T> void printHasOperator() {
  std::cout << typeid(T).name() << ":"
            << (FloatingPointOps<T>::Multiply::Assign::value ? " has "
                                                             : " has no ")
            << "operator*="
            << (FloatingPointOps<T>::Multiply::Binary::value ? " has "
                                                             : " has no ")
            << "operator*" << std::endl;
}

template <typename T> void printValidAlignedAccessBase() {
  std::cout << std::endl << "Type " << typeid(T).name();

  if (!org::simple::AlignedContainerInfo<T>::isValid) {
    std::cout << " is not a valid aligned container base." << std::endl;
    return;
  }
  std::cout << " contains type "
            << typeid(typename org::simple::AlignedContainerInfo<T>::valueType)
                   .name();
  if (org::simple::AlignedContainerInfo<T>::isArray) {
    std::cout << " as an aligned array with size "
              << org::simple::AlignedContainerInfo<T>::fixedSize;
  }
  else {
    std::cout << " as an container with aligned allocator";
  }
  std::cout << ", alignment is " << org::simple::AlignedContainerInfo<T>::alignment << std::endl;
}

struct alignas(32) AAA : std::array<int, 36> {};

int main(int, char **) {

  //  printAlignedDataInfo(value, "An integer");
  //
  //  auto &subject = double_array_16_32;
  //  fillConsecutive(subject);
  //  AlignedReference<double, 4, 32> slice(subject, 4);
  //
  //  printAlignedDataInfo(subject, "subject");
  //  printAlignedDataInfo(slice, "reference");
  //  slice.set(slice.data() + 4);
  //  printAlignedDataInfo(slice, "reference + 4");

  printHasOperator<double>();
  printHasOperator<std::vector<int>>();
  printHasOperator<HasOpMul>();

  printValidAlignedAccessBase<org::simple::AlignedVector<int, 32>>();
  printValidAlignedAccessBase<std::vector<int>>();
  printValidAlignedAccessBase<std::array<int, 18>>();
  printValidAlignedAccessBase<AAA>();
  printValidAlignedAccessBase<int>();

  std::cout << typeid(decltype(&std::declval<HasOpMul>().operator*=(
                          std::declval<double>())))
                   .name()
            << std::endl;
  std::cout
      << typeid(std::add_pointer<decltype(&std::declval<HasOpMul>().operator*=(
                    std::declval<double>()))>)
             .name()
      << std::endl;
}
