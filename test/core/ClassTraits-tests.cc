//
// Created by michel on 25-10-20.
//

#include "test-helper.h"
#include <org-simple/core/ClassTraits.h>

namespace {

struct ClassWithInt16Size {
  int16_t size();
};

struct ClassWithInt64Size {
  int64_t size();
};

struct ClassWithStaticInt16Size {
  static int16_t size() { return 0; };
};

struct ClassWithConstStaticInt16Size {
  static constexpr int16_t size() { return 0; };
};

struct ClassWithStaticInt64Size {
  int64_t size();
};

struct ClassWithoutSize {};

ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK(array, size);
ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK(array, size);
ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION(array, size);
ORG_SIMPLE_ABOUT_CLASS_FUNCTION(array, size);

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_org_ClassTraits)

BOOST_AUTO_TEST_CASE(testClassWithInt16SizeHasInt16SizeGenerated) {
  BOOST_CHECK((array_has_size<ClassWithInt16Size, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(
    testClassWithInt64SizeHasNoInt16SizeGeneratedAndViceVersa) {
  BOOST_CHECK((!array_has_size<ClassWithInt16Size, int64_t>::value));
  BOOST_CHECK((!array_has_size<ClassWithInt64Size, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithoutSizeHasNoInt16SizeGenerated) {
  BOOST_CHECK((!array_has_size<ClassWithoutSize, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithInt64SizeHasStaticNoInt16SizeGenerated) {
  BOOST_CHECK((!array_has_static_size<ClassWithInt64Size, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithStaticInt16SizeHasStaticInt16SizeGenerated) {
  BOOST_CHECK(
      (array_has_static_size<ClassWithStaticInt16Size, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithStaticInt16SizeHasInt16SizeGenerated) {
  BOOST_CHECK(
      (array_has_size<ClassWithStaticInt16Size, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithInt16SizeHasNoStaticInt16SizeGenerated) {
  BOOST_CHECK((!array_has_static_size<ClassWithInt16Size, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(
    testClassWithStaticInt64SizeHasStaticNoInt16SizeGeneratedAndViceVersa) {
  BOOST_CHECK(
      (!array_has_static_size<ClassWithStaticInt64Size, int16_t>::value));
  BOOST_CHECK(
      (!array_has_static_size<ClassWithStaticInt16Size, int64_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithoutSizeHasStaticNoInt16SizeGenerated) {
  BOOST_CHECK((!array_has_static_size<ClassWithoutSize, int16_t>::value));
}

BOOST_AUTO_TEST_CASE(testClassWithStaticInt16SizeHasNoConstStaticInt16Size) {
  typedef about_array_static_function_size<ClassWithStaticInt16Size, int16_t> check;
  BOOST_CHECK((check::exists));
  BOOST_CHECK((!check::is_constexpr));
}

BOOST_AUTO_TEST_CASE(testAboutClassFunctionWithClassWithNoSizeFunction) {
  typedef about_array_function_size<ClassWithoutSize, int16_t> check;
  BOOST_CHECK((!check::exists));
  BOOST_CHECK((!check::is_static));
  BOOST_CHECK((!check::is_constexpr));
}

BOOST_AUTO_TEST_CASE(testAboutClassFunctionWithClassWithMemberSizeFunction) {
  typedef about_array_function_size<ClassWithInt16Size, int16_t> check;
  BOOST_CHECK((check::exists));
  BOOST_CHECK((!check::is_static));
  BOOST_CHECK((!check::is_constexpr));
}

BOOST_AUTO_TEST_CASE(testAboutClassFunctionWithClassWithStaticSizeFunction) {
  typedef about_array_function_size<ClassWithStaticInt16Size, int16_t> check;
  BOOST_CHECK((check::exists));
  BOOST_CHECK((check::is_static));
  BOOST_CHECK((!check::is_constexpr));
}

BOOST_AUTO_TEST_CASE(testAboutClassFunctionWithClassWithStaticCOnstSizeFunction) {
  typedef about_array_function_size<ClassWithConstStaticInt16Size, int16_t> check;
  BOOST_CHECK((check::exists));
  BOOST_CHECK((check::is_static));
  BOOST_CHECK((check::is_constexpr));
}

BOOST_AUTO_TEST_SUITE_END()
