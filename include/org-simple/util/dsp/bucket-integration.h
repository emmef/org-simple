#ifndef ORG_SIMPLE_UTIL_DSP__BUCKET_INTEGRATION_H
#define ORG_SIMPLE_UTIL_DSP__BUCKET_INTEGRATION_H
/*
 * org-simple/util/dsp/bucket-integration.h
 *
 * Added by michel on 2021-12-19
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
namespace org::simple::util::dsp {

// Idea:
// A set of N buckets
// The most recent bucket (that gets the most recent samples)
// inherits the average of the previous most recent bucket
// and its own average is:
// (sum-of-recent-samples + (bucket-size - recent-samples) *  inherited-average)
// / bucket-size;
//
// The total average is then (sum-of-all-but-most-recent-buckets
// + sum-of-recent-samples + (bucket-size - recent-samples) * inherited-average)
// / bucket-size * bucket-count.

// Idea:
// For a given number of integration samples N, start with a bucket size M1 and
// number of buckets M2 where both M1 and M2 are the square root of N and then
// vary M1 down, while compensation with M2, so that the product of M1 and M2
// is as close to N as possible.:
/*
 * struct MinErr {
int m1;
int m2;
double err;

double set(int m1_, int m2_, int N) {
  int prod = m1_ * m2_;
  double err_ = fabs(prod - N) / N;
  if (err_ < err) {
    err = err_;
    m1 = m1_;
    m2 = m2_;
  }
  return err_;
}
};
MinErr getErr(int N) {
MinErr closest = {0, 0, 1};

double sqrM = sqrt(N);
int M1 = floor(sqrM);
int M2 = ceil(sqrM);
int M12 = M1 / 2;

if (closest.set(M1, M2, N) > std::numeric_limits<double>::epsilon()) {
double prevMaxErr = 1;
for (int m1 = M1; m1 > M12; m1--) {
  double maxErr = 0;
  double prevErr = 1;
  for (int m2 = M2; m1 * 3 >= m2; m2++) {
    int prod = m1 * m2;
    double err = closest.set(m1, m2, N);
    if (prod > N) {
      break;
    }
    if (err > prevErr) {
      break;
    }
    if (err > maxErr) {
      maxErr = err;
    }
    prevErr = err;
  }
  if (maxErr > prevMaxErr) {
    break;
  }
  prevMaxErr = maxErr;
}
}
return closest;
}
void makeProduct(int N) {
MinErr closest = getErr(N);

std::cout << N << " samples approached by " <<
    (closest.m1 * closest.m2) << " samples (err=" <<
    closest.err << ") with " << closest.m1 << " buckets of " <<
    closest.m2 << std::endl;
}

 */



} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_UTIL_DSP__BUCKET_INTEGRATION_H
