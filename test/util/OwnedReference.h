#ifndef ORG_SIMPLE_OWNEDREFERENCE_H
#define ORG_SIMPLE_OWNEDREFERENCE_H
/*
 * org-simple/OwnedReference.h
 *
 * Added by michel on 2020-09-24
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
#include <iostream>
#include <mutex>

namespace org::simple::test {

class ReferenceOwner {
  struct Entry {
    void *ptr;
    int id;
  };
  Entry *ptrs_;
  size_t capacity_;
  std::mutex mutex_;
  std::atomic_int ids_ = 1;
  std::atomic_int not_found_ = 0;
  std::atomic_int id_wrong_ = 0;
  std::atomic_int ptr_wrong_ = 0;

  using lock = std::lock_guard<std::mutex>;

public:
  explicit ReferenceOwner(size_t capacity);

  size_t count();

  [[nodiscard]] int not_found() const { return not_found_; }
  [[nodiscard]] int id_wrong() const { return id_wrong_; }
  [[nodiscard]] int ptr_wrong() const { return ptr_wrong_; }
  [[nodiscard]] int errors() const;

  int add_get_id(void *ptr);

  void remove(void *ptr, int id);

  size_t cleanup_count(bool log = true);

  ~ReferenceOwner();
};

class OwnedReference {
  ReferenceOwner &owner_;
  int id_;

public:
  explicit OwnedReference(ReferenceOwner &owner)
      : owner_(owner), id_(owner_.add_get_id(this)) {}

  [[nodiscard]] int id() const { return id_; }

  ~OwnedReference() { owner_.remove(this, id_); }
};

} // namespace org::simple::test

#endif // ORG_SIMPLE_OWNEDREFERENCE_H
