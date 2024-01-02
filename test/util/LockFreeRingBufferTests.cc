//
// Created by michel on 11-02-21.
//

#include "test-helper.h"
#include <boost/mpl/list.hpp>

#include <org-simple/LockfreeRingBuffer.h>

using namespace boost::unit_test;
using namespace org::simple;

enum class WriteMethod { WRITE, RESET, RESET_COUNT };

struct AbstractRingBufferTester {
  virtual size_t capacity() const noexcept  = 0;
  virtual size_t size() const noexcept = 0;
  virtual size_t read_ptr() const noexcept = 0;
  virtual size_t write_ptr() const noexcept = 0;
  virtual bool empty() const noexcept = 0;
  virtual bool full() const noexcept = 0;
  virtual bool write(const int &value) noexcept = 0;
  virtual bool write_if_empty_reset(const int &value) noexcept = 0;
  virtual bool write_if_empty_reset_total(const int &value, std::atomic_size_t &total) noexcept = 0;
  virtual bool read(int &value) noexcept = 0;
  virtual ~AbstractRingBufferTester() = default;
};

template<class Buffer, WriteMethod method>
class RingBufferWrapper : public AbstractRingBufferTester {
  Buffer &buffer;
  std::atomic_size_t count;
public:
  RingBufferWrapper(Buffer &buffer__) : buffer(buffer__){ count = 0; }
  size_t capacity() const noexcept override { return buffer.capacity(); }
  size_t size() const noexcept override { return buffer.size(); }
  size_t read_ptr() const noexcept override { return buffer.read_ptr(); }
  size_t write_ptr() const noexcept override { return buffer.write_ptr(); }
  size_t writes() const noexcept { return write_ptr() + count; }
  size_t reads() const noexcept { return read_ptr() + count; }
  bool empty() const noexcept override { return buffer.empty(); }
  bool full() const noexcept override { return buffer.full(); }
  bool write(const int &value) noexcept override {
    if constexpr(method == WriteMethod::RESET_COUNT) {
      return buffer.write_if_empty_reset_total(value, count);
    }
    else if constexpr (method == WriteMethod::RESET) {
      return buffer.write_if_empty_reset(value);
    }
    else {
      return buffer.write(value);
    }
  }
  bool write_if_empty_reset(const int &value) noexcept override { return buffer.write_if_empty_reset(value); }
  bool write_if_empty_reset_total(const int &value, std::atomic_size_t &total) noexcept override { return buffer.write_if_empty_reset_total(value, total); }
  bool read(int &value) noexcept override { return buffer.read(value); }
};

static constexpr size_t SIZE = 4;
static constexpr size_t VAR_SIZE = 16;

template<WriteMethod method>
class FixedBufferTester {
  RingBufferLockFreeFixedSize<int, SIZE> buffer;
public:
  FixedBufferTester() {}
  auto wrapped() {
    return RingBufferWrapper<RingBufferLockFreeFixedSize<int, SIZE>, method>(buffer);
  }
  static constexpr WriteMethod METHOD = method;
};

template<WriteMethod method>
class VariableBuffer {
  int data[VAR_SIZE];
  RingBufferLockFree<int>::Metric metric;
  RingBufferLockFree<int> buffer;
public:
  VariableBuffer() : metric(VAR_SIZE), buffer(metric, data) {
    metric.set_elements(SIZE);
  }
  auto wrapped() {
    return RingBufferWrapper<RingBufferLockFree<int>, method>(buffer);
  }
  static constexpr WriteMethod METHOD = method;
};

typedef boost::mpl::list<
    FixedBufferTester<WriteMethod::WRITE>,
    FixedBufferTester<WriteMethod::RESET>,
    FixedBufferTester<WriteMethod::RESET_COUNT>,
    VariableBuffer<WriteMethod::WRITE>,
    VariableBuffer<WriteMethod::RESET>,
    VariableBuffer<WriteMethod::RESET_COUNT>
    > testTypes;

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
  BOOST_CHECK_EQUAL(0, buffer.read_ptr());
  BOOST_CHECK_EQUAL(1, buffer.write_ptr());

  int read = 13;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
  BOOST_CHECK_EQUAL(read, 1);
  BOOST_CHECK_EQUAL(1, buffer.read_ptr());
  BOOST_CHECK_EQUAL(1, buffer.write_ptr());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testFillUntilFull, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();

  BOOST_CHECK(buffer.write(1));
  BOOST_CHECK(buffer.write(2));
  BOOST_CHECK_EQUAL(0, buffer.read_ptr());
  BOOST_CHECK_EQUAL(2, buffer.write_ptr());

  BOOST_CHECK(buffer.write(3));
  BOOST_CHECK_EQUAL(buffer.size(), 3);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  BOOST_CHECK_EQUAL(0, buffer.read_ptr());
  BOOST_CHECK_EQUAL(3, buffer.write_ptr());

  BOOST_CHECK(buffer.write(4));
  BOOST_CHECK_EQUAL(buffer.size(), 4);
  BOOST_CHECK(buffer.full());
  BOOST_CHECK_EQUAL(0, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
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
  BOOST_CHECK_EQUAL(0, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
}


BOOST_AUTO_TEST_CASE_TEMPLATE(testFillUntilFullAndReadUntilEmpty, Tester, testTypes) {
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
  BOOST_CHECK_EQUAL(1, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 2);
  BOOST_CHECK_EQUAL(read, 2);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  BOOST_CHECK_EQUAL(2, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 1);
  BOOST_CHECK_EQUAL(read, 3);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  BOOST_CHECK_EQUAL(3, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
  read = 12;
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK_EQUAL(read, 4);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
  BOOST_CHECK_EQUAL(4, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
  read = 12;
  BOOST_CHECK(!buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK_EQUAL(read, 12);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
  BOOST_CHECK_EQUAL(4, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testFillUntilFullAndReadUntilEmptyWrite, Tester, testTypes) {
  Tester tester;
  auto buffer = tester.wrapped();
  int read;

  BOOST_CHECK(buffer.write(1));
  BOOST_CHECK(buffer.write(2));
  BOOST_CHECK(buffer.write(3));
  BOOST_CHECK(buffer.write(4));
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(4, buffer.read_ptr());
  BOOST_CHECK_EQUAL(4, buffer.write_ptr());

  BOOST_CHECK(buffer.write(5));
  BOOST_CHECK_EQUAL(buffer.size(), 1);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(!buffer.empty());
  size_t ptr_base = Tester::METHOD == WriteMethod::WRITE ? SIZE : 0;
  size_t size_base = Tester::METHOD == WriteMethod::RESET ? 0 : SIZE;
  BOOST_CHECK_EQUAL(ptr_base + 0, buffer.read_ptr());
  BOOST_CHECK_EQUAL(ptr_base + 1, buffer.write_ptr());
  BOOST_CHECK_EQUAL(size_base + 0, buffer.reads());
  BOOST_CHECK_EQUAL(size_base + 1, buffer.writes());
  read = 12;

  BOOST_CHECK(buffer.read(read));
  BOOST_CHECK_EQUAL(buffer.size(), 0);
  BOOST_CHECK(!buffer.full());
  BOOST_CHECK(buffer.empty());
  BOOST_CHECK_EQUAL(read, 5);
  BOOST_CHECK_EQUAL(ptr_base + 1, buffer.read_ptr());
  BOOST_CHECK_EQUAL(ptr_base + 1, buffer.write_ptr());
  BOOST_CHECK_EQUAL(size_base + 1, buffer.reads());
  BOOST_CHECK_EQUAL(size_base + 1, buffer.writes());
}

BOOST_AUTO_TEST_SUITE_END();
