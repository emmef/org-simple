//
// Created by michel on 11-07-21.
//

#include <org-simple/util/Signals.h>

#include <iostream>
#include <test-helper.h>
#include <vector>

using Sig = org::simple::util::Signal;
using Type = org::simple::util::Signal::Type;

static std::ostream &operator<<(std::ostream &out, const Type &type) {
  out << Sig::type_name(type);
  return out;
}

static constexpr unsigned TEST_VALUES[] = {
    1, Sig::max_value() / 4, Sig::max_value() / 3, Sig::max_value() / 2,
    Sig::max_value()};
static constexpr unsigned TEST_VALUES_COUNT =
    sizeof(TEST_VALUES) / sizeof(unsigned);

static const std::vector<unsigned> wrap_test_values() {
//  static std::atomic_flag flag = ATOMIC_FLAG_INIT;
  std::vector<unsigned> result;
//  if (flag.test_and_set()) {
    result.push_back(1u);
    result.push_back(2u);
    result.push_back(Sig::max_value() / 4);
    result.push_back(Sig::max_value() / 3);
    result.push_back(Sig::max_value() / 2);
    result.push_back(Sig::max_value() - 1);
    result.push_back(Sig::max_value());

  return result;
}

BOOST_AUTO_TEST_SUITE(org_simple_util_Signals)

    BOOST_AUTO_TEST_CASE(initNone) {
  Sig sig = Sig::none();

  BOOST_CHECK_EQUAL(0u, sig.value());
  BOOST_CHECK(Type::NONE == sig.type());
}

BOOST_AUTO_TEST_CASE(initTimeout) {
  Sig sig = Sig::timeout();

  BOOST_CHECK_EQUAL(0u, sig.value());
  BOOST_CHECK(Type::TIMEOUT == sig.type());
}

BOOST_AUTO_TEST_CASE(initUser) {
  unsigned value = Sig::max_value() / 2;
  Sig sig = Sig::user(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::USER == sig.type());
}

BOOST_AUTO_TEST_CASE(initProgram) {
  unsigned value = Sig::max_value() / 2;
  Sig sig = Sig::program(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::PROGRAM == sig.type());
}

BOOST_AUTO_TEST_CASE(initSystem) {
  unsigned value = Sig::max_value() / 2;
  Sig sig = Sig::system(value);

  BOOST_CHECK_EQUAL(value, sig.value());
  BOOST_CHECK(Type::SYSTEM == sig.type());
}

BOOST_AUTO_TEST_CASE(initSystemWithZero) {
  BOOST_CHECK_THROW(Sig::system(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initSystemWithMaxPLusOne) {
  BOOST_CHECK_THROW(Sig::system(Sig::max_value() + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initSystemWithMax) {
  Sig sig = Sig::system(Sig::max_value());

  BOOST_CHECK_EQUAL(Sig::max_value(), sig.value());
}

BOOST_AUTO_TEST_CASE(initProgramWithZero) {
  BOOST_CHECK_THROW(Sig::program(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initProgramWithMaxPLusOne) {
  BOOST_CHECK_THROW(Sig::program(Sig::max_value() + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initProgramWithMax) {
  Sig sig = Sig::program(Sig::max_value());

  BOOST_CHECK_EQUAL(Sig::max_value(), sig.value());
}

BOOST_AUTO_TEST_CASE(initUserWithZero) {
  BOOST_CHECK_THROW(Sig::user(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initUserWithMaxPLusOne) {
  BOOST_CHECK_THROW(Sig::user(Sig::max_value() + 1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(initUserWithMax) {
  Sig sig = Sig::user(Sig::max_value());

  BOOST_CHECK_EQUAL(Sig::max_value(), sig.value());
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorNone) {
  Sig sig = Sig::none();

  sig.check_no_terminator();
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorTimeout) {
  Sig sig = Sig::timeout();

  sig.check_no_terminator();
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorSystem) {
  Sig sig = Sig::system(Sig::max_value() / 2);

  BOOST_CHECK_THROW(sig.check_no_terminator(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorProgram) {
  Sig sig = Sig::program(Sig::max_value() / 2);

  BOOST_CHECK_THROW(sig.check_no_terminator(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(checkNoTerminatorUser) {
  Sig sig = Sig::user(Sig::max_value() / 2);

  sig.check_no_terminator();
}

BOOST_AUTO_TEST_CASE(wrapUnwrapNone) {
  Sig sig = Sig::none();
  unsigned wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
}

BOOST_AUTO_TEST_CASE(wrapUnwrapNoneSetWrappedValue) {
  Sig sig = Sig::none();
  unsigned wrapped = sig.wrapped();
  wrapped |= Sig::max_value() / 2;
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
}

BOOST_AUTO_TEST_CASE(wrapUnwrapTimeout) {
  Sig sig = Sig::timeout();
  unsigned wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
}

BOOST_AUTO_TEST_CASE(wrapUnwrapTimoutSetWrappedValue) {
  Sig sig = Sig::timeout();
  unsigned wrapped = sig.wrapped();
  wrapped |= Sig::max_value() / 2;
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK(unwrapped.value() == sig.value());
  BOOST_CHECK(unwrapped.type() == sig.type());
}

BOOST_AUTO_TEST_CASE(unwrapInvalidType) {
  unsigned invalidWrapped;
  if (!Sig::test_invalid_wrapped_type(invalidWrapped)) {
    return;
  }
  BOOST_CHECK_THROW(Sig::unwrap(invalidWrapped), std::invalid_argument);
}

BOOST_DATA_TEST_CASE(testSystemWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::system(sample);
  unsigned wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
}

BOOST_DATA_TEST_CASE(testProgramWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::program(sample);
  unsigned wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
}

BOOST_DATA_TEST_CASE(testUserWrapAndUnwrap, wrap_test_values()) {
  Sig sig = Sig::user(sample);
  unsigned wrapped = sig.wrapped();
  Sig unwrapped = Sig::unwrap(wrapped);

  BOOST_CHECK_EQUAL(sig.value(), unwrapped.value());
}

BOOST_AUTO_TEST_SUITE_END()
