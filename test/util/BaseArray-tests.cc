//
// Created by michel on 22-10-20.
//

#include "test-helper.h"
#include <org-simple/util/BaseArray.h>

using namespace org::simple::util;


namespace {

typedef double value;
constexpr size_t SIZE = 10;
typedef ArrayInline<value, SIZE> Array;

static_assert(org::simple::util::concepts::BaseArrayConstSizeImpl<value, Array>);


}

BOOST_AUTO_TEST_SUITE(org_simple_util_NumArray)

BOOST_AUTO_TEST_CASE(testIsArrayInlineConstSizeArray) {
  BOOST_CHECK(true);
}


BOOST_AUTO_TEST_SUITE_END()
