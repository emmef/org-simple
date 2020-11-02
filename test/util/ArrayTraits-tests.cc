//
// Created by michel on 02-11-20.
//

#include <cstdint>
#include <typeinfo>
#include "test-helper.h"
#include <org-simple/util/ArrayTraits.h>

using namespace org::simple::array_traits;

struct TypeValueNotPresent {};
struct TypeValueIsMemberVariable {
  bool value_type;
};
struct TypeValueIsMemberConstant {
  static constexpr bool value_type = true;
};
struct TypeValueIsMemberFunction {
  bool value_type();
};
struct TypeValueIsConstantFunction{
  static constexpr bool value_type() { return true; }
};
struct TypeValueIsTypedef {
  typedef int64_t value_type;
};

struct LocalDataNotPresent {};
struct LocalDataIsMemberVariable {
  bool has_local_data;
};
struct LocalDataIsMemberFunction {
  bool has_local_data();
};
struct LocalDataIsMemberConstantTrue {
  static constexpr bool has_local_data = true;
};
struct LocalDataIsConstantFunctionTrue {
  static constexpr bool has_local_data() { return true; }
};
struct LocalDataIsMemberConstantFalse {
  static constexpr bool has_local_data = false;
};

struct ConstSizeNotpresent {};
struct ConstSizeVariable {
  size_t size;
};
struct ConstSizeConstNoFunction {
  static constexpr size_t size = 3;
};
struct ConstSizeFunctionNotConst {
  size_t size() const { return 3; };
};
struct ConstSizeFunctionConstNonZero {
  static constexpr size_t size() { return 3; };
};
struct ConstSizeFunctionConstDifferentIntegralType {
  static constexpr char size() { return 3; };
};
struct ConstSizeFunctionConstZero {
  static constexpr size_t size() { return 0; };
};

BOOST_AUTO_TEST_SUITE(org_simple_util_array_traits)

BOOST_AUTO_TEST_CASE(testHasValueType) {
  BOOST_CHECK(!(WithValueType<TypeValueNotPresent>::value));
  BOOST_CHECK(!(WithValueType<TypeValueIsMemberVariable>::value));
  BOOST_CHECK(!(WithValueType<TypeValueIsMemberFunction>::value));
  BOOST_CHECK(!(WithValueType<TypeValueIsMemberConstant>::value));
  BOOST_CHECK(!(WithValueType<TypeValueIsConstantFunction>::value));
  BOOST_CHECK((WithValueType<TypeValueIsTypedef>::value));
}

BOOST_AUTO_TEST_CASE(testHasLocalData) {
  BOOST_CHECK(!(HasLocalData<LocalDataNotPresent>::value));
  BOOST_CHECK(!(HasLocalData<LocalDataIsMemberVariable>::value));
  BOOST_CHECK(!(HasLocalData<LocalDataIsMemberFunction>::value));
  BOOST_CHECK(!(HasLocalData<LocalDataIsMemberConstantFalse>::value));
  BOOST_CHECK(!(HasLocalData<LocalDataIsConstantFunctionTrue>::value));
  BOOST_CHECK((HasLocalData<LocalDataIsMemberConstantTrue>::value));
}

BOOST_AUTO_TEST_CASE(testHasConstantSize) {
  BOOST_CHECK(!(HasSizeFunction<ConstSizeNotpresent>::value));
  BOOST_CHECK(!(HasSizeFunction<ConstSizeConstNoFunction>::value));
  BOOST_CHECK(!(HasSizeFunction<ConstSizeVariable>::value));
  BOOST_CHECK((HasSizeFunction<ConstSizeFunctionNotConst>::value));
  BOOST_CHECK((HasSizeFunction<ConstSizeFunctionConstZero>::value));
  BOOST_CHECK((HasSizeFunction<ConstSizeFunctionConstNonZero>::value));

  BOOST_CHECK(!(HasNonZeroConstSizeFunction<ConstSizeNotpresent>::value));
  BOOST_CHECK(!(HasNonZeroConstSizeFunction<ConstSizeConstNoFunction>::value));
  BOOST_CHECK(!(HasNonZeroConstSizeFunction<ConstSizeVariable>::value));
  BOOST_CHECK(!(HasNonZeroConstSizeFunction<ConstSizeFunctionNotConst>::value));
  BOOST_CHECK(!(HasNonZeroConstSizeFunction<ConstSizeFunctionConstZero>::value));
  BOOST_CHECK((HasNonZeroConstSizeFunction<ConstSizeFunctionConstNonZero>::value));

  BOOST_CHECK_EQUAL(0,(AboutSizeFunction<ConstSizeNotpresent>::value));
  BOOST_CHECK_EQUAL(0, (AboutSizeFunction<ConstSizeConstNoFunction>::value));
  BOOST_CHECK_EQUAL(0, (AboutSizeFunction<ConstSizeVariable>::value));
  BOOST_CHECK_EQUAL(0, (AboutSizeFunction<ConstSizeFunctionNotConst>::value));
  BOOST_CHECK_EQUAL(0, (AboutSizeFunction<ConstSizeFunctionConstZero>::value));
  BOOST_CHECK_EQUAL(0, (AboutSizeFunction<ConstSizeFunctionConstDifferentIntegralType>::value));
  BOOST_CHECK_EQUAL(3, (AboutSizeFunction<ConstSizeFunctionConstNonZero>::value));

  BOOST_CHECK(!(AboutSizeFunction<ConstSizeNotpresent>::has));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeConstNoFunction>::has));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeVariable>::has));
  BOOST_CHECK((AboutSizeFunction<ConstSizeFunctionNotConst>::has));
  BOOST_CHECK((AboutSizeFunction<ConstSizeFunctionConstZero>::has));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeFunctionConstDifferentIntegralType>::has));
  BOOST_CHECK((AboutSizeFunction<ConstSizeFunctionConstNonZero>::has));

  BOOST_CHECK(!(AboutSizeFunction<ConstSizeNotpresent>::has_static));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeConstNoFunction>::has_static));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeVariable>::has_static));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeFunctionNotConst>::has_static));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeFunctionConstZero>::has_static));
  BOOST_CHECK(!(AboutSizeFunction<ConstSizeFunctionConstDifferentIntegralType>::has_static));
  BOOST_CHECK((AboutSizeFunction<ConstSizeFunctionConstNonZero>::has_static));

//  ConstSizeFunction
}

BOOST_AUTO_TEST_SUITE_END()
