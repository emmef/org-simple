#ifndef ORG_SIMPLE_CONFIG_H
#define ORG_SIMPLE_CONFIG_H
/*
 * org-simple/Config.h
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
#include <org-simple/util/Characters.h>

namespace org::simple::config {

struct Classifier {
  const org::simple::charClass::Utf8 &classifier =
      org::simple::charClass::Classifiers::utf8();

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

template <typename codePoint> class KeyOrValueChecker {
  const Classifier classifier;

public:
  virtual bool validAt(size_t position, codePoint value) const = 0;

  virtual ~KeyOrValueChecker() = default;

  static const KeyOrValueChecker &defaultInstance() {
    class Def : public KeyOrValueChecker {
      bool validAt(size_t, codePoint value) const final {
        return classifier.template isValueCharacter(value);
      }
    };
    static const Def inst;
    return inst;
  }
};

template <typename codePoint> class AbstractKeyOrValue {
  size_t pos_ = 0;

protected:
  virtual bool addAtPosition(size_t position, codePoint c) = 0;
  [[nodiscard]] virtual const codePoint *getRaw() const = 0;
  virtual void onReset() {}

public:
  virtual const KeyOrValueChecker<codePoint> &checker() const {
    return KeyOrValueChecker<codePoint>::defaultInstance();
  };

  bool add(codePoint c) {
    if (checker().validAt(pos_, c)) {
      if (addAtPosition(pos_, c)) {
        ++pos_;
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] const codePoint *get() const {
    const codePoint *raw = getRaw();
    if (raw != nullptr && raw[pos_] == '\0') {
      return raw;
    }
    return nullptr;
  }

  void reset() {
    pos_ = 0;
    onReset();
  }
};

template <typename codePoint>
class FixedLengthKeyOrValue : public AbstractKeyOrValue<codePoint> {
  codePoint * const data_;
  const size_t length_;
  const KeyOrValueChecker<codePoint> &checker_;

protected:
  bool addAtPosition(size_t position, codePoint c) override {
    if (length_ <= position) {
      return false;
    }
    data_[position] = c;
    data_[position + 1] = 0;
    return true;
  }

  [[nodiscard]] const codePoint *getRaw() const override {
    return data_;
  }

  void onReset() override {}

public:
  FixedLengthKeyOrValue(size_t length,
                        const KeyOrValueChecker<codePoint> &checker)
      : data_(new codePoint[length]), length_(length), checker_(checker) {}

  ~FixedLengthKeyOrValue() { delete[] data_; }

  const KeyOrValueChecker<codePoint> &checker() const override {
    return checker_;
  };
};

} // namespace org::simple::config

#endif // ORG_SIMPLE_CONFIG_H
