#ifndef ORG_SIMPLE_UTIL_M_PREDICATE_H
#define ORG_SIMPLE_UTIL_M_PREDICATE_H
/*
 * org-simple/util/Predicate.h
 *
 * Added by michel on 2021-12-27
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

#include <type_traits>

namespace org::simple::util {

template <class T> using PredicateFunction = bool (*)(const T &);

template <class T> static bool truePredicateFunction(const T &) { return true; }

template <class T> static bool falsePredicateFunction(const T &) {
  return false;
}

template <typename C, PredicateFunction<C> f1, PredicateFunction<C> f2>
static bool combinedPredicateFunction(const C &c) {
  return f1(c) && f2(c);
}

template <typename C> class Predicate {
public:
  virtual bool test(const C &) const = 0;

  struct Traits {
    template <class X>
    requires(std::is_same_v<bool, decltype(std::declval<X>().test(
                                      (const C &)std::declval<C &>()))>) //
        static constexpr bool substTest(X *) {
      return true;
    }

    template <class X> static constexpr bool substTest(...) { return false; }

    template <class X> static constexpr bool isA() {
      return substTest<const X>(static_cast<const X *>(nullptr));
    }
  };

  static_assert(Traits::template isA<Predicate<C>>());

  static const Predicate &falsePredicate() {
    struct False : public Predicate {
      bool test(const C &) const final { return false; }
    } static f;
    return f;
  }

  static const Predicate &truePredicate() {
    struct True : public Predicate {
      bool test(const C &) const final { return true; }
    } static t;
    return t;
  }
};

struct Predicates {

  template <typename C>
  static auto of(PredicateFunction<C> f) {
    struct P : public Predicate<C> {
      PredicateFunction<C> f;
      bool test(const C &c) const final { return f(c); }
      P(PredicateFunction<C> funk) : f(funk) {}
    };
    return P{ f} ;
  }

  template <typename C>
  static auto of(PredicateFunction<C> f1, PredicateFunction<C> f2) {
    struct P : public Predicate<C> {
      PredicateFunction<C> f1, f2;
      bool test(const C &c) const final { return f1(c) && f2(c); }
      P(PredicateFunction<C> funk1, PredicateFunction<C> funk2) : f1(funk1), f2(funk2) {}
    };
    return P{ f1, f2 } ;
  }

//
//  static auto of(PredicateFunction<C> f1, PredicateFunction<C> f2) {
//    class P : public Predicate {
//      PredicateFunction<C> f1, f2;
//    public:
//      bool test(const C &c) const { return f1(c) && f2(c); }
//    };
//    return P(f1, f2);
//  }

};

template <class P, typename C>
static constexpr bool
    hasPredicateSignature = Predicate<C>::Traits::template isA<P>();

template <typename C, class P1, class P2>
class CombinedPredicate : public Predicate<C> {
  static_assert(hasPredicateSignature<P1, C>);
  static_assert(hasPredicateSignature<P2, C>);

  const P1 &p1;
  const P2 &p2;

public:
  CombinedPredicate(const P1 &predicate1, const P2 &predicate2)
      : p1(predicate1), p2(predicate2) {}

  bool test(const C &c) const override { return p1.test(c) && p2.test(c); }
};

template <class P1, class P2, typename C>
static auto combinedPredicate(const P1 &p1, const P2 &p2) {
  return CombinedPredicate<C, P1, P2>(p1, p2);
}

} // namespace org::simple::util

#endif // ORG_SIMPLE_UTIL_M_PREDICATE_H
