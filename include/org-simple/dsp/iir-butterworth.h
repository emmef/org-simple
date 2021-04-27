#ifndef ORG_SIMPLE_IIR_BUTTERWORTH_H
#define ORG_SIMPLE_IIR_BUTTERWORTH_H
/*
 * org-simple/dsp/iir-butterworth.h
 *
 * Added by michel on 2021-04-27
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

#include <org-simple/dsp/iir-coefficients.h>
#include <stdexcept>

namespace org::simple::iir {

static constexpr bool is_valid_bw_order(size_t order) {
  return order >= 1 && order <= 20;
}

static size_t valid_bw_order(size_t order) {
  if (is_valid_bw_order(order)) {
    return order;
  }
  throw std::invalid_argument("valid_bw_order: order must be between 1 and 20.");
}

template <typename... A>
static size_t valid_bw_order(const Coefficients<A...> &coeffs) {
  return valid_bw_order(coeffs.getOrder());
}

static constexpr bool is_supported_bw_type(FilterType type) {
  return type == FilterType::HIGH_PASS || type == FilterType::LOW_PASS;
}

static FilterType supported_bw_type(FilterType type) {
  if (is_supported_bw_type(type)) {
    return type;
  }
  throw std::invalid_argument(
      "supported_bw_type: type must be LOW_PASS or HIGH_PASS.");
}

static double get_wb_high_pass_gain(size_t order, double relative_w0_freq) {
  double r = fabs(relative_w0_freq);
  return r < std::numeric_limits<double>::epsilon()
             ? 0
             : 1.0 / sqrt(1.0 + pow(r, 2.0 * valid_bw_order(order)));
}

static double get_wb_low_pass_gain(size_t order, double relative_w0_freq) {
  double r = fabs(relative_w0_freq);
  return r < std::numeric_limits<double>::epsilon()
      ? 1.0 : 1.0 / sqrt(1.0 + pow(r, -2.0 * valid_bw_order(order)));
}

} // namespace org::simple::iir

#endif // ORG_SIMPLE_IIR_BUTTERWORTH_H
