//
// Created by michel on 28-12-21.
//

#include "boost-unit-tests.h"
#include <functional>
#include <org-simple/text/StreamProbe.h>

using namespace org::simple::text;
using CharStreamProbe = StreamProbe<char>;
using LongStreamProbe = StreamProbe<long>;

static constexpr bool expectedTrue = true;
static constexpr bool expectedFalse = false;

BOOST_AUTO_TEST_SUITE(org_simple_util_text_StringSprobe_Tests)

BOOST_AUTO_TEST_CASE(testTraitsSelf) {
  BOOST_CHECK_EQUAL(expectedTrue,
                    CharStreamProbe ::Traits::template isA<CharStreamProbe>());
  BOOST_CHECK_EQUAL(expectedTrue,
                    (hasStreamProbeSignature<StreamProbe<char>, char>));
}

BOOST_AUTO_TEST_CASE(testTraitsDifferentCharType) {
  /*
   * TODO Why this returns true can be a misunderstanding of how the compiler
   * substitutes templates or it is a serious error.
   */
  BOOST_CHECK_EQUAL(expectedTrue,
                    CharStreamProbe::Traits::template isA<LongStreamProbe>());
  BOOST_CHECK_EQUAL(expectedTrue,
                    (hasStreamProbeSignature<LongStreamProbe, char>));
}

BOOST_AUTO_TEST_CASE(testTraitsNoProbeSignatureCorrect) {
  struct NoProbeSignatureCorrect {
    void probe(const char &) { /**/
    }
  };
  BOOST_CHECK_EQUAL(
      expectedTrue,
      CharStreamProbe::Traits::template isA<NoProbeSignatureCorrect>());
  BOOST_CHECK_EQUAL(expectedTrue,
                    (hasStreamProbeSignature<NoProbeSignatureCorrect, char>));
}

BOOST_AUTO_TEST_CASE(testTraitsNoProbeSignatureCorrectDifferentType) {
  struct NoProbeSignatureCorrectDifferentType {
    void probe(const long &) { /**/
    }
  };
  BOOST_CHECK_EQUAL(expectedTrue, CharStreamProbe::Traits::template isA<
                                      NoProbeSignatureCorrectDifferentType>());
  BOOST_CHECK_EQUAL(
      expectedTrue,
      (hasStreamProbeSignature<NoProbeSignatureCorrectDifferentType, char>));
}

BOOST_AUTO_TEST_CASE(testTraitsNoProbeNonConstArgument) {
  struct NoProbeNonConstArgument {
    void probe(char &) { /**/
    }
  };
  BOOST_CHECK_EQUAL(
      expectedFalse,
      CharStreamProbe::Traits::template isA<NoProbeNonConstArgument>());
  BOOST_CHECK_EQUAL(expectedFalse,
                    (hasStreamProbeSignature<NoProbeNonConstArgument, char>));
}

BOOST_AUTO_TEST_CASE(testTraitsNoProbeNoMethod) {
  struct NoProbeNoMethod {};
  BOOST_CHECK_EQUAL(expectedFalse,
                    CharStreamProbe::Traits::template isA<NoProbeNoMethod>());
  BOOST_CHECK_EQUAL(expectedFalse,
                    (hasStreamProbeSignature<NoProbeNoMethod, char>));
}

BOOST_AUTO_TEST_CASE(testTraitsNoProbeWrongReturnType) {
  struct NoProbeNoMethod {
    bool probe(const char &) { return false; }
  };
  BOOST_CHECK_EQUAL(expectedFalse,
                    CharStreamProbe::Traits::template isA<NoProbeNoMethod>());
  BOOST_CHECK_EQUAL(expectedFalse,
                    (hasStreamProbeSignature<NoProbeNoMethod, char>));
}

BOOST_AUTO_TEST_SUITE_END()
