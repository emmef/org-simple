//
// Created by michel on 23-09-20.
//

#include <org-simple/core/Bits.h>
#include "test-helper.h"

namespace {

using FunctionTest = org::simple::test::FunctionTestScenario;

struct Scenarios {

  static FunctionTest most_significant_bit(size_t value, int expected) {

    return FunctionTest::create(
        value, expected, org::simple::core::Bits<size_t>::most_significant,
        "Bits::most_significant");
  }

  static FunctionTest most_significant_single_bit(size_t value, int expected) {
    return FunctionTest::create(
        value, expected,
        org::simple::core::Bits<size_t>::most_significant_single,
        "Bits::most_significant_single");
  }

  static FunctionTest bit_fill(size_t value, size_t expected) {
    return FunctionTest::create(
        value, expected, org::simple::core::Bits<size_t>::fill, "Bits::fill");
  }
};

class BitsTestCases {
  std::vector<FunctionTest> testCases;

public:
  BitsTestCases() {

    testCases.push_back(Scenarios::most_significant_bit(0, -1));
    testCases.push_back(Scenarios::most_significant_single_bit(0, -1));

    testCases.push_back(Scenarios::most_significant_bit(1, 0));
    testCases.push_back(Scenarios::most_significant_single_bit(1, 0));

    testCases.push_back(Scenarios::most_significant_bit(2, 1));
    testCases.push_back(Scenarios::most_significant_single_bit(2, 1));

    testCases.push_back(Scenarios::most_significant_bit(4, 2));
    testCases.push_back(Scenarios::most_significant_single_bit(4, 2));

    testCases.push_back(Scenarios::most_significant_bit(0x10, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x10, 4));

    testCases.push_back(Scenarios::most_significant_bit(0x11, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x11, -1));

    testCases.push_back(Scenarios::most_significant_bit(0x12, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x12, -2));

    testCases.push_back(Scenarios::most_significant_bit(0x13, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x13, -2));

    testCases.push_back(Scenarios::most_significant_bit(0x14, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x14, -3));

    testCases.push_back(Scenarios::most_significant_bit(0x18, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x18, -4));

    testCases.push_back(Scenarios::most_significant_bit(0x1C, 4));
    testCases.push_back(Scenarios::most_significant_single_bit(0x1C, -4));

    int max_bit = sizeof(size_t) * 8 - 1;
    size_t max = size_t(1) << size_t(max_bit);

    testCases.push_back(Scenarios::most_significant_bit(max, max_bit));
    testCases.push_back(Scenarios::most_significant_single_bit(max, max_bit));

    testCases.push_back(Scenarios::most_significant_bit(max + 1, max_bit));
    testCases.push_back(Scenarios::most_significant_single_bit(max + 1, -1));

    testCases.push_back(Scenarios::bit_fill(0x01, 0x01));
    testCases.push_back(Scenarios::bit_fill(0x02, 0x03));
    testCases.push_back(Scenarios::bit_fill(0x35, 0x3f));
    testCases.push_back(Scenarios::bit_fill(0x37, 0x3f));
    testCases.push_back(Scenarios::bit_fill(0x47, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x57, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x67, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x77, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x78, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x79, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x7C, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x7f, 0x7f));
    testCases.push_back(Scenarios::bit_fill(0x80, 0xff));
    testCases.push_back(Scenarios::bit_fill(0x8000000, 0xfffffff));
    testCases.push_back(Scenarios::bit_fill(0x8070100, 0xfffffff));
  }

  const std::vector<FunctionTest> getTestCases() { return testCases; }

} TESTCASES;

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_bits)

BOOST_DATA_TEST_CASE(msbScenarios, TESTCASES.getTestCases()) {
  BOOST_CHECK(sample.success());
}

BOOST_AUTO_TEST_SUITE_END()
