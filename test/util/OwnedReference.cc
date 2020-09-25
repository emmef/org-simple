//
// Created by michel on 24-09-20.
//

#include "OwnedReference.h"

namespace org::simple::test {

ReferenceOwner::ReferenceOwner(size_t capacity)
    : ptrs_(new Entry[capacity]), capacity_(capacity) {
  for (size_t i = 0; i < capacity_; i++) {
    ptrs_[i] = {nullptr, 0};
  }
}
size_t ReferenceOwner::count() {
  size_t result = 0;
  lock guard(mutex_);

  for (size_t i = 0; i < capacity_; i++) {
    if (ptrs_[i].ptr) {
      result++;
    }
  }
  return result;
}
int ReferenceOwner::add_get_id(void *ptr) {
  if (!ptr) {
    throw std::invalid_argument("ReferenceOwner::cannot add nullptr");
  }
  lock guard(mutex_);
  for (size_t i = 0; i < capacity_; i++) {
    if (ptrs_[i].ptr == ptr) {
      throw std::runtime_error(
          "anonymous::ReferenceOwner: cannot add duplicate.");
    }
  }
  for (size_t i = 0; i < capacity_; i++) {
    if (!ptrs_[i].ptr) {
      ptrs_[i].ptr = ptr;
      return ptrs_[i].id = ids_.fetch_add(1);
    }
  }
  throw std::runtime_error("anonymous::ReferenceOwner: cannot add: full.");
}
void ReferenceOwner::remove(void *ptr, int id) {
  lock guard(mutex_);
  for (size_t i = 0; i < capacity_; i++) {
    if (ptrs_[i].ptr == ptr) {
      if (ptrs_[i].id == id) {
        ptrs_[i] = {nullptr, 0};
        return;
      }
      id_wrong_.fetch_add(1);
      return;
    } else if (ptrs_[i].id == id) {
      ptr_wrong_.fetch_add(1);
      return;
    }
  }
  not_found_.fetch_add(1);
}
size_t ReferenceOwner::cleanup_count(bool log) {
  size_t result = 0;
  lock guard(mutex_);

  for (size_t i = 0; i < capacity_; i++) {
    if (ptrs_[i].ptr) {
      result++;
      if (log) {
        std::cerr << "ReferenceOwner: Object{id=" << ptrs_[i].id
                  << ", this=" << ptrs_[i].ptr << "}" << std::endl;
      }
    }
  }
  return result;
}
ReferenceOwner::~ReferenceOwner() {
  // explicitly NOT deleting each element, as we have no idea what it is.
  delete[] ptrs_;
}
int ReferenceOwner::errors() const {
  return not_found_ + id_wrong_ + ptr_wrong_;
}
} // namespace org::simple::test
