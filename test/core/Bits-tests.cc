//
// Created by michel on 23-09-20.
//

#include "test-helper.h"
#include <org-simple/core/Bits.h>

namespace {

using FunctionTest = org::simple::test::FunctionTestScenario;

template <typename T>
struct Scenarios {

  static FunctionTest most_significant_bit(T value, int expected) {

    return FunctionTest::create(value, expected,
                                org::simple::core::Bits<T>::most_significant,
                                "Bits::most_significant");
  }

  static FunctionTest leading_zero_bits(T value, int expected) {

    return FunctionTest::create(
        value, expected, org::simple::core::Bits<T>::number_of_leading_zeroes,
        "Bits::leading_zero_bits");
  }

  static FunctionTest most_significant_single_bit(T value, int expected) {
    return FunctionTest::create(
        value, expected, org::simple::core::Bits<T>::most_significant_single,
        "Bits::most_significant_single");
  }

  static FunctionTest bit_fill(T value, T expected) {
    return FunctionTest::create(value, expected,
                                org::simple::core::Bits<T>::fill, "Bits::fill");
  }
};

template <typename T> class BitsTestCases {
  static_assert(org::simple::core::unsignedIntegral<T>);
  static constexpr int bits = sizeof(T) * 8;
  static constexpr int max_bit = sizeof(T) * 8 - 1;
  static constexpr T max = T(1) << max_bit;
  std::vector<FunctionTest> testCases;
  using Scenarios = Scenarios<T>;

public:
  BitsTestCases() {
    testCases.push_back(Scenarios::leading_zero_bits(0x00u, bits));
    testCases.push_back(Scenarios::leading_zero_bits(0x01u, bits - 1));
    testCases.push_back(Scenarios::leading_zero_bits(0x02u, bits - 2));
    testCases.push_back(Scenarios::leading_zero_bits(0x03u, bits - 2));
    testCases.push_back(Scenarios::leading_zero_bits(0x04u, bits - 3));
    testCases.push_back(Scenarios::leading_zero_bits(0x05u, bits - 3));
    testCases.push_back(Scenarios::leading_zero_bits(0x06u, bits - 3));
    testCases.push_back(Scenarios::leading_zero_bits(0x07u, bits - 3));
    testCases.push_back(Scenarios::leading_zero_bits(0x08u, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x09u, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x0au, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x0bu, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x0cu, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x0du, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x0eu, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x0fu, bits - 4));
    testCases.push_back(Scenarios::leading_zero_bits(0x10u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x11u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x12u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x13u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x14u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x15u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x16u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x17u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x18u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x19u, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x1au, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x1bu, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x1cu, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x1du, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x1eu, bits - 5));
    testCases.push_back(Scenarios::leading_zero_bits(0x1fu, bits - 5));

    testCases.push_back(Scenarios::leading_zero_bits(max, 0));
    testCases.push_back(Scenarios::leading_zero_bits(T(max + 1), 0));

    testCases.push_back(Scenarios::most_significant_bit(0u, -1));
    testCases.push_back(Scenarios::most_significant_single_bit(0u, -1));

    testCases.push_back(Scenarios::most_significant_bit(1u, 0));
    testCases.push_back(Scenarios::most_significant_single_bit(1u, 0));

    testCases.push_back(Scenarios::most_significant_bit(2u, 1));
    testCases.push_back(Scenarios::most_significant_single_bit(2u, 1));

    testCases.push_back(Scenarios::most_significant_bit(4u, 2));
    testCases.push_back(Scenarios::most_significant_single_bit(4u, 2));

    testCases.push_back(Scenarios::most_significant_bit(0x10u, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x10u, 4));

    testCases.push_back(Scenarios::most_significant_bit(0x11u, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x11u, -1));

    testCases.push_back(Scenarios::most_significant_bit(0x12u, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x12u, -2));

    testCases.push_back(Scenarios::most_significant_bit(0x13u, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x13u, -2));

    testCases.push_back(Scenarios::most_significant_bit(0x14u, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x14u, -3));

    testCases.push_back(Scenarios::most_significant_bit(0x18u, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x18u, -4));

    testCases.push_back(Scenarios::most_significant_bit(0x1Cu, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x1Cu, -4));

    testCases.push_back(Scenarios::most_significant_bit(max, max_bit));
    testCases.push_back(Scenarios::most_significant_single_bit(max, max_bit));

    testCases.push_back(Scenarios::most_significant_bit(max + 1, max_bit));
    testCases.push_back(Scenarios::most_significant_single_bit(max + 1, -1));

    testCases.push_back(Scenarios::bit_fill(0x01u, 0x01u));
    testCases.push_back(Scenarios::bit_fill(0x02u, 0x03u));
    testCases.push_back(Scenarios::bit_fill(0x35u, 0x3fu));
    testCases.push_back(Scenarios::bit_fill(0x37u, 0x3fu));
    testCases.push_back(Scenarios::bit_fill(0x47u, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x57u, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x67u, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x77u, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x78u, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x79u, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x7Cu, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x7fu, 0x7fu));
    testCases.push_back(Scenarios::bit_fill(0x80u, 0xffu));
//    testCases.push_back(Scenarios::bit_fill(0x8000000u, 0xfffffffu));
//    testCases.push_back(Scenarios::bit_fill(0x8070100u, 0xfffffffu));
  }

  const std::vector<FunctionTest> getTestCases() { return testCases; }
};

static BitsTestCases<uint32_t> TESTCASES_08;
static BitsTestCases<uint16_t> TESTCASES_16;
static BitsTestCases<uint32_t> TESTCASES_32;
static BitsTestCases<uint64_t> TESTCASES_64;

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_bits)

BOOST_DATA_TEST_CASE(msbScenarios_uint8_t, TESTCASES_08.getTestCases()) {
  BOOST_CHECK(sample.success());
}

BOOST_DATA_TEST_CASE(msbScenarios_uint16_t, TESTCASES_16.getTestCases()) {
  BOOST_CHECK(sample.success());
}

BOOST_DATA_TEST_CASE(msbScenarios_uint32_t, TESTCASES_32.getTestCases()) {
  BOOST_CHECK(sample.success());
}

BOOST_DATA_TEST_CASE(msbScenarios_uint64_t, TESTCASES_64.getTestCases()) {
  BOOST_CHECK(sample.success());
}

BOOST_AUTO_TEST_SUITE_END()
