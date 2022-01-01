//
// Created by michel on 01-01-22.
//

#include <org-simple/util/text/ReplayStream.h>
#include "boost-unit-tests.h"

using namespace org::simple::util;



BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_ReplayStream)

BOOST_AUTO_TEST_CASE(test_add_get_get) {
  text::ReplayStream<int, 1> stream;
  int value;
  stream << 1;
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(1, value);
  value = 13;
  BOOST_CHECK_EQUAL(false, stream.get(value));
  BOOST_CHECK_EQUAL(13, value);
}

BOOST_AUTO_TEST_CASE(test_add3_get4) {
  text::ReplayStream<int, 3> stream;
  int value;

  stream << 1;
  stream << 2;
  stream << 3;
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(1, value);
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(2, value);
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(3, value);
  value = 13;
  BOOST_CHECK_EQUAL(false, stream.get(value));
  BOOST_CHECK_EQUAL(13, value);
}

BOOST_AUTO_TEST_CASE(test_add_2_get1_add1_get2) {
  text::ReplayStream<int, 3> stream;
  int value;

  stream << 1;
  stream << 2;
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(1, value);
  stream << 3;
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(2, value);
  BOOST_CHECK_EQUAL(true, stream.get(value));
  BOOST_CHECK_EQUAL(3, value);
  value = 13;
  BOOST_CHECK_EQUAL(false, stream.get(value));
  BOOST_CHECK_EQUAL(13, value);
}

BOOST_AUTO_TEST_SUITE_END()
