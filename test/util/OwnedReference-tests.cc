//
// Created by michel on 24-09-20.
//

#include "OwnedReference.h"
#include "boost-unit-tests.h"

using Owned = org::simple::test::OwnedReference;
using Owner = org::simple::test::ReferenceOwner;

BOOST_AUTO_TEST_SUITE(org_simple_util_test_OwnedReference)

BOOST_AUTO_TEST_CASE(testOwnerStarsWithZeroErrors) {
  Owner owner(10);

  BOOST_CHECK_EQUAL(0, owner.errors());
}

BOOST_AUTO_TEST_CASE(testOwnerCountMatchesAdds) {
  Owner owner(2);
  int x, y;

  BOOST_CHECK_EQUAL(0, owner.count());
  owner.add_get_id(&x);
  BOOST_CHECK_EQUAL(1, owner.count());
  owner.add_get_id(&y);
  BOOST_CHECK_EQUAL(2, owner.count());
}

BOOST_AUTO_TEST_CASE(testOwnerCountMatchesAddsAndRemoves) {
  Owner owner(2);
  int x, y;
  int x_id;
  [[maybe_unused]] int y_id;

  BOOST_CHECK_EQUAL(0, owner.count());
  x_id = owner.add_get_id(&x);
  BOOST_CHECK_EQUAL(1, owner.count());
  owner.add_get_id(&y);
  BOOST_CHECK_EQUAL(2, owner.count());
  owner.remove(&x, x_id);
  BOOST_CHECK_EQUAL(1, owner.count());
}

BOOST_AUTO_TEST_CASE(testOwnerRemoveWrongIdReportedAndCleanedUp) {
  Owner owner(2);
  int x;
  int x_id;

  x_id = owner.add_get_id(&x);
  BOOST_CHECK_EQUAL(1, owner.count());
  owner.remove(&x, x_id + 5);
  BOOST_CHECK_EQUAL(1, owner.count());
  BOOST_CHECK_EQUAL(1, owner.id_wrong());
  BOOST_CHECK_EQUAL(1, owner.errors());
  BOOST_CHECK_EQUAL(1, owner.cleanup_count(false));
}

BOOST_AUTO_TEST_CASE(testOwnerRemoveWrongPtrReportedAndCleanedUp) {
  Owner owner(2);
  int x, y;
  int x_id;

  x_id = owner.add_get_id(&x);
  BOOST_CHECK_EQUAL(1, owner.count());
  owner.remove(&y, x_id);
  BOOST_CHECK_EQUAL(1, owner.count());
  BOOST_CHECK_EQUAL(1, owner.ptr_wrong());
  BOOST_CHECK_EQUAL(1, owner.errors());
  BOOST_CHECK_EQUAL(1, owner.cleanup_count(false));
}

BOOST_AUTO_TEST_CASE(testOwnerRemoveNotFoundReportedAndCleanedUp) {
  Owner owner(2);
  int x, y;
  int x_id;

  x_id = owner.add_get_id(&x);
  BOOST_CHECK_EQUAL(1, owner.count());
  owner.remove(&y, x_id + 5);
  BOOST_CHECK_EQUAL(1, owner.count());
  BOOST_CHECK_EQUAL(1, owner.not_found());
  BOOST_CHECK_EQUAL(1, owner.errors());
  BOOST_CHECK_EQUAL(1, owner.cleanup_count(false));
}

BOOST_AUTO_TEST_CASE(testOwnerThrowsIfFullAndAdd) {
  Owner owner(1);
  int x, y;

  owner.add_get_id(&x);
  BOOST_CHECK_THROW(owner.add_get_id(&y), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testOwnerThrowsIfAddDuplicate) {
  Owner owner(1);
  int x;

  owner.add_get_id(&x);

  BOOST_CHECK_THROW(owner.add_get_id(&x), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testOwnedConstructionDestructionMatch) {
  Owner owner(1);
  {
    Owned ref(owner);
    BOOST_CHECK_EQUAL(1, owner.count());
  }
  BOOST_CHECK_EQUAL(0, owner.cleanup_count());
}

BOOST_AUTO_TEST_CASE(testOwnedConstructionDestructionMatchMulti) {
  Owner owner(2);
  {
    Owned ref1(owner);
    BOOST_CHECK_EQUAL(1, owner.count());
    {
      Owned ref2(owner);
      BOOST_CHECK_EQUAL(2, owner.count());
    }
    BOOST_CHECK_EQUAL(1, owner.count());
  }
  BOOST_CHECK_EQUAL(0, owner.cleanup_count());
}

BOOST_AUTO_TEST_SUITE_END()
