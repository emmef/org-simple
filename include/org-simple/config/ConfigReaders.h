#ifndef ORG_SIMPLE_CONFIG_M_CONFIG_READERS_H
#define ORG_SIMPLE_CONFIG_M_CONFIG_READERS_H
/*
 * org-simple/config/ConfigReaders.h
 *
 * Added by michel on 2021-12-23
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

#include <org-simple/Predicate.h>
#include <org-simple/config/ConfigException.h>
#include <org-simple/text/Characters.h>
#include <org-simple/text/InputStream.h>
#include <org-simple/text/QuoteState.h>

namespace org::simple::config {

template <typename CodePoint> struct ValueReaderClassifier {
  using type = text::Classifiers::defaultType<CodePoint>;
  static auto instance() {
    return text::Classifiers::defaultInstance<CodePoint>();
  }
};

enum class ReaderResult { Ok, NotFound, Invalid, OutOfRange,
  TooLarge, TooMany };

static constexpr const char * readerResultToString(ReaderResult result) {
  switch(result) {
  case ReaderResult::Ok:
    return "Ok";
  case ReaderResult::NotFound:
    return "NotFound";
  case ReaderResult::OutOfRange:
    return "OutOfRange";
  case ReaderResult::TooLarge:
    return "TooLarge";
  case ReaderResult::TooMany:
    return "TooMany";
  case ReaderResult::Invalid:
    return "Invalid";
  default:
    return "[unknown]";
  }
}

class AbstractReader {

protected:
  static ReaderResult &readerResult() {
    static thread_local ReaderResult result;
    return result;
  }

public:
  static ReaderResult getReaderResult() {
    return readerResult();
  }
};

template <typename C> class KeyReader : public AbstractReader {
public:
  /**
   * Read a key name from the input stream.
   *
   * <ul>
   * <li>If the input stream is finished and no errors occurred, \c
   * ReaderResult::Ok is returned.</li>
   * <li>If the input stream is not finished and no errors occurred,
   * however, reading more would exceed a maximum length, \c
   * ReaderResult::TooLong is returned.
   * <li>If an error happens, a \c ConfigException is thrown.</li>
   * </ul>
   *
   * @param input The input stream to read from.
   * @throws ConfigException
   */
  virtual ReaderResult read(text::InputStream<C> &input) = 0;

  /**
   * Returns the key name if that was successfully read, or throws a
   * std::runtime_error() is it was not.
   * @return The key name.
   */
  virtual const C *getKey() const = 0;

  virtual ~KeyReader() = default;
};

template <typename C> class ValueReader : public AbstractReader {
  const Predicate<C> *separatorPredicate;

public:
  ValueReader(const Predicate<C> *allowedSeparators)
      : separatorPredicate(allowedSeparators? allowedSeparators : &Predicate<C>::falsePredicate()) {}
  ValueReader() : ValueReader(nullptr){};

  /*!
   * Reads a value from the input stream.
   *
   * <ul>
   * <li>If the specified \c keyName is not found in an implementation that
   * checks for that, \c ReaderResult::NotFound is returned.</li>
   * <li>If the input stream is finished and no errors occurred, \c
   * org::simple::config::ReaderResult::Ok is returned.</li>
   * <li>If the input stream is not finished and no errors occurred,
   * however, reading more would exceed a maximum length, \c
   * ReaderResult::TooLong is returned.
   * <li>If an error happens, a \c ConfigException is thrown.</li>
   * </ul>
   * @param input The input stream to read from.
   * @throws ConfigException
   */
  virtual ReaderResult read(text::TokenizedInputStream<C> &input, const C *keyName) = 0;

  const Predicate<C> &getSeparatorPredicate() const { return *separatorPredicate; }

  virtual ~ValueReader() = default;
};

template <typename C, typename V>
class SingleValueReader : public ValueReader<C> {
public:
  template <typename... A>
  SingleValueReader(A... arguments) : ValueReader<C>(arguments...) {}

  ReaderResult read(text::TokenizedInputStream<C> &input, const C *) override {
    AbstractReader::readerResult() = ReaderResult::Ok;
    V resultValue;
    ReaderResult result = readValue(input, resultValue);
    if (result == ReaderResult::Ok) {
      writeValue(resultValue);
    }
    return result;
  }

protected:
  virtual ReaderResult readValue(text::InputStream<C> &input, V &value) = 0;
  virtual void writeValue(const V &value) = 0;
};

} // namespace org::simple::config

#endif // ORG_SIMPLE_CONFIG_M_CONFIG_READERS_H
