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

  [[nodiscard]] virtual const char *name() const = 0;

  [[nodiscard]] virtual size_t nextOrSame(size_t size) const = 0;

  [[nodiscard]] virtual bool is(size_t size) const = 0;

  [[nodiscard]] virtual bool minusOne(size_t size) const = 0;

  virtual ~PowerOfTwoImplementation() = default;
};

struct SubjectImpl : public PowerOfTwoImplementation {
  [[nodiscard]] const char *name() const override { return "Power2"; }
  [[nodiscard]] size_t nextOrSame(size_t size) const override {
    constexpr size_t max = std::bit_floor(std::numeric_limits<size_t>::max());
    return size <= max ? std::bit_ceil(size) : 0;
  }
  [[nodiscard]] bool is(size_t size) const override {
    return std::has_single_bit(size);
  }
  [[nodiscard]] bool minusOne(size_t size) const override {
    return size && std::has_single_bit(size ^ (std::bit_floor(size) - 1));
  }
};

struct ReferenceImpl : public PowerOfTwoImplementation {

  [[nodiscard]] const char *name() const override { return "Reference"; }

  [[nodiscard]] size_t nextOrSame(size_t size) const override {
    if (size <= 1) {
      return 1;
    }

    for (size_t test = 1; test > 0; test *= 2) {
      if (test >= size) {
        return test;
      }
    }

    return 0;
  }

  [[nodiscard]] bool is(size_t size) const override {
    if (size < 1) {
      return false;
    }
    for (size_t test = 1; test > 0; test *= 2) {
      if (test == size) {
        return true;
      }
    }
    return size == 1 + (maximumSize / 2);
  }

  [[nodiscard]] bool minusOne(size_t size) const override {
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

  [[nodiscard]] const char *typeOfTestName() const override {
    return name.c_str();
  }
};

struct IsPowerTestCase : public Power2TestCase<bool, size_t> {

  IsPowerTestCase(const size_t value, const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, value) {}

  [[nodiscard]] const char *methodName() const override { return "is"; }

  [[nodiscard]] bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.is(getArgument(0));
  }
};

struct NextPowerTestCase : public Power2TestCase<size_t, size_t> {

  NextPowerTestCase(const size_t value, const PowerOfTwoImplementation &subject)
      : Power2TestCase<size_t, size_t>(subject, value) {}

  [[nodiscard]] const char *methodName() const override {
    return "same_or_bigger";
  }

  [[nodiscard]] size_t
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.nextOrSame(getArgument(0));
  }
};

struct IsMinusOneTestCase : public Power2TestCase<bool, size_t> {

  IsMinusOneTestCase(const size_t value,
                     const PowerOfTwoImplementation &subject)
      : Power2TestCase<bool, size_t>(subject, value) {}

  [[nodiscard]] const char *methodName() const override {
    return "is_minus_one";
  }

  [[nodiscard]] bool
  generateValue(const PowerOfTwoImplementation &impl) const override {
    return impl.minusOne(getArgument(0));
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
