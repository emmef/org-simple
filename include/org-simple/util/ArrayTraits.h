#ifndef ORG_SIMPLE_ARRAYTRAITS_H
#define ORG_SIMPLE_ARRAYTRAITS_H
/*
 * org-simple/ArrayTraits.h
 *
 * Added by michel on 2020-11-02
 * Copyright (C) 2015-2020 Michel Fleur.
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
#include <type_traits>

namespace org::simple::array_traits {

/**
 * Trait that checks if the class \c Array has a type definition named
 * \c value_type and sets the constant \c value accordingly.
 * @tparam Array The class to test.
 */
template <typename Array> class WithValueType {
  /**
   * If \c value_type represents a type, the argument represents a function
   * pointer. A function pointer is more specific than a variable argument list.
   * Passing \c nullptr at the initialization of \c value will therefore use
   * this function and initialize \c value with \c true.
   * @tparam T The class to test for a type definition named \c type_value.
   * @return \c true if this is a possible substitution.
   */
  template <typename T>
  static constexpr bool trySubst(typename T::value_type (*)()) {
    return true;
  }

  /**
   * Fallback function when the more specific function pointer variant above is
   * not available, meaning there is no type definition named \c type_value.
   * @tparam T The class to test for a type definition named \c type_value.
   * @param ... An uninteresting fallback parameter.
   * @return \c false.
   */
  template <typename T> static constexpr bool trySubst(...) { return false; }

public:
  /**
   * Contains \c true if T \c Array has a type definition named \c type_value.
   */
  static constexpr bool value = trySubst<Array>(nullptr);
};

/**
 * Helper class, whose specialization that matches when \c Array has a static,
 * constant boolean member \c has_local_data, defines a function \c value(),
 * that returns the value for \c Array::has_local_data. This class should only
 * be used for substitution deduction in \c HasLocalData.
 * @tparam Array The class to test.
 */
template <typename Array, bool = Array::has_local_data>
struct WithLocalDataSpec {};
/**
 * Specialization of \c WithLocalDataSpec that has \c value() defined.
 * @tparam Array The class that has a boolean \c has_local_data.
 */
template <typename Array> struct WithLocalDataSpec<Array, true> {
  /**
   * @return the value for \c Array::has_local_data.
   */
  static constexpr bool value() { return Array::has_local_data; }
};

/**
 * Trait that checks if the class \c Array has a static boolean member \c
 * has_local_data that has the value \c true and sets \c value accordingly.
 * @tparam Array The class to test.
 */
template <typename Array> class HasLocalData {
  /**
   * If T leads to a specialization of \c WithlocalDataSpec that has a function
   * \c value() defined, this trySubst is valid and has an argument of type \c
   * bool. It there is no such specialization, this function is undefined and
   * will not be chosen for substitution.
   * @tparam T The class to test.
   * @return The value of \c T::has_local_data constant if defined, \c false
   * otherwise.
   */
  template <typename T>
  static constexpr bool trySubst(decltype(WithLocalDataSpec<T>::value())) {
    return WithLocalDataSpec<T>::value();
  }

  /**
   * Fall-back function for substitution.
   * @return \c false.
   */
  template <typename T> static constexpr bool trySubst(...) { return false; }

public:
  /**
   * True if \c Array has a static boolean member \c has_local_data that is also
   * \c true.
   */
  static constexpr bool value = trySubst<Array>(true);
};

/**
 * Helper class, whose specialization that matches when \c Array has a static,
 * constant function \c size() with an integral return type, defines a function
 * \c value() that returns the value of \c Array::size(). In all other cases,
 * the function \c value() is undefined. This class should only be used for
 * a substitution attempt in \c HasNonZeroConstSize.
 * @tparam Array The class to test.
 */
template <typename Array, bool = Array::size() != 0> struct WithConstSize {};
/**
 * Specialization where \c value()  returns \c Array::size().
 * @tparam Array The class that has a static, constant function \c size() with
 * integral return type.
 */
template <typename Array> struct WithConstSize<Array, true> {
  static constexpr bool value() { return true; }
};

/**
 * Helper Trait that checks if \c Array has a static, constant function \c
 * size() with an integral return type and if so, sets \c size to the value of
 * \c Array::size() and \c value to \c true if size is non-zero. In all other
 * cases \c size = 0 and \c value = \c false.
 * @tparam Array The class to test
 */
template <typename Array> class HasNonZeroConstSizeFunction {

  /**
   * This substitution option only works if there is a specialization for
   * \c WithConstSize that works for \c T, meaning \c T has a static, constant
   * function \c size() with an integral return type.
   * @tparam T The class to test
   * @return The value of \c T::size().
   */
  template <typename T>
  static constexpr bool trySubst(decltype(WithConstSize<T>::value())) {
    return true;
  }

  /**
   * Fallback function for substitution.
   * @return \c false.
   */
  template <typename T> static constexpr bool trySubst(...) { return false; }

public:
  /**
   * Has the value of \c Array::size() is that is a constant, static function
   * with an integral return type and zero otherwise.
   */
  static constexpr bool value = trySubst<Array>(true);
};

template <typename Array> class HasSizeFunction {
  template <typename T>
  static constexpr bool trySubst(decltype(std::declval<T>().size())(*)()) {
    return true;
  }
  template <typename T>
  static constexpr bool trySubst(...) {
    return false;
  }
  static constexpr bool hasFunction(size_t (*v)()) {
    return trySubst<Array>(v);
  }
public:
  static constexpr bool value = hasFunction(nullptr);
};

template <typename Array,
    bool hasConstSizeFunction = HasNonZeroConstSizeFunction<Array>::value>
struct AboutSizeFunction {
  static constexpr bool has_static = false;
  static constexpr bool has = HasSizeFunction<Array>::value;
  static constexpr size_t value = 0;
};
template <typename Array>
struct AboutSizeFunction<Array, true> {
  static constexpr bool has_static = std::is_same_v<size_t, decltype(Array::size())>;
  static constexpr bool has = has_static || HasSizeFunction<Array>::value;
  static constexpr size_t value = has_static ? size_t(Array::size()) : 0;

};


} // namespace org::simple::array_traits

#endif // ORG_SIMPLE_ARRAYTRAITS_H
