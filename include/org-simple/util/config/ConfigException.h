#ifndef ORG_SIMPLE_CONFIGEXCEPTION_H
#define ORG_SIMPLE_CONFIGEXCEPTION_H
/*
 * org-simple/util/config/ConfigException.h
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

#include <exception>
#include <sstream>

namespace org::simple::util::config {

class ConfigException : public std::exception {
  std::string message;

public:
  explicit ConfigException(const char *s) : std::exception(), message(s) {}

  explicit ConfigException(const std::string &s)
      : std::exception(), message(s) {}

  const char *what() const noexcept override { return message.c_str(); }

};

} // namespace org::simple::util::config

#endif // ORG_SIMPLE_CONFIGEXCEPTION_H
