#ifndef ORG_SIMPLE_UTIL_M_REFERENCE_H
#define ORG_SIMPLE_UTIL_M_REFERENCE_H
/*
 * org-simple/util/Reference.h
 *
 * Added by michel on 2020-09-23
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
#include <stdexcept>

#ifndef ORG_SIMPLE_CORE_REF_COUNT_POINTER_ALLOW_NULL_INIT
static void *ref_counter_pointer_check_null_init(void *ptr) {
  if (ptr) {
    return ptr;
  }
  throw std::invalid_argument(
      "org::simple::util::UntypedRefCountPointer: nullptr");
}
#else
static constexpr void *ref_counter_pointer_check_null_init(void *ptr) {
  return ptr;
}
#endif

#ifdef ORG_SIMPLE_CORE_REF_COUNT_POINTER_CHECK_NULL_DEREFERENCE
static void *ref_counter_pointer_check_null_dereference(void *ptr) {
  if (ptr) {
    return ptr;
  }
  throw std::invalid_argument(
      "org::simple::util::UntypedRefCountPointer: nullptr");
}
#else
static constexpr void *ref_counter_pointer_check_null_dereference(void *ptr) {
  return ptr;
}
#endif

namespace org::simple::util {

class UntypedRefCountPointer {
  void *ptr_;
  std::atomic<ptrdiff_t> count_;

protected:
  virtual void destroy_ptr(void *ptr) noexcept = 0;

public:
  explicit UntypedRefCountPointer(void *ptr)
      : ptr_(ref_counter_pointer_check_null_init(ptr)), count_(1) {}
  UntypedRefCountPointer(const UntypedRefCountPointer &) = delete;
  UntypedRefCountPointer(UntypedRefCountPointer &&) = delete;

  bool add_ref() { return count_.fetch_add(1) >= 1; }

  ptrdiff_t del_ref_no_destroy() noexcept { return count_.fetch_sub(1); }

  bool del_ref_get_if_destroyed() noexcept {
    ptrdiff_t previousValue = count_.fetch_sub(1);
    if (previousValue == 1) {
      destroy();
      return true;
    }
    return false;
  }

  void destroy() noexcept {
    static std::atomic_flag flag_;
    // At least attempts to use recent state and make memory visible afterwards.
    flag_.test_and_set(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    destroy_ptr(ptr_);
    ptr_ = nullptr;
    std::atomic_thread_fence(std::memory_order_release);
    flag_.clear();
  }

  [[nodiscard]] void *get_ptr() const noexcept {
    return ref_counter_pointer_check_null_dereference(ptr_);
  }

  virtual ~UntypedRefCountPointer() noexcept = default;
};

template <typename T>
class TypedRefCountPointer final : public UntypedRefCountPointer {
protected:
  void destroy_ptr(void *ptr) noexcept override {
    delete static_cast<T *>(ptr);
  }

public:
  explicit TypedRefCountPointer(T *ptr) : UntypedRefCountPointer(ptr){};
  TypedRefCountPointer(const TypedRefCountPointer &) = delete;
  TypedRefCountPointer(TypedRefCountPointer &&) = delete;

  T *get() { return static_cast<T *>(get_ptr()); }

  ~TypedRefCountPointer() noexcept override {
    if (del_ref_no_destroy() >= 1) {
      destroy();
    }
  }
};

template <typename T> class Reference {
  TypedRefCountPointer<T> *ptr_;

  static void cleanup(UntypedRefCountPointer *ptr_) {
    if (ptr_ && ptr_->del_ref_get_if_destroyed()) {
      delete ptr_;
    }
  }

public:
  Reference() : ptr_(nullptr) {}

  explicit Reference<T>(T *ptr) : ptr_(new TypedRefCountPointer(ptr)){};

  Reference(Reference<T> &&source) noexcept : ptr_(source.ptr_) {
    source.ptr_ = nullptr;
  }

  Reference(const Reference<T> &source) : ptr_(source.ptr_) { ptr_->add_ref(); }

  Reference &operator=(const Reference<T> &source) {
    if (&source == this || ptr_ == source.ptr_) {
      return *this;
    }
    cleanup(ptr_);
    ptr_ = source.ptr_;
    ptr_->add_ref();
    return *this;
  }

  T *get() const { return ptr_->get(); }

  T *operator->() { return ptr_->get(); }

  ~Reference() {
    // Destroy when leaving scope: no queueing
    cleanup(ptr_);
    ptr_ = nullptr;
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_UTIL_M_REFERENCE_H
