//
// Created by michel on 01-01-22.
//

#include <org-simple/util/text/InputStream.h>
#include "boost-unit-tests.h"

/**
 * These are not tests, as the input stream is just an interface (pure virtual
 * class). However, some important traits need to be verified here.
 */
using namespace org::simple::util::text;

static constexpr char dummyValue = 13;

class Sub : public InputStream<char> {
  static_assert(Traits::template isA<InputStream<char>>());
public:
  bool get(char &c) {
    c = dummyValue;
    return true;
  }
};

BOOST_AUTO_TEST_SUITE(test_org_simple_util_text_InputStream)

BOOST_AUTO_TEST_CASE(testSubAsAFormality) {
  Sub x;
  char value = 0;
  BOOST_CHECK_EQUAL(true, x.get(value));
  BOOST_CHECK_EQUAL(dummyValue, x.get(value));
}

BOOST_AUTO_TEST_SUITE_END()
