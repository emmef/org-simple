//
// Created by michel on 24-09-20.
//

/*
 * This should normally be included in the build variables, but we want to
 * ensure behaviour is correct with the default, where
 * ORG_SIMPLE_CORE_REF_COUNT_POINTER_ALLOW_NULL_INIT is undefined.
 */
#undef ORG_SIMPLE_CORE_REF_COUNT_POINTER_ALLOW_NULL_INIT

#include "OwnedReference.h"
#include "test-helper.h"
#include <mutex>
#include <org-simple/util/Reference.h>

using Owned = org::simple::test::OwnedReference;
using Owner = org::simple::test::ReferenceOwner;
using Ref = org::simple::util::Reference<org::simple::test::OwnedReference>;

BOOST_AUTO_TEST_SUITE(org_simple_util_Reference)

BOOST_AUTO_TEST_CASE(testReferenceOwnsSingleObjectAndFreesIt) {
  Owner owner(10);
  {
    Ref ref(new Owned(owner));
    BOOST_CHECK_EQUAL(1, owner.count());
  }
  BOOST_CHECK_EQUAL(0, owner.count());
}

BOOST_AUTO_TEST_CASE(testReferencesHandleCopyConstructorCorrectly) {
  Owner owner(10);
  {
    Ref ref(new Owned(owner));
    BOOST_CHECK_EQUAL(1, owner.count());
    {
      Ref ref2(ref);
      BOOST_CHECK_EQUAL(1, owner.count());
    }
    BOOST_CHECK_EQUAL(1, owner.count());
  }
  BOOST_CHECK_EQUAL(0, owner.count());
}

BOOST_AUTO_TEST_CASE(testReferencesHandleAssignmentCorrectly) {
  Owner owner(10);

  {
    Ref ref(new Owned(owner));
    BOOST_CHECK_EQUAL(1, owner.count());
    {
      Ref ref2(new Owned(owner));
      BOOST_CHECK_EQUAL(2, owner.count());
      ref2 = ref; // original owned object of ref2 should be destroyed
      BOOST_CHECK_EQUAL(1, owner.count());
      [[maybe_unused]] Owned *suppressWarning = ref2.get();
    }
    BOOST_CHECK_EQUAL(1, owner.count());
  }
  BOOST_CHECK_EQUAL(0, owner.count());
}

static Ref createNew(Owner &owner, int count_before) {
  Ref ref(new Owned(owner));
  BOOST_CHECK_EQUAL(count_before + 1, owner.count());
  return ref;
}

BOOST_AUTO_TEST_CASE(testReferencesHandleMove) {
  Owner owner(10);
  {
    Ref ref(new Owned(owner));
    BOOST_CHECK_EQUAL(1, owner.count());
    {
      Ref newRef = createNew(owner, 1);
      BOOST_CHECK_EQUAL(2, owner.count());
    }
    BOOST_CHECK_EQUAL(1, owner.count());
  }
  BOOST_CHECK_EQUAL(0, owner.count());
}


BOOST_AUTO_TEST_SUITE_END()
