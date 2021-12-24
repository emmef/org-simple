#ifndef ORG_SIMPLE_INPUTSTREAMS_H
#define ORG_SIMPLE_INPUTSTREAMS_H
/*
 * org-simple/util/text/WhitespaceTerminatedStream.h
 *
 * Added by michel on 2021-12-24
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

#include <org-simple/util/text/Characters.h>
#include <org-simple/util/text/InputFilter.h>

namespace org::simple::util::text {

template <typename C, class D> class NonGraphTerminatedFilter {
  static_assert(std::is_same_v<Ascii, D> || std::is_same_v<Unicode, D>);
  static constexpr D &classifier = Classifiers::instance<D>();

public:
  InputFilterResult directFilter(C &c) {
    return classifier.isGraph(c) ? InputFilterResult::Ok
                                 : InputFilterResult::Stop;
  }

  typedef AbstractInputFilter<NonGraphTerminatedFilter, C> Interface;
};

template <typename C, class D, bool resetInputOnStop>
using NonGraphTerminatedInputStream =
    AbstractFilteredInputStream<NonGraphTerminatedFilter<C, D>, C,
                                resetInputOnStop>;

template <typename C> class NewLineTerminatedFilter {

public:
  InputFilterResult directFilter(C &c) {
    return c != '\n' ? InputFilterResult::Ok : InputFilterResult::Stop;
  }

  typedef AbstractInputFilter<NewLineTerminatedFilter, C> Interface;
};

template <typename C, bool resetInputOnStop>
using NewLineTerminatedInputStream =
    AbstractFilteredInputStream<NewLineTerminatedFilter<C>, C,
                                resetInputOnStop>;

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_INPUTSTREAMS_H
