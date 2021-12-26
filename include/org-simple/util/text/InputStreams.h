#ifndef ORG_SIMPLE_INPUTSTREAMS_H
#define ORG_SIMPLE_INPUTSTREAMS_H
/*
 * org-simple/util/text/InputStreams.h
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

template <typename C, class D> class NonGraphTerminatedFilter : public InputFilter<C> {
  static_assert(std::is_same_v<Ascii, D> || std::is_same_v<Unicode, D>);
  const D &classifier = Classifiers::instance<D>();

public:
  InputFilterResult filter(C &c) final {
    return classifier.isGraph(c) ? InputFilterResult::Ok
                                 : InputFilterResult::Stop;
  }
};

template <typename C> class NewLineTerminatedFilter : public InputFilter<C> {

public:
  InputFilterResult filter(C &c) final {
    return c != '\n' ? InputFilterResult::Ok : InputFilterResult::Stop;
  }
};

template <typename C> class EchoRememberLastInputStream : public InputStream<C> {
  InputStream<C> & input;
  C v = 0;
public:

  EchoRememberLastInputStream(InputStream<C> & stream) : input(stream) {}

  bool get(C &result) final {
    if (input.get(result)) {
      v = result;
      return true;
    }
    return false;
  }

  C &lastValue() { return v; }
  const C &lastValue() const { return v; }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_INPUTSTREAMS_H
