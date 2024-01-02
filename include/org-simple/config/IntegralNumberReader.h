#ifndef ORG_SIMPLE_CONFIG_M_INTEGRAL_NUMBER_READER_H
#define ORG_SIMPLE_CONFIG_M_INTEGRAL_NUMBER_READER_H
/*
 * org-simple/config/IntegralNumberReader.h
 *
 * Added by michel on 2021-12-28
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

#include <org-simple/config/ConfigReaders.h>
#include <org-simple/text/NumberParser.h>

namespace org::simple::config {

static constexpr ReaderResult
toReaderResult(text::NumberParser::Result numberParserResult) {
  switch (numberParserResult) {
  case text::NumberParser::Result::Ok:
    return ReaderResult::Ok;
  case text::NumberParser::Result::OutOfRange:
    return ReaderResult::OutOfRange;
  case text::NumberParser::Result::InputTooLong:
    return ReaderResult::TooLarge;
  case text::NumberParser::Result::UnexpectedCharacter:
  case text::NumberParser::Result::UnexpectedEndOfInput:
  default:
    return ReaderResult::Invalid;
  };
}

template <typename C, typename V>
class IntegralNumberReader : public SingleValueReader<C, V> {
  static_assert(std::is_integral<V>() && !std::is_same<bool, V>());
  static constexpr bool isSigned = std::is_signed<V>();

protected:
  virtual ReaderResult readValue(text::InputStream<C> &input, V &resultValue) {
    return toReaderResult(
        text::NumberParser::readIntegralValueFromStream<C>(input, resultValue));
  }

public:
  template <typename... A>
  IntegralNumberReader(A... arguments)
      : SingleValueReader<C, V>(arguments...) {}
};

} // namespace org::simple::config

#endif // ORG_SIMPLE_CONFIG_M_INTEGRAL_NUMBER_READER_H
