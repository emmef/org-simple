//
// Created by michel on 30-07-21.
//

#include "org-simple/util/dsp/Biquad.h"
#include "boost-unit-tests.h"
#include <vector>

using Sample = float;
using Coefficients = org::simple::util::dsp::BiQuad::Coefficients<Sample>;
using History = org::simple::util::dsp::BiQuad::History<Sample>;
using Butterworth = org::simple::util::dsp::BiQuad::Butterworth;
using Buffer = std::vector<Sample>;

static constexpr size_t samples = 20;

static void generateSignal(Buffer &output, size_t len) {
  std::minstd_rand random;
  output.resize(len);
  for (auto &v : output) {
    v = random();
  }
}

static void runOnBuffer(const Coefficients &coefficients, const Buffer &signal,
                        Buffer &output) {
  output.resize(signal.size());
  History history; // zeroed
  for (size_t i = 0; i < signal.size(); i++) {
    output[i] = coefficients.runAndGet(history, signal[i]);
  }
}

static void runBackwardsOnBuffer(const Coefficients &coefficients,
                                 const Buffer &signal, Buffer &output) {
  output.resize(signal.size());
  History history; // zeroed
  for (ssize_t i = signal.size(); i >= 0; i--) {
    output[i] = coefficients.runAndGet(history, signal[i]);
  }
}

static void runOnBufferWithGet(const Coefficients &coefficients,
                               const Buffer &signal, Buffer &output) {
  output.resize(signal.size());
  History history; // zeroed
  for (size_t i = 0; i < signal.size(); i++) {
    coefficients.run(history, signal[i], output[i]);
  }
}

static void applyToBuffer(const Coefficients &coefficients,
                          const Buffer &signal, Buffer &output) {
  output.resize(signal.size());
  coefficients.apply(signal.data(), output.data(), signal.size());
}

static void applyBackwardsToBuffer(const Coefficients &coefficients,
                                   const Buffer &signal, Buffer &output) {
  output.resize(signal.size());
  coefficients.applyBackwards(signal.data(), output.data(), signal.size());
}

static void applyWithHistoryToBuffer(const Coefficients &coefficients,
                                     const Buffer &signal, Buffer &output) {
  output.resize(signal.size());
  History history; // zeroed
  coefficients.applyWithHistory(signal.data(), output.data(), signal.size(),
                                history);
}

std::ostream &operator<<(std::ostream &out, const Coefficients &c) {
  out << "{\nb0=" << c.b0 << ", b1=" << c.b1 << ", b2=" << c.b2
      << "\n\t\ta1=" << c.a1 << ", a2=" << c.a2 << "\n}" << std::endl;
  return out;
}

BOOST_AUTO_TEST_SUITE(test_audioDsp_Biquad)

BOOST_AUTO_TEST_CASE(test_audioDsp_TestRunSameAsRunWithGet) {
  Buffer signal;
  generateSignal(signal, samples);
  Buffer withRun;
  Buffer other;
  Coefficients coefficients;
  Butterworth::configureLowPass(coefficients, 48000, 1000);

  runOnBuffer(coefficients, signal, withRun);
  runOnBufferWithGet(coefficients, signal, other);

  BOOST_CHECK_EQUAL_COLLECTIONS(withRun.begin(), withRun.end(), other.begin(),
                                other.end());
}

BOOST_AUTO_TEST_CASE(test_audioDsp_TestRunSameAsApply) {
  Buffer signal;
  generateSignal(signal, samples);
  Buffer withRun;
  Buffer other;
  Coefficients coefficients;
  Butterworth::configureLowPass(coefficients, 48000, 1000);

  runOnBuffer(coefficients, signal, withRun);
  applyToBuffer(coefficients, signal, other);

  BOOST_CHECK_EQUAL_COLLECTIONS(withRun.begin(), withRun.end(), other.begin(),
                                other.end());
}

BOOST_AUTO_TEST_CASE(test_audioDsp_TestRunSameAsApplyWithHistory) {
  Buffer signal;
  generateSignal(signal, samples);
  Buffer withRun;
  Buffer other;
  Coefficients coefficients;
  Butterworth::configureLowPass(coefficients, 48000, 1000);

  runOnBuffer(coefficients, signal, withRun);
  applyWithHistoryToBuffer(coefficients, signal, other);

  BOOST_CHECK_EQUAL_COLLECTIONS(withRun.begin(), withRun.end(), other.begin(),
                                other.end());
}

BOOST_AUTO_TEST_CASE(test_audioDsp_TestRunBackwardsSameAsApplyBackwards) {
  Buffer signal;
  generateSignal(signal, samples);
  Buffer withRun;
  Buffer other;
  Coefficients coefficients;
  Butterworth::configureLowPass(coefficients, 48000, 1000);

  runBackwardsOnBuffer(coefficients, signal, withRun);
  applyBackwardsToBuffer(coefficients, signal, other);

  BOOST_CHECK_EQUAL_COLLECTIONS(withRun.begin(), withRun.end(), other.begin(),
                                other.end());
}

BOOST_AUTO_TEST_SUITE_END()
