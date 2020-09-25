//
// Created by michel on 21-09-20.
//

#include "test-helper.h"
#include <org-simple/core/Power2.h>

namespace {

constexpr size_t maximumSize = std::numeric_limits<size_t>::max();

/**
 * Defines a wrapper for power of two-related implementations.
 */
struct PowerOfTwoImplementation {

  org_nodiscard virtual const char *name() const = 0;

  org_nodiscard virtual size_t nextOrSame(size_t size) const = 0;

  org_nodiscard virtual bool is(size_t size) const = 0;

  org_nodiscard virtual bool minusOne(size_t size) const = 0;

  org_nodiscard virtual size_t alignedWith(size_t value,
                                           size_t power) const = 0;
  org_nodiscard virtual bool isAlignedWith(size_t value,
                                           size_t power) const = 0;

  virtual ~PowerOfTwoImplementation() = default;
};

struct SubjectImpl : public PowerOfTwoImplementation {
  org_nodiscard const char *name() const override { return "Power2"; }
  org_nodiscard size_t nextOrSame(size_t size) const override {
    return org::simple::core::Power2::same_or_bigger(size);
  }
  org_nodiscard bool is(size_t size) const override {
    return org::simple::core::Power2::is(size);
  }
  org_nodiscard bool minusOne(size_t size) const override {
    return org::simple::core::Power2::is_minus_one(size);
  }
  org_nodiscard size_t alignedWith(size_t value, size_t power) const override {
    return org::simple::core::Power2::get_aligned_with(value, power);
  };
  org_nodiscard bool isAlignedWith(size_t value, size_t power) const override {
    return org::simple::core::Power2::is_aligned_with(value, power);
  }
};

struct ReferenceImpl : public PowerOfTwoImplementation {

  org_nodiscard const char *name() const override { return "Reference"; }

  org_nodiscard size_t nextOrSame(size_t size) const override {
    if (size <= 2) {
      return 2;
    }

    for (size_t test = 2; test > 0; test *= 2) {
      if (test >= size) {
        return test;
      }
    }

    return 0;
  }

  org_nodiscard bool is(size_t size) const override {
    if (size < 2) {
      return false;
    }
    for (size_t test = 2; test > 0; test *= 2) {
      if (test == size) {
        return true;
      }
    }
    return size == 1 + (maximumSize / 2);
  }

  org_nodiscard bool minusOne(size_t size) const override {
    switch (size) {
    case maximumSize:
    case 1:
      return true;
    case 0:
      return false;
    default:
      break;
    }
    for (size_t test = 2; test > 0; test *= 2) {
      if (test > size) {
        return size == test - 1;
      }
    }
    return false;
  }

  org_nodiscard size_t alignedWith(size_t value, size_t power) const override {
    if (!is(power)) {
      return 0;
    }
    if (value == 0) {
      return 0;
    }
    return power * ((value + power - 1) / power);
  }

  org_nodiscard bool isAlignedWith(size_t value, size_t power) const override {
    return value == alignedWith(value, power);
  }

} referenceImplementation;

template <typename T, typename A, class P>
using AbstractPower2TestCase =
    org::simple::test::CompareWithReferenceTestCase<T, A, P>;

template <typename T, typename A>
class Power2TestCase
    : public AbstractPower2TestCase<T, A, PowerOfTwoImplementation> {
  const std::string name;

public:
  Power2TestCase(const PowerOfTwoImplementation &subject, A arg)
      : AbstractPower2TestCase<T, A, PowerOfTwoImplementation>(
            referenceImplementation, subject, arg),
        name(subject.name()) {}

  Power2TestCase(const PowerOfTwoImplementation &subject, A arg1, A arg2)
      : AbstractPower2TestCase<T, A, PowerOfTwoImplementation>(
            referenceImplementation, subject, arg1, arg2),
        name(subject.name()) {}

  org_nodiscard const char *typeOfTestName() const override {
    return name.c_str();
  }
};

struct IsPowerTestCase : public Power2TestCase<bool, size_t> {

  IsPowerTestCase(const size_t value, const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, value) {}

  org_nodiscard const char *methodName() const override { return "is"; }

  org_nodiscard bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.is(getArgument(0));
  }
};

struct NextPowerTestCase : public Power2TestCase<size_t, size_t> {

  NextPowerTestCase(const size_t value, const PowerOfTwoImplementation &subject)
      : Power2TestCase<size_t, size_t>(subject, value) {}

  org_nodiscard const char *methodName() const override {
    return "same_or_bigger";
  }

  org_nodiscard size_t
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.nextOrSame(getArgument(0));
  }
};

struct IsMinusOneTestCase : public Power2TestCase<bool, size_t> {

  IsMinusOneTestCase(const size_t value,
                     const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, value) {}

  org_nodiscard const char *methodName() const override {
    return "is_minus_one";
  }

  org_nodiscard bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.minusOne(getArgument(0));
  }
};

struct AlignedWithTestCase : public Power2TestCase<size_t, size_t> {

  AlignedWithTestCase(const size_t offset, size_t power,
                      const PowerOfTwoImplementation &subject)
      : Power2TestCase<size_t, size_t>(subject, offset, power) {}

  org_nodiscard const char *methodName() const override {
    return "is_aligned_with";
  }

  org_nodiscard size_t
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.alignedWith(getArgument(0), getArgument(1));
  }

  org_nodiscard const char *getArgumentName(size_t i) const override {
    return i == 0 ? "offset" : i == 1 ? "powerOfTwoValue" : nullptr;
  }
};

struct IsAlignedWithTestCase : public Power2TestCase<bool, size_t> {

  IsAlignedWithTestCase(const size_t offset, size_t power,
                        const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, offset, power) {}

  org_nodiscard const char *methodName() const override {
    return "is_aligned_with";
  }

  org_nodiscard bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.isAlignedWith(getArgument(0), getArgument(1));
  }

  org_nodiscard const char *getArgumentName(size_t i) const override {
    return i == 0 ? "offset" : i == 1 ? "powerOfTwoValue" : nullptr;
  }
};

SubjectImpl constant;

struct TestSet {
  using TestCase = org::simple::test::AbstractValueTestCase;

  std::vector<const TestCase *> getTestCases() { return testCases; }

  TestSet() {
    // Corner cases
    testCases.emplace_back(new IsPowerTestCase(0, constant));
    testCases.emplace_back(new IsPowerTestCase(1, constant));
    testCases.emplace_back(new IsMinusOneTestCase(1, constant));
    testCases.emplace_back(new IsPowerTestCase(maximumSize, constant));
    testCases.emplace_back(new IsMinusOneTestCase(maximumSize, constant));

    // Generate unique values for testing values surrounding powers of two
    std::vector<size_t> powerTestValues;
    for (size_t i = 2, j = 1; i > j; j = i, i *= 2) {
      addIfAbsent(powerTestValues, j - 1);
      addIfAbsent(powerTestValues, j);
      addIfAbsent(powerTestValues, j + 1);
    }
    addIfAbsent(powerTestValues, std::numeric_limits<size_t>::max() - 1);
    addIfAbsent(powerTestValues, std::numeric_limits<size_t>::max());
    // Test these values
    for (size_t value : powerTestValues) {
      testCases.emplace_back(new IsPowerTestCase(value, constant));
      testCases.emplace_back(new IsMinusOneTestCase(value, constant));
      testCases.emplace_back(new NextPowerTestCase(value, constant));
    }
    // Alignment test cases
    for (size_t offset : powerTestValues) {
      for (size_t powerOfTwo : powerTestValues) {
        if (offset < 10000000 && powerOfTwo < 128 &&
            referenceImplementation.is(powerOfTwo)) {
          testCases.emplace_back(
              new AlignedWithTestCase(offset, powerOfTwo, constant));
          testCases.emplace_back(
              new IsAlignedWithTestCase(offset, powerOfTwo, constant));
        }
      }
    }
  }

  ~TestSet() {
    for (const TestCase *testCase : testCases) {
      delete testCase;
    }
  }

private:
  std::vector<const TestCase *> testCases;

  static void addIfAbsent(std::vector<size_t> &values, size_t value) {
    if (!contains(values, value)) {
      values.push_back(value);
    }
  }

  static bool contains(const std::vector<size_t> &haystack, size_t needle) {
    for (size_t value : haystack) {
      if (value == needle) {
        return true;
      }
    }
    return false;
  }
} TEST_SET;
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(org_simple_power2)

BOOST_DATA_TEST_CASE(Power2Scenarios, TEST_SET.getTestCases()) {
  sample->test();
}

BOOST_AUTO_TEST_SUITE_END()
