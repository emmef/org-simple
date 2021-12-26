#ifndef ORG_SIMPLE_CORE__DENORMAL_H
#define ORG_SIMPLE_CORE__DENORMAL_H
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

#include <cmath>
#include <limits>

#if defined(__SSE__) &&                                                        \
    (defined(__amd64__) || defined(__x86_64__) || defined(__i386__))
#include <xmmintrin.h>
#define ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE true
#else
#undef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
#endif

namespace org::simple::core {

/**
 * If applicable to the type and enabled and a value is denormal, it will be
 * flushed to zero.
 * @tparam F Type of the value.
 * @param value The value.
 * @return The value or zero when the value is denormal and flushing to zero is
 * enabled.
 */
template <typename F> static constexpr F flush_to_zero(const F value) {
#ifndef ORG_SIMPLE_DENORMAL_IGNORE_FTZ
  if constexpr (std::numeric_limits<F>::is_iec559 &&
                std::numeric_limits<F>::has_denorm) {
    return std::isnormal(value) ? value : 0;
  }
#endif
  return value;
}

/**
 * If #ON then denormal operands of operations will be treated as zero.
 */
enum class TreatAsZero { UNKNOWN, ON, OFF };
/**
 * If #ON then denormal result of operations will be flushed to zero.
 */
enum class FlushToZero { UNKNOWN, ON, OFF };

static FlushToZero get_flush_to_zero() {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
  return _MM_GET_FLUSH_ZERO_MODE() == _MM_FLUSH_ZERO_ON ? FlushToZero::ON
                                                        : FlushToZero::OFF;
#else
  return DenormalFlushToZero::UNKNOWN;
#endif
}

static void set_flush_to_zero(FlushToZero mode) {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
  if (mode != FlushToZero::UNKNOWN) {
    _MM_SET_FLUSH_ZERO_MODE(mode == FlushToZero::ON ? _MM_FLUSH_ZERO_ON
                                                    : _MM_FLUSH_ZERO_OFF);
  }
#endif
}

static TreatAsZero get_treat_as_zero() {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
  return _MM_GET_DENORMALS_ZERO_MODE() == _MM_DENORMALS_ZERO_ON
             ? TreatAsZero::ON
             : TreatAsZero::OFF;
#else
  return DenormalAsZero::UNKNOWN;
#endif
}

static void set_treat_as_zero(TreatAsZero mode) {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
  if (mode != TreatAsZero::UNKNOWN) {
    _MM_SET_DENORMALS_ZERO_MODE(mode == TreatAsZero::ON
                                    ? _MM_DENORMALS_ZERO_ON
                                    : _MM_DENORMALS_ZERO_OFF);
  }
#endif
}

class DenormalTreatmentGuard {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
  TreatAsZero treatAs_;
  FlushToZero flush_;
#endif
public:
  DenormalTreatmentGuard(FlushToZero flush = FlushToZero::ON,
                         TreatAsZero treatAs = TreatAsZero::ON) {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
    treatAs_ = get_treat_as_zero();
    flush_ = get_flush_to_zero();
    set_flush_to_zero(flush);
    set_treat_as_zero(treatAs);
#endif
  };

  ~DenormalTreatmentGuard() {
#ifdef ORG_SIMPLE_SSE_INSTRUCTIONS_AVAILABLE
    set_flush_to_zero(flush_);
    set_treat_as_zero(treatAs_);
#endif
  }
};

} // namespace org::simple::core

#endif // ORG_SIMPLE_CORE__DENORMAL_H
