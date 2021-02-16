#ifndef ORG_SIMPLE_LOCKFREERINGBUFFER_H
#define ORG_SIMPLE_LOCKFREERINGBUFFER_H
/*
 * org-simple/LockfreeRingBuffer.h
 *
 * Added by michel on 2020-10-21
 * Copyright (C) 2015-2020 Michel Fleur.
 * Source https://github.com/emmef/org-simple
 * Email org-simple@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <atomic>
#include <cstddef>
#include <org-simple/core/Circular.h>
#include <org-simple/core/Power2.h>

namespace org::simple::util {

struct LockFreeRingBuffer {

  template <typename T, size_t S> class BaseMonotonicFixedMasked {
    std::atomic_size_t read_at = 0;
    std::atomic_size_t write_at = 0;
    using Metric = ::org::simple::core::Circular::FixedMetric<
        ::org::simple::core::WrappingType::BIT_MASK, S>;

  public:
    size_t capacity() const noexcept { return Metric::elements(); }
    size_t size() const noexcept { return write_at - read_at; }
    bool empty() const noexcept { return size() == 0; }
    bool full() const noexcept { return size() == capacity(); }
    size_t read_ptr() const noexcept { return read_at; }
    size_t write_ptr() const noexcept { return write_at; }

    /**
     * Pushes \c value on the queue, which fails if the queue is full.
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write(const T &value, T *const data) noexcept {
      size_t wr = write_at;
      size_t rd = read_at;
      if (wr - rd >= capacity()) {
        return false;
      }
      data[Metric::wrapped(wr)] = value;
      std::atomic_thread_fence(std::memory_order_release);
      write_at = wr + 1;
      return true;
    }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty resets read and write counters.
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset(const T &value, T *const data) noexcept {
      size_t wr = write_at;
      size_t rd = read_at;
      size_t elements = wr - rd;
      if (elements == 0) {
        // Buffer is EMPTY, so no read can happen and resetting counters
        // can be done without a race condition
        data[Metric::wrapped(0)] = value;
        std::atomic_thread_fence(std::memory_order_release);
        write_at = 1;
        read_at = 0;
        return true;
      } else if (elements >= capacity()) {
        return false;
      }
      data[Metric::wrapped(wr)] = value;
      std::atomic_thread_fence(std::memory_order_release);
      write_at = wr + 1;
      return true;
    }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty adds write counter to total and resets read and write
     * counters.
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset_total(const T &value, T *const data,
                                    std::atomic_size_t &total) noexcept {
      size_t wr = write_at;
      size_t rd = read_at;
      size_t elements = wr - rd;
      if (elements == 0) {
        // Buffer is EMPTY, so no read can happen and resetting counters
        // can be done without a race condition
        total += wr;
        data[Metric::wrapped(0)] = value;
        std::atomic_thread_fence(std::memory_order_release);
        write_at = 1;
        read_at = 0;
        return true;
      } else if (elements >= capacity()) {
        return false;
      }
      data[Metric::wrapped(wr)] = value;
      std::atomic_thread_fence(std::memory_order_release);
      write_at = wr + 1;
      return true;
    }

    /**
     * Shifts a value off the queue into \c value, which fails if the queue is
     * empty.
     * @param value Contains the shifted value on success.
     * @returns \c true if shift was successful, \c false otherwise
     */
    bool read(T &value, const T *const data) noexcept {
      size_t rd = read_at;
      size_t wr = write_at;
      if (wr <= rd) {
        return false;
      }
      std::atomic_thread_fence(std::memory_order_acquire);
      value = data[Metric::wrapped(rd)];
      read_at = rd + 1;
      return true;
    }
  };

  template <typename T, class Metric> class BaseMonotonic {
    std::atomic_size_t read_at = 0;
    std::atomic_size_t write_at = 0;
    const Metric &metric;

  public:
    BaseMonotonic(const Metric &metric__) : metric(metric__) {}

    size_t capacity() const noexcept { return metric.elements(); }
    size_t size() const noexcept { return write_at - read_at; }
    bool empty() const noexcept { return size() == 0; }
    bool full() const noexcept { return size() == capacity(); }
    size_t read_ptr() const noexcept { return read_at; }
    size_t write_ptr() const noexcept { return write_at; }

    /**
     * Pushes \c value on the queue, which fails if the queue is full.
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write(const T &value, T *const data) noexcept {
      size_t wr = write_at;
      size_t rd = read_at;
      if (wr - rd >= capacity()) {
        return false;
      }
      data[metric.wrapped(wr)] = value;
      std::atomic_thread_fence(std::memory_order_release);
      write_at = wr + 1;
      return true;
    }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty resets read and write counters.
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset(const T &value, T *const data) noexcept {
      size_t wr = write_at;
      size_t rd = read_at;
      size_t elements = wr - rd;
      if (elements == 0) {
        // Buffer is EMPTY, so no read can happen and resetting counters
        // can be done without a race condition
        data[metric.wrapped(0)] = value;
        std::atomic_thread_fence(std::memory_order_release);
        write_at = 1;
        read_at = 0;
        return true;
      } else if (elements >= capacity()) {
        return false;
      }
      data[metric.wrapped(wr)] = value;
      std::atomic_thread_fence(std::memory_order_release);
      write_at = wr + 1;
      return true;
    }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty adds write counter to total and resets read and write
     * counters.
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset_total(const T &value, T *const data,
                                    std::atomic_size_t &total) noexcept {
      size_t wr = write_at;
      size_t rd = read_at;
      size_t elements = wr - rd;
      if (elements == 0) {
        // Buffer is EMPTY, so no read can happen and resetting counters
        // can be done without a race condition
        total += wr;
        data[metric.wrapped(0)] = value;
        std::atomic_thread_fence(std::memory_order_release);
        write_at = 1;
        read_at = 0;
        return true;
      } else if (elements >= capacity()) {
        return false;
      }
      data[metric.wrapped(wr)] = value;
      std::atomic_thread_fence(std::memory_order_release);
      write_at = wr + 1;
      return true;
    }

    /**
     * Shifts a value off the queue into \c value, which fails if the queue is
     * empty.
     * @param value Contains the shifted value on success.
     * @returns \c true if shift was successful, \c false otherwise
     */
    bool read(T &value, const T *const data) noexcept {
      size_t rd = read_at;
      size_t wr = write_at;
      if (wr <= rd) {
        return false;
      }
      std::atomic_thread_fence(std::memory_order_acquire);
      value = data[metric.wrapped(rd)];
      read_at = rd + 1;
      return true;
    }
  };

  /**
   * A ring buffer that is thread-safe for a single producer and a single
   * consumer thread only.
   *
   * The buffer implementation is extremely simple, using monotonic rising read
   * and write pointers (wrapped for buffer access). This has the advantage that
   * the total number of reads and writes can be obtained. However, the buffer
   * state is undefined if the number of writes exceeds the maximum value of
   * size_t.
   *
   * @tparam T The type of values in the buffer.
   * @tparam S The buffer capacity
   */
  template <typename T, size_t S> class MonotonicFixed {
    static_assert(S > 0 && org::simple::core::Power2::is(S));
    BaseMonotonicFixedMasked<T, S> base;
    T data[S];

  public:
    size_t capacity() const noexcept { return base.capacity(); }
    size_t size() const noexcept { return base.size(); }
    bool empty() const noexcept { return base.empty(); }
    bool full() const noexcept { return base.full(); }
    size_t read_ptr() const noexcept { return base.read_ptr(); }
    size_t write_ptr() const noexcept { return base.write_ptr(); }

    void zero() noexcept {
      std::memset(data, 0, sizeof(data));
    }
    /**
     * Pushes \c value on the queue, which fails if the queue is full.
     *
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write(const T &value) noexcept { return base.write(value, data); }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty resets read and write counters.
     *
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset(const T &value) noexcept {
      return base.write_if_empty_reset(value, data);
    }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty resets read and write counters and adds the previous
     * value of the write pointer to the total.
     *
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset_total(const T &value,
                                    std::atomic_size_t &total) noexcept {
      return base.write_if_empty_reset_total(value, data, total);
    }

    /**
     * Shifts a value off the queue into \c value, which fails if the queue is
     * empty.
     * @param value Contains the shifted value on success.
     * @returns \c true if shift was successful, \c false otherwise
     */
    bool read(T &value) noexcept { return base.read(value, data); }
  };

  /**
   * A ring buffer that is thread-safe for a single producer and a single
   * consumer thread only.
   *
   * The buffer implementation is extremely simple, using monotonic rising read
   * and write pointers (wrapped for buffer access). This has the advantage that
   * the total number of reads and writes can be obtained. However, the buffer
   * state is undefined if the number of writes exceeds the maximum value of
   * size_t.
   *
   * @tparam T The type of values in the buffer.
   * @tparam S The buffer capacity
   */
  template <typename T, class M> class Monotonic {
    BaseMonotonic<T, M> base;
    T *data;

  public:
    using Metric = M;
    Monotonic(const Metric &metric, T * data__) : base(metric), data(data__) {}

    size_t capacity() const noexcept { return base.capacity(); }
    size_t size() const noexcept { return base.size(); }
    bool empty() const noexcept { return base.empty(); }
    bool full() const noexcept { return base.full(); }
    size_t read_ptr() const noexcept { return base.read_ptr(); }
    size_t write_ptr() const noexcept { return base.write_ptr(); }

    /**
     * Pushes \c value on the queue, which fails if the queue is full.
     *
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write(const T &value) noexcept { return base.write(value, data); }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty resets read and write counters.
     *
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset(const T &value) noexcept {
      return base.write_if_empty_reset(value, data);
    }

    /**
     * Pushes \c value on the queue, which fails if the queue is full, and if
     * the queue is empty resets read and write counters and adds the previous
     * value of the write pointer to the total.
     *
     * @param value The value to push.
     * @returns \c true if push was successful, \c false otherwise.
     */
    bool write_if_empty_reset_total(const T &value,
                                    std::atomic_size_t &total) noexcept {
      return base.write_if_empty_reset_total(value, data, total);
    }

    /**
     * Shifts a value off the queue into \c value, which fails if the queue is
     * empty.
     * @param value Contains the shifted value on success.
     * @returns \c true if shift was successful, \c false otherwise
     */
    bool read(T &value) { return base.read(value, data); }
  };
};

template <typename T, size_t S>
using RingBufferLockFreeFixedSize = LockFreeRingBuffer::MonotonicFixed<T, S>;
template <typename T>
using RingBufferLockFree = LockFreeRingBuffer::Monotonic<
    T, ::org::simple::core::Circular::Metric<
           ::org::simple::core::WrappingType::BIT_MASK>>;

} // namespace org::simple::util

#endif // ORG_SIMPLE_LOCKFREERINGBUFFER_H
