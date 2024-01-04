//
// Created by michel on 27-03-21.
//

#include <iostream>
#include <org-simple/AlignedData.h>
#include <org-simple/Size.h>

struct Empty {};

using namespace org::simple;

template <typename T, size_t elements, size_t alignment>
using AlignedArray =
    AlignedData<T, elements, alignment, AlignedDataType::ARRAY, false>;
template <typename T, size_t elements, size_t alignment>
using AlignedPointer = AlignedData<T, elements, alignment,
                                   AlignedDataType::ALLOCATED_ARRAY, false>;
template <typename T, size_t elements, size_t alignment>
using AlignedReference =
    AlignedData<T, elements, alignment, AlignedDataType::REFERENCE, false>;

template <class type> void fillConsecutive(type &value) {
  int i = 0;
  for (auto &v : value.getInternalStorage()) {
    v = i++;
  }
}

std::ostream &operator<<(std::ostream &out, const AlignedDataInfo &info) {
  if (!info.isAlignedData) {
    out << "Not AlignedData!";
    return out;
  }
  out << "AlignedDataInfo<";
  out << "elements=" << info.elements;
  out << ",alignment=" << info.alignment;
  out << ",type=";
  switch (info.type) {
  case org::simple::AlignedDataType::ARRAY:
    out << "ARRAY";
    break;
  case org::simple::AlignedDataType::ALLOCATED_ARRAY:
    out << "ALLOCATED_ARRAY";
    break;
  case org::simple::AlignedDataType::REFERENCE:
    out << "REFERENCE";
    break;
  default:
    out << "NONE";
  }
  out << ",isConst=" << (info.isConst ? "true" : "false") << ">";
  return out;
}

template <class T>
void printAlignedDataInfo(const T &data, const char *message) {
  constexpr auto info = AlignedDataInfo::get<T>();
  std::cout << std::endl
            << message << " (" << typeid(T).name() << ")" << std::endl;
  std::cout << "\t" << info << std::endl;
  if constexpr (info.isAlignedData) {
    std::cout << "\tData location " << data.data() << std::endl;
    std::cout << "\t" << message << ":" << std::endl << "\t\t{";
    for (auto &v : data) {
      std::cout << " " << v;
    }
    std::cout << " }" << std::endl;
  }
}

AlignedArray<double, 3, 32> double_array_3_32;
AlignedArray<double, 13, 32> double_array_13_32;
AlignedArray<double, 16, 32> double_array_16_32;
AlignedArray<double, 17, 32> double_array_17_32;

AlignedPointer<double, 3, 32> double_pointer_3_32;
AlignedPointer<double, 13, 32> double_pointer_13_32;
AlignedPointer<double, 16, 32> double_pointer_16_32;
AlignedPointer<double, 17, 32> double_pointer_17_32;

int main(int, char **) {

  int value = 3;

  printAlignedDataInfo(value, "An integer");

  auto &subject = double_array_16_32;
  fillConsecutive(subject);
  AlignedReference<double, 4, 32> slice(subject, 4);

  printAlignedDataInfo(subject, "subject");
  printAlignedDataInfo(slice, "reference");
  slice.set(slice.data() + 4);
  printAlignedDataInfo(slice, "reference + 4");
}
