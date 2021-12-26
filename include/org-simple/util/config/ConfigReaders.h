#ifndef ORG_SIMPLE_UTIL_CONFIG_M_CONFIG_READERS_H
#define ORG_SIMPLE_UTIL_CONFIG_M_CONFIG_READERS_H
/*
 * org-simple/util/config/ConfigReaders.h
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

#include <org-simple/util/config/ConfigException.h>
#include <org-simple/util/text/InputStream.h>

namespace org::simple::util::config {

enum class ReaderResult { Ok, TooLong, NotFound };

template <typename T> class KeyReader {
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
  virtual ReaderResult read(text::InputStream<T> &input) = 0;

  /**
   * Returns the key name if that was successfully read, or throws a
   * std::runtime_error() is it was not.
   * @return The key name.
   */
  virtual const T *getKey() const = 0;

  virtual ~KeyReader() = default;
};

template <typename T> class ValueReader {
public:
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
  virtual ReaderResult read(text::InputStream<T> &input, const T *keyName) = 0;

  virtual ~ValueReader() = default;
};

} // namespace org::simple::util::config

#endif // ORG_SIMPLE_UTIL_CONFIG_M_CONFIG_READERS_H
