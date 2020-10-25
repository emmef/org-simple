#ifndef ORG_SIMPLE_CLASSTRAITS_H
#define ORG_SIMPLE_CLASSTRAITS_H
/*
 * org-simple/core/ClassTraits.h
 *
 * Added by michel on 2020-10-25
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

namespace org::simple::core::substitution_helpers {

/*
 * This namespace defines some helper templates that allows the compiler to
 * select the desired template specialization. These specializations can then be
 * used to derive certain properties of classes (and whether they have them).
 */

/**
 * Creates from any type a valid non-type template argument that is always
 * the int constant 0. Only useful in tricks to make the compiler select desired
 * template specializations.
 * @tparam T The type to create a template argument for
 * @return
 */
template <class T>
constexpr int make_dummy_valid_non_type_template_argument(T &&) {
  return 0;
}
/**
 * Exposes a member \c type that represents a function (pointer) without
 * arguments and a return type of \c R.
 * @tparam R The return type of the function signature.
 */
template <class R> struct return_type_function { using type = R (*)(); };

/**
 * Creates a void template argument for a list of integers; each likely the
 * output of make_dummy_valid_non_type_template_argument.
 */
template <int...>
using make_dummy_template_argument_from_substitutable_expression = void;

/**
 * Declares \c value that is \c true if and only if the function \c F with no
 * arguments and return type \c R of class \c T is constexpr.
 * The default, non specialized implementation, has \c value set to \c false.
 * @tparam T The class.
 * @tparam R The return type of the function.
 * @tparam F The function.
 */
template <class T, typename R, typename return_type_function<R>::type F,
          class = void>
struct is_constexpr_member_function {
  static constexpr bool value = false;
};

/**
 * Specialization that is chosen if the function argument can evaluated compile
 * time as an constexpr and therefore is a valid substitution (which is then
 * stripped of all its properties by \c
 * make_dummy_valid_non_type_template_argument and \c
 * make_dummy_template_argument_from_substitutable_expression respectively.
 * @see default, non-specialized implementation for parameter docs.
 */
template <class T, typename R, typename return_type_function<R>::type F>
struct is_constexpr_member_function<
    T, R, F,
    make_dummy_template_argument_from_substitutable_expression<
        make_dummy_valid_non_type_template_argument(F())>> {
  static constexpr bool value = true;
};

} // namespace org::simple::core::substitution_helpers

#define ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK_NAME(class_category,               \
                                                 function_name)                \
  class_category##_has_##function_name
/**
 * Defines a template class name \c class_category_has_function_name with a
 * constant \c class_category_has_function_name::value that is \c true if anf
 * only if the type specified by the first template parameter has a function \c
 * function_name (without arguments) that returns a value that has the type of
 * the second template argument. Both instance and static functions are
 * detected. <p> The detection mechanism uses SFINAE: substitution failure is
 * not an error. Let \c C be the class to be tested and \c R the return value of
 * the member function \c C.function_name(). If and only if the class has a
 * function \c function_name() without argument, a function \c trySubst(\R
 * (*)()) is defined that always returns \c true. There is also a fallback
 * function \c trySubst(...) defined that accepts any argument and always
 * returns \c false. The function \c hasFunction(\R (*ptr)()) forces the
 * compiler to choose a substitution for \c trySubst for the argument \c ptr,
 * which is the more specific \c trySubst if the \c C has the desired function.
 * The reason for the intricate use of function pointer types is that not every
 * type can be initialized with zero or \c nullptr and we do not want automatic
 * widening of types to create false positives.
 */
#define ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK_PREFIX(class_category,             \
                                                   function_name, prefix)      \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE>   \
  class ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK_NAME(class_category,               \
                                                 function_name) {              \
    template <typename prefix##OSCHMFC_X>                                      \
    static constexpr bool trySubst(                                            \
        decltype(std::declval<prefix##OSCHMFC_X>().function_name()) (*)()) {   \
      return true;                                                             \
    }                                                                          \
    template <typename prefix##OSCHMFC_X>                                      \
    static constexpr bool trySubst(...) {                                      \
      return false;                                                            \
    }                                                                          \
    static constexpr bool hasFunction(prefix##OSCHMFC_RVALUE (*v)()) {         \
      return trySubst<prefix##OSCHMFC_CLASS>(v);                               \
    }                                                                          \
                                                                               \
  public:                                                                      \
    static constexpr bool value = hasFunction(nullptr);                        \
  }

#define ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK(class_category, function_name)     \
  ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK_PREFIX(class_category, function_name,    \
                                             none_)

#define ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_NAME(class_category,        \
                                                        function_name)         \
  class_category##_has_static_##function_name

/**
 * Defines a template class name \c class_category_has_function_name with a
 * constant \c class_category_has_function_name::value that is \c true if anf
 * only if the type specified by the first template parameter has a \e static
 * function \c function_name (without arguments) that returns a value that has
 * the type of the second template argument. <p> The detection mechanism works
 * the same as that for ORG_SIMPLE_CLASS_HAS_MEMBER_FUNCTION, except that the
 * decltype works a little different.
 */
#define ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_PREFIX(                     \
    class_category, function_name, prefix)                                     \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE>   \
  class ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_NAME(class_category,        \
                                                        function_name) {       \
    template <typename prefix##OSCHMFC_X>                                      \
    static constexpr bool                                                      \
    trySubst(decltype(prefix##OSCHMFC_X::function_name()) (*)()) {             \
      return true;                                                             \
    }                                                                          \
    template <typename prefix##OSCHMFC_X>                                      \
    static constexpr bool trySubst(...) {                                      \
      return false;                                                            \
    }                                                                          \
    static constexpr bool hasFunction(prefix##OSCHMFC_RVALUE (*v)()) {         \
      return trySubst<prefix##OSCHMFC_CLASS>(v);                               \
    }                                                                          \
                                                                               \
  public:                                                                      \
    static constexpr bool value = hasFunction(nullptr);                        \
  }

#define ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK(class_category,             \
                                                   function_name)              \
  ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_PREFIX(class_category,            \
                                                    function_name, none_)

#define ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_NAME(class_category,            \
                                                    function_name)             \
  about_##class_category##_static_function_##function_name

#define ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_PREFIX(class_category,          \
                                                      function_name, prefix)   \
                                                                               \
  ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_PREFIX(                           \
      about_static_helper_##class_category, function_name, about_static_);     \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE,   \
            bool = ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_NAME(            \
                       about_static_helper_##class_category, function_name) <  \
                   prefix##OSCHMFC_CLASS,                                      \
            prefix##OSCHMFC_RVALUE>                                            \
      ::value >                                                                \
                                                                               \
      struct ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_NAME(class_category,       \
                                                         function_name) {      \
    static constexpr bool exists = false;                                      \
    static constexpr bool is_constexpr = false;                                \
  };                                                                           \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE>   \
  struct ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_NAME(                          \
      class_category,                                                          \
      function_name)<prefix##OSCHMFC_CLASS, prefix##OSCHMFC_RVALUE, true> {    \
    static constexpr bool exists = true;                                       \
    static constexpr bool is_constexpr = ::org::simple::core::                 \
        substitution_helpers::is_constexpr_member_function<                    \
            prefix##OSCHMFC_CLASS, prefix##OSCHMFC_RVALUE,                     \
            prefix##OSCHMFC_CLASS::function_name>::value;                      \
  }

#define ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION(class_category, function_name)  \
  ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_PREFIX(class_category, function_name, \
                                                none_)

#define ORG_SIMPLE_ABOUT_CLASS_FUNCTION_NAME(class_category, function_name)    \
  about_##class_category##_function_##function_name

#define ORG_SIMPLE_ABOUT_CLASS_FUNCTION_PREFIX(class_category, function_name,  \
                                               prefix)                         \
                                                                               \
  ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK(about_helper_##class_category,           \
                                      function_name);                          \
  ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK(about_helper_##class_category,    \
                                             function_name);                   \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE,   \
            int = ORG_SIMPLE_CLASS_HAS_STATIC_FUNCTION_CHECK_NAME(             \
                      about_helper_##class_category, function_name) <          \
                  prefix##OSCHMFC_CLASS,                                       \
            prefix##OSCHMFC_RVALUE>                                            \
      ::value ? 2                                                              \
  : ORG_SIMPLE_CLASS_HAS_FUNCTION_CHECK_NAME(                                  \
        about_helper_##class_category,                                         \
        function_name)<prefix##OSCHMFC_CLASS, prefix##OSCHMFC_RVALUE>::value   \
      ? 1                                                                      \
      : 0 > struct ORG_SIMPLE_ABOUT_CLASS_FUNCTION_NAME(class_category,        \
                                                        function_name) {       \
    static constexpr bool exists = false;                                      \
    static constexpr bool is_static = false;                                   \
    static constexpr bool is_constexpr = false;                                \
  };                                                                           \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE>   \
  struct ORG_SIMPLE_ABOUT_CLASS_FUNCTION_NAME(                                 \
      class_category,                                                          \
      function_name)<prefix##OSCHMFC_CLASS, prefix##OSCHMFC_RVALUE, 1> {       \
                                                                               \
    static constexpr bool exists = true;                                       \
    static constexpr bool is_static = false;                                   \
    static constexpr bool is_constexpr = false;                                \
  };                                                                           \
                                                                               \
  template <typename prefix##OSCHMFC_CLASS, typename prefix##OSCHMFC_RVALUE>   \
  struct ORG_SIMPLE_ABOUT_CLASS_FUNCTION_NAME(                                 \
      class_category,                                                          \
      function_name)<prefix##OSCHMFC_CLASS, prefix##OSCHMFC_RVALUE, 2> {       \
                                                                               \
    ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_PREFIX(class_category,              \
                                                  function_name, loc_);        \
                                                                               \
    typedef ORG_SIMPLE_ABOUT_CLASS_STATIC_FUNCTION_NAME(                       \
        class_category,                                                        \
        function_name)<prefix##OSCHMFC_CLASS, prefix##OSCHMFC_RVALUE> about;   \
    static constexpr bool exists = true;                                       \
    static constexpr bool is_static = true;                                    \
    static constexpr bool is_constexpr = about::is_constexpr;                  \
  }

#define ORG_SIMPLE_ABOUT_CLASS_FUNCTION(class_category, function_name)         \
  ORG_SIMPLE_ABOUT_CLASS_FUNCTION_PREFIX(class_category, function_name, none_)

namespace org::simple::traits {} // namespace org::simple::traits

#endif // ORG_SIMPLE_CLASSTRAITS_H
