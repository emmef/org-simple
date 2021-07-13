//
// Created by michel on 12-07-21.
//

#include "test-helper.h"
#include <org-simple/util/SignalManager.h>


using Signal = org::simple::util::Signal;
using SignalManager = org::simple::util::SignalManager;
using SignalResult = org::simple::util::SignalResult;

//SignalManager &SignalManager::instance = SignalManager::get_default_instance();
//
//SignalManager &signal_manager = SignalManager::instance;

BOOST_AUTO_TEST_SUITE(org_simple_util_SignalManager)

BOOST_AUTO_TEST_CASE(testInstance) {
  SignalManager manager;
  Signal sig = manager.get_signal();

  BOOST_CHECK(sig.type() == org::simple::util::Signal::Type::NONE);
  BOOST_CHECK(sig.value() == 0);
}

BOOST_AUTO_TEST_CASE(testSetSystemAndGet) {
  SignalManager manager;
  int value = 23;
  BOOST_CHECK(manager.system(value) == org::simple::util::SignalResult::SUCCESS);

  Signal sig = manager.get_signal();
  BOOST_CHECK(sig.type() == org::simple::util::Signal::Type::SYSTEM);
  BOOST_CHECK(sig.value() == value);
}

BOOST_AUTO_TEST_CASE(testSetProgramAndGet) {
  SignalManager manager;
  int value = 23;
  BOOST_CHECK(manager.program(value) == org::simple::util::SignalResult::SUCCESS);

  Signal sig = manager.get_signal();
  BOOST_CHECK(sig.type() == org::simple::util::Signal::Type::PROGRAM);
  BOOST_CHECK(sig.value() == value);
}

BOOST_AUTO_TEST_CASE(testSetUserAndGet) {
  SignalManager manager;
  int value = 23;
  BOOST_CHECK(manager.user(value) == org::simple::util::SignalResult::SUCCESS);

  Signal sig = manager.get_signal();
  BOOST_CHECK(sig.type() == org::simple::util::Signal::Type::USER);
  BOOST_CHECK(sig.value() == value);
}

BOOST_AUTO_TEST_CASE(testSetSystemTwiceMustFail) {
  SignalManager manager;
  BOOST_CHECK(manager.system(23) == org::simple::util::SignalResult::SUCCESS);
  BOOST_CHECK(manager.system(17) == org::simple::util::SignalResult::NOT_ALLOWED);
}

BOOST_AUTO_TEST_CASE(testSetProgramTwiceMustFail) {
  SignalManager manager;
  BOOST_CHECK(manager.program(23) == org::simple::util::SignalResult::SUCCESS);
  BOOST_CHECK(manager.program(17) == org::simple::util::SignalResult::NOT_ALLOWED);
}

BOOST_AUTO_TEST_CASE(testSetUserTwiceMustSucceed) {
  SignalManager manager;
  BOOST_CHECK(manager.user(23) == org::simple::util::SignalResult::SUCCESS);
  BOOST_CHECK(manager.user(17) == org::simple::util::SignalResult::SUCCESS);
}

BOOST_AUTO_TEST_CASE(testSetUserTwiceLockFreeMustFail) {
  SignalManager manager;
  BOOST_CHECK(manager.lockFree().user(23) == org::simple::util::SignalResult::SUCCESS);
  BOOST_CHECK(manager.lockFree().user(17) == org::simple::util::SignalResult::NOT_ALLOWED);
}

BOOST_AUTO_TEST_SUITE_END()

