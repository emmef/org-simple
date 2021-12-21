#ifndef ORG_SIMPLE_CONFIG_H
#define ORG_SIMPLE_CONFIG_H
/*
 * org-simple/util/Config.h
 *
 * Added by michel on 2021-12-05
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

#include <cstddef>
#include <exception>
#include <org-simple/util/Characters.h>
#include <org-simple/util/InputStream.h>
#include <string>

namespace org::simple::config {

class ParseError : public std::runtime_error {

public:
  explicit ParseError(const std::string &s) : std::runtime_error(s) {}
  explicit ParseError(const char *s) : std::runtime_error(s) {}
  // TODO perhaps add some constructors with more formatting utils
};

template <typename T> class AbstractReader {
public:
  /**
   * Read a key name from the input stream.
   * @param input The input stream to read from.
   * @throws ParserError When key could not be read successfully.
   */
  virtual void read(util::InputStream<T> &input) = 0;

  virtual ~AbstractReader() = default;
};

template <typename T> class AbstractKeyReader : public AbstractReader<T> {
public:
  /**
   * Returns the key name if that was successfully read, or throws a
   * std::runtime_error() is it was not.
   * @return The key name.
   */
  virtual const T *getKey() const = 0;
};


struct Classifier {
  const org::simple::charClass::Unicode &classifier =
      org::simple::charClass::Classifiers::unicode();

  template <typename codePoint>
  bool isKeyCharacter(codePoint c, codePoint assignment) const {
    return c != assignment && classifier.isGraph(c);
  }
  template <typename codePoint> bool isKeyCharacter(codePoint c) const {
    return isKeyCharacter(c, '=');
  }
  template <typename codePoint> bool isValueCharacter(codePoint c) const {
    return (!classifier.isControl(c) || classifier.isBlank(c));
  }
  template <typename codePoint>
  bool isValueCharacter(codePoint c, codePoint commentStart) const {
    return c != commentStart && isValueCharacter(c);
  }
};


} // namespace org::simple::config

#endif // ORG_SIMPLE_CONFIG_H
