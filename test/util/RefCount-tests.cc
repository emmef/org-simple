//
// Created by michel on 24-09-20.
//

/*
 * This should normally be included in the build variables, but we want to
 * ensure behaviour is correct with the default, where
 * ORG_SIMPLE_CORE_REF_COUNT_POINTER_ALLOW_NULL_INIT is undefined.
 */
#undef ORG_SIMPLE_CORE_REF_COUNT_POINTER_ALLOW_NULL_INIT

#include "test-helper.h"
#include <org-simple/util/Reference.h>
#include "OwnedReference.h"

using Owned = org::simple::test::OwnedReference;
using Owner = org::simple::test::ReferenceOwner;
using Ref = org::simple::util::TypedRefCountPointer<Owned>;

struct RefCntPtr : public org::simple::util::UntypedRefCountPointer {
  void destroy_ptr(void *ptr) noexcept override { free(ptr); }

  explicit RefCntPtr(void *ptr) : UntypedRefCountPointer(ptr) {}
};

BOOST_AUTO_TEST_SUITE(org_simple_util_RefCount)

BOOST_AUTO_TEST_CASE(testUntypedRefCountPointerThrowsOnNullInitialization) {
  BOOST_CHECK_THROW(RefCntPtr(nullptr), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(testUntypedRefCountPointerGetReturnsInitArgument) {
  int *ptr = new int;
  RefCntPtr refPtr(ptr);

  BOOST_CHECK_EQUAL((void *)ptr, refPtr.get_ptr());
  delete ptr;
}

BOOST_AUTO_TEST_CASE(testUntypedRefCountPointerRemoveReferenceReturnsOne) {
  int *ptr = new int;
  RefCntPtr refPtr(ptr);

  BOOST_CHECK_EQUAL(1, refPtr.del_ref_no_destroy());
  delete ptr;
}

BOOST_AUTO_TEST_CASE(
    testUntypedRefCountPointerAddThenRemoveReferenceReturnsTwo) {
  int *ptr = new int;
  RefCntPtr refPtr(ptr);

  refPtr.add_ref();
  BOOST_CHECK_EQUAL(2, refPtr.del_ref_no_destroy());
  delete ptr;
}

BOOST_AUTO_TEST_CASE(testUntypedRefCountPointerRemoveReferenceDestroys) {
  int *ptr = new int;
  RefCntPtr refPtr(ptr);

  BOOST_CHECK_EQUAL(true, refPtr.del_ref_get_if_destroyed());
}

BOOST_AUTO_TEST_CASE(
    testUntypedRefCountPointerAddThenRemoveReferenceDoesNotDestroy) {
  int *ptr = new int;
  RefCntPtr refPtr(ptr);

  refPtr.add_ref();
  BOOST_CHECK_EQUAL(false, refPtr.del_ref_get_if_destroyed());
  delete ptr;
}

BOOST_AUTO_TEST_CASE(
    testUntypedRefCountPointerAddThenRemoveReferenceTwiceDestroys) {
  int *ptr = new int;
  RefCntPtr refPtr(ptr);

  refPtr.add_ref();
  BOOST_CHECK_EQUAL(false, refPtr.del_ref_get_if_destroyed());
  BOOST_CHECK_EQUAL(true, refPtr.del_ref_get_if_destroyed());
}

BOOST_AUTO_TEST_CASE(testInitDelDestroyCorrect) {
  Owner owner(10);
  auto *owned = new Owned(owner);
  Ref refPtr(owned);
  BOOST_CHECK_EQUAL(1, owner.count());
  bool destroyed = refPtr.del_ref_get_if_destroyed();
  BOOST_CHECK_MESSAGE(destroyed, "Owned object was destroyed");
  BOOST_CHECK_EQUAL(0, owner.count());
}

BOOST_AUTO_TEST_CASE(testInitDelDestroyDoubleNoop) {
  Owner owner(10);
  auto *owned = new Owned(owner);
  Ref refPtr(owned);
  BOOST_CHECK_EQUAL(1, owner.count());
  refPtr.del_ref_get_if_destroyed();
  bool destroyed = refPtr.del_ref_get_if_destroyed();
  BOOST_CHECK_MESSAGE(!destroyed, "Owned object was not doubly destroyed");
  BOOST_CHECK_EQUAL(0, owner.count());
}


BOOST_AUTO_TEST_CASE(testInitDelDestroyDoubleAdd) {
  Owner owner(10);
  auto *owned = new Owned(owner);
  Ref refPtr(owned);
  BOOST_CHECK_EQUAL(1, owner.count());
  refPtr.add_ref();
  BOOST_CHECK_EQUAL(1, owner.count());
  bool destroyed = refPtr.del_ref_get_if_destroyed();
  BOOST_CHECK_MESSAGE(!destroyed, "Owned object has multiple owners");
  destroyed = refPtr.del_ref_get_if_destroyed();
  BOOST_CHECK_MESSAGE(destroyed, "Owned object was destroyed");
  BOOST_CHECK_EQUAL(0, owner.count());
}

BOOST_AUTO_TEST_CASE(testInitDelNoDestroyCorrect) {
  Owner owner(10);
  auto *owned = new Owned(owner);
  Ref refPtr(owned);
  BOOST_CHECK_EQUAL(1, owner.count());
  ptrdiff_t count_before_del_ref = refPtr.del_ref_no_destroy();
  BOOST_CHECK_EQUAL(1, count_before_del_ref);
  BOOST_CHECK_EQUAL(1, owner.count());
  delete owned;
  BOOST_CHECK_EQUAL(0, owner.count());
}

BOOST_AUTO_TEST_CASE(testInitDelNoDestroyDoubleNoop) {
  Owner owner(10);
  auto *owned = new Owned(owner);
  Ref refPtr(owned);
  ptrdiff_t count_before_del_ref = refPtr.del_ref_no_destroy();
  BOOST_CHECK_EQUAL(1, count_before_del_ref);
  count_before_del_ref = refPtr.del_ref_no_destroy();
  BOOST_CHECK_EQUAL(0, count_before_del_ref);
  BOOST_CHECK_EQUAL(1, owner.count());
  delete owned;
  BOOST_CHECK_EQUAL(0, owner.count());
}

BOOST_AUTO_TEST_SUITE_END()
