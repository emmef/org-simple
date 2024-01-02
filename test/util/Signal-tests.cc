//
// Created by michel on 11-07-21.
//

#include <org-simple/Signal.h>
#include <ostream>
#include <test-helper.h>
#include <vector>

using Sig = org::simple::Signal;
using Type = org::simple::SignalType;
using value_type = typename Sig::value_type;
using wrapped_type = typename Sig::wrap_type;

static const std::vector<wrapped_type> wrap_test_values() {
  std::vector<wrapped_type> result;
    result.push_back(1u);
    result.push_back(2u);
    result.push_back(Sig::maxValue / 4);
    result.push_back(Sig::maxValue / 3);
    result.push_back(Sig::maxValue / 2);
    result.push_back(Sig::maxValue - 1);
    result.push_back(Sig::maxValue);

  return result;
}

BOOST_AUTO_TEST_SUITE(org_simple_util_Signals)

BOOST_AUTO_TEST_CASE(initNone) {
  Sig sig;

  BOOST_CHECK_EQUAL(0, sig.value());
  BOOST_CHECK(Type::NONE ==sig.type());
}

BOOST_AUTO_TEST_CASE(testNoneWRapsToZero) {
  Sig none;

  BOOST_CHECK_EQUAL(none.wrapped(), 0u);
}

BOOST_AUTO_TEST_CASE(initUser) {
  wrapped_type value = Sig::maxValue / 2;
  Sig sig = Sig::user(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::USER == sig.type());
}

BOOST_AUTO_TEST_CASE(initProgram) {
  wrapped_type value = Sig::maxValue / 2;
  Sig sig = Sig::program(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::PROGRAM == sig.type());
}

BOOST_AUTO_TEST_CASE(initSystem) {
  wrapped_type value = Sig::maxValue / 2;
  Sig sig = Sig::system(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::SYSTEM == sig.type());
}

BOOST_AUTO_TEST_CASE(initSystemWithZero) {
  BOOST_CHECK_THROW(Sig::system(0), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(initSystemWithMax) {
  Sig sig = Sig::system(Sig::maxValue);

  BOOST_CHECK_EQUAL(Sig::maxValue, sig.value());
}

BOOST_AUTO_TEST_CASE(initProgramWithZero) {
  BOOST_CHECK_THROW(Sig::program(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initProgramWithMax) {
  Sig sig = Sig::program(Sig::maxValue);

  BOOST_CHECK_EQUAL(Sig::maxValue, sig.value());
}

BOOST_AUTO_TEST_CASE(initUserWithZero) {
  BOOST_CHECK_THROW(Sig::user(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initUserWithMax) {
  Sig sig = Sig::user(Sig::maxValue);

  BOOST_CHECK_EQUAL(Sig::maxValue, sig.value());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorNone) {
  Sig sig;

  BOOST_CHECK(!sig.terminates());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorSystem) {
  Sig sig = Sig::system(Sig::maxValue / 2);

  BOOST_CHECK(sig.terminates());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorProgram) {
  Sig sig = Sig::program(Sig::maxValue / 2);

  BOOST_CHECK(sig.terminates());
}

BOOST_AUTO_TEST_CASE(checkTerminatorUser) {
  Sig sig = Sig::user(Sig::maxValue / 2);

  BOOST_CHECK(sig.terminates());
}

BOOST_AUTO_TEST_CASE(wrapUnwrapNone) {
  Sig sig;
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
  BOOST_CHECK(unwrapped == sig);
}

BOOST_AUTO_TEST_CASE(wrapUnwrapNoneSetWrappedValue) {
  Sig sig;
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
  BOOST_CHECK(unwrapped == sig);
}

BOOST_DATA_TEST_CASE(testSystemWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::system(sample);
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
  BOOST_CHECK(unwrapped == sig);
}

BOOST_DATA_TEST_CASE(testProgramWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::program(sample);
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
  BOOST_CHECK(unwrapped == sig);
  BOOST_CHECK(unwrapped == sig);
}

BOOST_DATA_TEST_CASE(testUserWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::user(sample);
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
  BOOST_CHECK(unwrapped == sig);
}

BOOST_DATA_TEST_CASE(testSystemWrapAndUnwrapNoTerminates, wrap_test_values()) {
  Sig sig = Sig::system(sample, false);
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
  BOOST_CHECK(unwrapped == sig);
}

BOOST_DATA_TEST_CASE(testProgramWrapAndUnwrapNoTerminates, wrap_test_values()) {
  Sig sig = Sig::program(sample, false);
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
  BOOST_CHECK(unwrapped == sig);
  BOOST_CHECK(unwrapped == sig);
}

BOOST_DATA_TEST_CASE(testUserWrapAndUnwrapTerminates, wrap_test_values()) {
  Sig sig = Sig::user(sample, true);
  wrapped_type wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
  BOOST_CHECK(unwrapped == sig);
}

BOOST_AUTO_TEST_SUITE_END()
