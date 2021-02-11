//
// Created by michel on 11-02-21.
//

#include "test-helper.h"
#include <boost/mpl/list.hpp>

#include <org-simple/util/LockfreeRingBuffer.h>

using namespace boost::unit_test;
using namespace org::simple::util;

struct AbstractRingBufferTester {
  virtual size_t capacity() const noexcept  = 0;
  virtual size_t size() const noexcept = 0;
  virtual bool empty() const noexcept = 0;
  virtual bool full() const noexcept = 0;
  virtual bool write(const int &value) noexcept = 0;
  virtual bool write_if_empty_reset(const int &value) noexcept = 0;
  virtual bool write_if_empty_reset_total(const int &value, std::atomic_size_t &total) noexcept = 0;
  virtual bool read(int &value) noexcept = 0;
  virtual ~AbstractRingBufferTester() = default;
};

template<class Buffer>
class RingBufferWrapper : public AbstractRingBufferTester {
  Buffer &buffer;
public:
  RingBufferWrapper(Buffer &buffer__) : buffer(buffer__){}
  virtual size_t capacity() const noexcept { return buffer.capacity(); }
  virtual size_t size() const noexcept { return buffer.size(); }
  virtual bool empty() const noexcept { return buffer.empty(); }
  virtual bool full() const noexcept { return buffer.full(); }
  virtual bool write(const int &value) noexcept { return buffer.write(value); }
  virtual bool write_if_empty_reset(const int &value) noexcept { return buffer.write_if_empty_reset(value); }
  virtual bool write_if_empty_reset_total(const int &value, std::atomic_size_t &total) noexcept { return buffer.write_if_empty_reset_total(value, total); }
  virtual bool read(int &value) noexcept{ return buffer.read(value); }
};

static constexpr size_t SIZE = 4;
static constexpr size_t VAR_SIZE = 16;

class FixedBufferTester {
  RingBufferLockFreeFixedSize<int, SIZE> buffer;
public:
  FixedBufferTester() {}
  RingBufferWrapper<RingBufferLockFreeFixedSize<int, SIZE>> wrapped() {
    return RingBufferWrapper<RingBufferLockFreeFixedSize<int, SIZE>>(buffer);
  }
};

class VariableBuffer {
  int data[VAR_SIZE];
  RingBufferLockFree<int>::Metric metric;
  RingBufferLockFree<int> buffer;
public:
  VariableBuffer() : metric(VAR_SIZE), buffer(metric, data) {
    metric.set_elements(SIZE);
  }
  RingBufferWrapper<RingBufferLockFree<int>> wrapped() {
    return RingBufferWrapper<RingBufferLockFree<int>>(buffer);
  }
};

typedef boost::mpl::list<FixedBufferTester, VariableBuffer> testTypes;

BOOST_AUTO_TEST_SUITE(org_simple_util_LockFreeRingBuffer)

BOOST_AUTO_TEST_CASE_TEMPLATE(testInit, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();

  BOOST_CHECK(buffer.capacity() == SIZE);
  BOOST_CHECK(buffer.size() == 0);
  BOOST_CHECK(buffer.empty());
  BOOST_CHECK(!buffer.full());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testSingleWriteAndRead, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();

  BOOST_CHECK(buffer.write(1));
  BOOST_CHECK_EQUAL(buffer.size(), 1);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  int read = 13;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
  BOOST_CHECK_EQUAL(read, 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testFillUntilFull, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();

  BOOST_CHECK(buffer.write(1));
  BOOST_CHECK(buffer.write(2));
  BOOST_CHECK(buffer.write(3));
  BOOST_CHECK_EQUAL(buffer.size(), 3);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());

  BOOST_CHECK(buffer.write(4));
  BOOST_CHECK_EQUAL(buffer.size(), 4);
  BOOST_CHECK(buffer.full());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testFillUntilFullPlusOne, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();

  BOOST_CHECK(buffer.write(1));
  BOOST_CHECK(buffer.write(2));
  BOOST_CHECK(buffer.write(3));
  BOOST_CHECK(buffer.write(4));
  BOOST_CHECK(!buffer.write(5));
  BOOST_CHECK_EQUAL(buffer.size(), 4);
  BOOST_CHECK(buffer.full());
}


BOOST_AUTO_TEST_CASE_TEMPLATE(testFillUntilFullAndRead, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();
  int read;

  BOOST_CHECK(buffer.write(1));
  BOOST_CHECK(buffer.write(2));
  BOOST_CHECK(buffer.write(3));
  BOOST_CHECK(buffer.write(4));
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 3);
  BOOST_CHECK_EQUAL(read, 1);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 2);
  BOOST_CHECK_EQUAL(read, 2);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 1);
  BOOST_CHECK_EQUAL(read, 3);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK_EQUAL(read, 4);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
  read = 12;
  BOOST_CHECK(!buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK_EQUAL(read, 12);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
}

BOOST_AUTO_TEST_SUITE_END();
