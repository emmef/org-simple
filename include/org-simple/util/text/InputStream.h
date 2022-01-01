#ifndef ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAM_H
/*
 * org-simple/util/text/InputStream.h
 *
 * Added by michel on 2021-12-15
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

namespace org::simple::util::text {

template <typename C> class InputStream {

public:
  virtual bool get(C &) = 0;
  virtual ~InputStream() = default;

  struct Traits {
    template <class X>
    requires(std::is_same_v<
             bool,
             decltype(std::declval<X>().get(
                 std::declval<C &>()))>) static constexpr bool substGet(X *) {
      return true;
    }
    template <class X> static constexpr bool substGet(...) { return false; }

    template <class X> static constexpr bool isA() {
      return substGet<X>(static_cast<X *>(nullptr));
    }
  };

  static InputStream<C> &empty() {
    class EmptyInputStream : public InputStream<C> {
    public:
      bool get(C &) final { return false; }
    } static instance;
    return instance;
  }
};

template <class S, typename C>
static constexpr bool
    hasInputStreamSignature = InputStream<C>::Traits::template isA<S>();

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_INPUT_STREAM_H
