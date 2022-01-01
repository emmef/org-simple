#ifndef ORG_SIMPLE_UTIL_TEXT_M_ECHO_STREAM_H
#define ORG_SIMPLE_UTIL_TEXT_M_ECHO_STREAM_H
/*
 * org-simple/util/text/EchoStream.h
 *
 * Added by michel on 2022-01-01
 * Copyright (C) 2015-2022 Michel Fleur.
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

#include <org-simple/util/text/InputStream.h>

namespace org::simple::util::text {
/**
 * A stream that uses another stream as a basis and retains the last
 * value returned, which can be obtained by using the \c peek() function.
 * @tparam C The type of character streamed.
 * @tparam S The type of input stream that is echoed, which defaults to {@code
 * InputStream(C)}.
 */
template <typename C, class S = InputStream<C>>
class EchoStream : public InputStream<C> {
  static_assert(hasInputStreamSignature<S, C>);
  enum class State { NoData, HaveData, Repeat };
  S *input = nullptr;
  C lastValue = 0;
  State state;

public:
  /**
   * Create a stream that (initially) uses the provided underlying stream,
   * and is empty per definition if that is \c nullptr.
   * @param stream The underlying stream to use.
   */
  explicit EchoStream(S *stream) : input(stream) {}
  /**
   * Create a stream that is (initially) empty.
   */
  EchoStream() : EchoStream(nullptr) {}

  bool get(C &result) override {
    if (state == State::Repeat) {
      result = lastValue;
      state = State::NoData;
      return true;
    }
    if (input && input->get(lastValue)) {
      result = lastValue;
      state = State::HaveData;
      return true;
    }
    state = State::NoData;
    return false;
  }

  /**
   * Use the provided underlying stream, where \c nullptr means an empty stream.
   * @param stream The new underlying stream.
   */
  void set(S *stream) { input = stream; }
  /**
   * Use the provided underlying stream, where \c nullptr means an empty stream.
   * @param stream The new underlying stream.
   */
  EchoStream &operator=(S *stream) {
    set(stream);
    return *this;
  }
  /**
   * Returns the character of the most recent successful \c get or zero
   * otherwise.
   * @return the last character or 0.
   */
  C peek() const { return state == State::HaveData ? lastValue : 0; }
  /**
   * If the last \c get was successful, repeat that last value.
   */
  void repeat() {
    if (state == State::HaveData) {
      state = State::Repeat;
    }
  }
  /**
   * Reset the state, basically undo the call to \c repeat/
   */
  void reset() {
    state = State::NoData;
    lastValue = 0;
  }
};

} // namespace org::simple::util::text

#endif // ORG_SIMPLE_UTIL_TEXT_M_ECHO_STREAM_H
