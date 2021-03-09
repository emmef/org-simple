//
// Created by michel on 05-03-21.
//

#include "boost-unit-tests.h"
#include <org-simple/core/Numbers.h>

using namespace org::simple::core;

BOOST_AUTO_TEST_SUITE(org_simple_core_Numbers)

BOOST_AUTO_TEST_CASE(testDoubleIsNotComplex) {
  static constexpr bool DOUBLE_IS_COMPLEX =
      org::simple::core::is_complex<double>::value;

  BOOST_CHECK_EQUAL(DOUBLE_IS_COMPLEX, false);
  BOOST_CHECK_EQUAL(is_number<double>, true);
}

BOOST_AUTO_TEST_CASE(testComplexDoubleIsComplex) {
  static constexpr bool COMPLEX_DOUBLE_IS_COMPLEX =
      org::simple::core::is_complex<std::complex<double>>::value;

  BOOST_CHECK_EQUAL(COMPLEX_DOUBLE_IS_COMPLEX, true);
  BOOST_CHECK_EQUAL(is_number<std::complex<double>>, true);
}

BOOST_AUTO_TEST_CASE(testComplexComplexDoubleIsNotComplex) {
  static constexpr bool COMPLEX_COMPLEX_DOUBLE_IS_COMPLEX =
      org::simple::core::is_complex<std::complex<std::complex<double>>>::value;

  BOOST_CHECK_EQUAL(COMPLEX_COMPLEX_DOUBLE_IS_COMPLEX, false);
  BOOST_CHECK_EQUAL(is_number<std::complex<std::complex<double>>>, false);
}

BOOST_AUTO_TEST_CASE(testSquaredAbsoluteReal) {
  double x = 4.0;
  double y = Numbers::squared_absolute(x);

  BOOST_CHECK_EQUAL(x * x, y);
}

BOOST_AUTO_TEST_CASE(testSquaredAbsoluteComplexZeroImg) {
  using complex = std::complex<double>;
  double r = 4.0;
  complex x = {r, 0.0};
  double y = Numbers::squared_absolute(x);

  BOOST_CHECK_EQUAL(r * r, y);
}

BOOST_AUTO_TEST_CASE(testSquaredAbsoluteComplex) {
  using complex = std::complex<double>;
  double r = 4.0;
  double i = 3.0;
  complex x = {r, i};
  double y = Numbers::squared_absolute(x);

  BOOST_CHECK_EQUAL(r * r + i * i, y);
}

BOOST_AUTO_TEST_CASE(testTimesConjugate) {
  using complex = std::complex<double>;

  double r1 = 3;
  double i1 = 5;
  double r2 = 7;
  double i2 = 11;
  complex v1 = {r1, i1};
  complex v2 = {r2, i2};
  complex y = Numbers::times_conj(v1, v2);

  BOOST_CHECK_EQUAL(y.real(), v1.real() * v2.real() + v1.imag() * v2.imag());
  BOOST_CHECK_EQUAL(y.imag(), v1.imag() * v2.real() - v1.real() * v2.imag());
}

BOOST_AUTO_TEST_SUITE_END()
