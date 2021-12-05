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

namespace org::simple::util {

struct KeyValueCharacters {
  using Ascii = org::simple::util::Ascii;

  /**
   * Returns \c true if character \c c can be part of a key name in the widest
   * sense. By default, all control characters, including the space, are
   * excluded, as well as the assigment character indicated in \c assignment.
   * @param c The character to check.
   * @param assignment The character used for assignment.
   * @return whether character \c can be used in a key name.
   */
  static constexpr bool isKeyCharacter(char c, char assignment) {
    return !(Ascii::isControlStrict(c) || c == assignment);
  }

  /**
   * Returns \c true if the character \c c can be part of a key name in the
   * widest sense. By default, all control characters, including the space, are
   * excluded, as well as the assigment character '='.
   * @param c The character to check.
   * @return whether character \c can be used in a key name.
   */
  static constexpr bool isKeyCharacter(char c) {
    return isKeyCharacter(c, '=');
  }

  /**
   * Returns \c true when character \c can be part of a value in the widest
   * sense. By default, this is anything that is a whitespace and not a control
   * character.
   * @param c The character to check.
   * @return whether character \c can be used in a value.
   */
  static constexpr bool isValueCharacter(char c) {
    return Ascii::isWhiteSpace(c) || !Ascii::isControlStrict(c);
  }

  /**
   * Returns \c true when character \c can be part of a value in the widest
   * sense. By default, this is anything that is a whitespace and not a control
   * character or the character to start a comment provided in \c commentStart.
   * @param c The character to check.
   * @param commentStart The character that indicates the start of a comment
   * @return whether character \c can be used in a value.
   */
  static constexpr bool isValueCharacter(char c, char commentStart) {
    return Ascii::isWhiteSpace(c) || !Ascii::isControlStrict(c);
  }

  class CharacterChecker {
  public:
    virtual bool isValid(size_t position, char32_t c) {
      return !Ascii ::isControl(c);
    }
    virtual ~CharacterChecker() = default;

    static CharacterChecker *defaultInstance() {
      static CharacterChecker checker;
      return &checker;
    }
  };

  class KeyOrValue {
  public:
    virtual CharacterChecker *checker() {
      return CharacterChecker::defaultInstance();
    }
    virtual bool add(size_t index, char c) = 0;
    virtual const char *get() const = 0;
    virtual void reset();
    virtual bool complete() const = 0;
  };

  class AbstractKeyOrValue : public KeyOrValue {
    size_t position_;

  protected:
    char *name_;
    size_t length_;

    virtual bool handleState(size_t position, char c) {
      return checker()->isValid(position, c);
    }

    AbstractKeyOrValue(char *name, size_t length)
        : name_(name), length_(length), position_(0) {}

  public:
    bool add(size_t index, char c) override {
      if (position_ != index || position_ >= length_) {
        return false;
      }
      if (!handleState(position_, c)) {
        return false;
      }
      name_[position_++] = c;
      name_[position_] = '\0';
      return true;
    }

    const char *get() const override { return name_; }

    void reset() override {
      position_ = 0;
      name_[0] = '\0';
    };

    bool complete() const override {}
  };

  class AbstractUtf8KeyOrValue : public AbstractKeyOrValue {
    Utf8::Reader reader;

  protected:
    virtual bool handleState(size_t position, char c) {
      switch (reader.addGetState(c)) {
      case Utf8::Reader::State::OK:
        return checker()->isValid(position, reader.getValueAndReset());
      case Utf8::Reader::State::READING:
        return true;
      default:
        return false;
      }
    }

    AbstractUtf8KeyOrValue(char *name, size_t length)
        : AbstractKeyOrValue(name, length) {}

  public:
    bool complete() const override {
      return reader.state() == Utf8::Reader::State::OK;
    }
  };

  class DefaultKeyOrValue : public AbstractKeyOrValue {
    CharacterChecker *checker_;

  public:
    DefaultKeyOrValue(size_t length, CharacterChecker *checker =
                                         CharacterChecker::defaultInstance())
        : AbstractKeyOrValue(new char[length + 1], length), checker_() {}

    CharacterChecker *checker() override { return checker_; }

    ~DefaultKeyOrValue() { delete[] this->name_; }
  };

  class DefaultUtf8KeyOrValue : public AbstractUtf8KeyOrValue {
    CharacterChecker *checker_;

  public:
    DefaultUtf8KeyOrValue(
        size_t length,
        CharacterChecker *checker = CharacterChecker::defaultInstance())
        : AbstractUtf8KeyOrValue(new char[length + 1], length), checker_() {}

    CharacterChecker *checker() override { return checker_; }

    ~DefaultUtf8KeyOrValue() { delete[] this->name_; }
  };
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_CONFIG_H
