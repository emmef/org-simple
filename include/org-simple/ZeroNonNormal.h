#ifndef ORG_SIMPLE_CORE_M_DENORMAL_H
#define ORG_SIMPLE_CORE_M_DENORMAL_H
/*
 * org-simple/core/denormal.h
 *
 * Added by michel on 2021-04-11
 * Copyright (C) 2015-2021 Michel Fleur.
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

#include <limits>
#include <type_traits>

#ifdef __SSE__
#include <pmmintrin.h>
#ifdef _MM_FLUSH_ZERO_ON
#ifdef _MM_DENORMALS_ZERO_ON
namespace org::simple::core {

class [[maybe_unused]] ZeroNonNormal {
  using FlushZeroFlag = decltype(_MM_GET_FLUSH_ZERO_MODE());
  using IsZeroFlag = decltype(_MM_GET_DENORMALS_ZERO_MODE());
  FlushZeroFlag capturedFlushToZero_;
  IsZeroFlag capturedIsZero_;
  bool restore_;

public:
  explicit ZeroNonNormal()
      : capturedFlushToZero_(_MM_GET_FLUSH_ZERO_MODE()),
        capturedIsZero_(_MM_GET_DENORMALS_ZERO_MODE()), restore_(true) {
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  }
  ZeroNonNormal(ZeroNonNormal &&origin) noexcept
      : capturedFlushToZero_(origin.capturedFlushToZero_),
        capturedIsZero_(origin.capturedIsZero_), restore_(true) {
    origin.restore_ = false;
  }
  ~ZeroNonNormal() {
    if (restore_) {
      _MM_SET_FLUSH_ZERO_MODE(capturedFlushToZero_);
      _MM_SET_DENORMALS_ZERO_MODE(capturedIsZero_);
    }
  }
};

} // namespace org::simple::core
#else
namespace org::simple::core {

struct [[maybe_unused]] ZeroNonNormal {
};

} // namespace org::simple::core
#endif
#endif
#endif

#endif // ORG_SIMPLE_CORE_M_DENORMAL_H
