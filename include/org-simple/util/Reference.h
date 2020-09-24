#ifndef ORG_SIMPLE_REFERENCE_H
#define ORG_SIMPLE_REFERENCE_H
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
#include <org-simple/core/attributes.h>
#include <stdexcept>

namespace org::simple {

class UntypedRefCountPointer {
  void *ptr_;
  std::atomic<ptrdiff_t> count_;
  static std::atomic_flag flag_;

protected:
  virtual void destroy_ptr() noexcept = 0;

  static void * not_null(void *p) {
    if (p) {
      return p;
    }
    throw std::invalid_argument("org::simple::UntypedRefCountPointer: nullptr");
  }
public:
  explicit UntypedRefCountPointer(void *ptr) : ptr_(ptr), count_(1) {}

  bool add_ref() noexcept { return count_.fetch_add(1) >= 1; }

  ptrdiff_t del_ref_no_destroy() noexcept { return count_.fetch_sub(1); }

  bool del_ref_get_if_destroyed() noexcept {
    ptrdiff_t previousValue = count_.fetch_sub(1);
    if (previousValue == 1) {
      destroy();
      return true;
    }
    return false;
  }

  void destroy() {
    // At least attempts to use recent state and make memory visible afterwards.
    flag_.test_and_set(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    destroy_ptr();
    std::atomic_thread_fence(std::memory_order_release);
    flag_.clear();
  }

  org_nodiscard void *get_ptr() const noexcept { return ptr_; }

  virtual ~UntypedRefCountPointer() noexcept = default;
};

template <typename T> class TypedRefCountPointer final : public UntypedRefCountPointer {
protected:
  void destroy_ptr() noexcept override { delete static_cast<T *>(get()); }

public:
  explicit TypedRefCountPointer(T *ptr) : UntypedRefCountPointer(ptr){};

  T *get() noexcept { return static_cast<T *>(get_ptr()); }

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

  Reference<T> *operator&() = delete;

  explicit Reference(Reference<T> &&source) noexcept : ptr_(source.ptr_) {
    source.ptr_ = nullptr;
  }

  explicit Reference(const Reference<T> &source) : ptr_(source.ptr_) {
    ptr_->add_ref();
  }

  void operator=(const Reference<T> &source) {
    if (&source == this || ptr_ == source.ptr_) {
      return;
    }
    cleanup(ptr_);
    ptr_ = source.ptr_;
    ptr_->add_ref();
  }

  T *get() const noexcept { return ptr_->get(); }

  T *operator->() noexcept { return ptr_->get(); }

  ~Reference() {
    // Destroy when leaving scope: no queueing
    cleanup(ptr_);
    ptr_ = nullptr;
  }
};

} // namespace org::simple

#endif // ORG_SIMPLE_REFERENCE_H
