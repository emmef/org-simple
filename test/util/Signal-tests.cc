//
// Created by michel on 11-07-21.
//

#include <org-simple/util/Signal.h>
#include <ostream>
#include <test-helper.h>
#include <vector>

using Sig = org::simple::util::Signal;
using Type = org::simple::util::SignalType;
using value_type = typename Sig::value_type;
using ext_type = typename Sig::external_type ;

static const std::vector<ext_type> wrap_test_values() {
  std::vector<ext_type> result;
    result.push_back(1u);
    result.push_back(2u);
    result.push_back(Sig::MAX_VALUE / 4);
    result.push_back(Sig::MAX_VALUE / 3);
    result.push_back(Sig::MAX_VALUE / 2);
    result.push_back(Sig::MAX_VALUE - 1);
    result.push_back(Sig::MAX_VALUE);

  return result;
}

BOOST_AUTO_TEST_SUITE(org_simple_util_Signals)

    BOOST_AUTO_TEST_CASE(initNone) {
  Sig sig;

  BOOST_CHECK_EQUAL(0, sig.value());
  BOOST_CHECK(Type::NONE ==sig.type());
}

BOOST_AUTO_TEST_CASE(initUser) {
  ext_type value = Sig::MAX_VALUE / 2;
  Sig sig = Sig::user(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::USER == sig.type());
}

BOOST_AUTO_TEST_CASE(initProgram) {
  ext_type value = Sig::MAX_VALUE / 2;
  Sig sig = Sig::program(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::PROGRAM == sig.type());
}

BOOST_AUTO_TEST_CASE(initSystem) {
  ext_type value = Sig::MAX_VALUE / 2;
  Sig sig = Sig::system(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::SYSTEM == sig.type());
}

BOOST_AUTO_TEST_CASE(initSystemWithZero) {
  BOOST_CHECK_THROW(Sig::system(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initSystemWithMaxPLusOne) {
  BOOST_CHECK_THROW(Sig::system(Sig::MAX_VALUE + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initSystemWithMax) {
  Sig sig = Sig::system(Sig::MAX_VALUE);

  BOOST_CHECK_EQUAL(Sig::MAX_VALUE, sig.value());
}

BOOST_AUTO_TEST_CASE(initProgramWithZero) {
  BOOST_CHECK_THROW(Sig::program(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initProgramWithMaxPLusOne) {
  BOOST_CHECK_THROW(Sig::program(Sig::MAX_VALUE + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initProgramWithMax) {
  Sig sig = Sig::program(Sig::MAX_VALUE);

  BOOST_CHECK_EQUAL(Sig::MAX_VALUE, sig.value());
}

BOOST_AUTO_TEST_CASE(initUserWithZero) {
  BOOST_CHECK_THROW(Sig::user(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initUserWithMaxPLusOne) {
  BOOST_CHECK_THROW(Sig::user(Sig::MAX_VALUE + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initUserWithMax) {
  Sig sig = Sig::user(Sig::MAX_VALUE);

  BOOST_CHECK_EQUAL(Sig::MAX_VALUE, sig.value());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorNone) {
  Sig sig;

  BOOST_CHECK(!sig.terminates());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorSystem) {
  Sig sig = Sig::system(Sig::MAX_VALUE / 2);

  BOOST_CHECK(sig.terminates());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorProgram) {
  Sig sig = Sig::program(Sig::MAX_VALUE / 2);

  BOOST_CHECK(sig.terminates());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorUser) {
  Sig sig = Sig::user(Sig::MAX_VALUE / 2);

  BOOST_CHECK(!sig.terminates());
}

BOOST_AUTO_TEST_CASE(wrapUnwrapNone) {
  Sig sig;
  ext_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
}

BOOST_AUTO_TEST_CASE(wrapUnwrapNoneSetWrappedValue) {
  Sig sig;
  ext_type wrapped = sig.wrapped();
  wrapped |= Sig::MAX_VALUE / 2;
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
}

BOOST_DATA_TEST_CASE(testSystemWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::system(sample);
  ext_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
}

BOOST_DATA_TEST_CASE(testProgramWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::program(sample);
  ext_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
}

BOOST_DATA_TEST_CASE(testUserWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::user(sample);
  ext_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
}

BOOST_AUTO_TEST_SUITE_END()
