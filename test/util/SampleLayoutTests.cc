//
// Created by michel on 16-02-21.
//

#include "test-helper.h"
#include <org-simple/SampleLayout.h>
#include <sstream>
#include <string>

using Layout = org::simple::SampleLayout;

namespace {

static constexpr size_t CHANNELS = 3;
static constexpr size_t FRAMES = 5;
static constexpr size_t SAMPLES = CHANNELS * FRAMES;

std::string print_interleaved(const int *const samples) {
  std::stringstream out;

  for (size_t i = 0; i < SAMPLES; i++) {
    out << " " << samples[i];
  }
  return out.str();
}

std::string
print_interleaved(const org::simple::NumArray<int, CHANNELS> *frames) {
  std::stringstream out;

  for (size_t i = 0; i < FRAMES; i++) {
    for (size_t j = 0; j < CHANNELS; j++) {
      out << " " << frames[i][j];
    }
  }
  return out.str();
}

std::string
print_interleaved(const int ** frames) {
  std::stringstream out;

  for (size_t f = 0; f < FRAMES; f++) {
    for (size_t c = 0; c < CHANNELS; c++) {
      out << " " << frames[c][f];
    }
  }
  return out.str();
}

static const int channel_buffer_samples[CHANNELS][FRAMES] = {
    {11, 12, 13, 14, 15}, {21, 22, 23, 24, 25}, {31, 32, 33, 34, 35}};

const int interleaved_samples[SAMPLES] = {11, 21, 31, 12, 22, 32, 13, 23,
                                          33, 14, 24, 34, 15, 25, 35};

const org::simple::NumArray<int, CHANNELS> sample_frames[FRAMES] =
    {{11, 21, 31}, {12, 22, 32}, {13, 23, 33}, {14, 24, 34}, {15, 25, 35}};

} // namespace

BOOST_AUTO_TEST_SUITE(org_simple_util_SampleLayout)

BOOST_AUTO_TEST_CASE(testChannelBuffersToInterleavedArray) {
  int output[SAMPLES];
  for (size_t i = 0; i < SAMPLES; i++) {
    output[i] = 0;
  }

  int *output_ptr = &output[0];
  const int *channel_start[CHANNELS];
  const int *channel_walk[CHANNELS];
  channel_start[0] = channel_walk[0] = &channel_buffer_samples[0][0];
  channel_start[1] = channel_walk[1] = &channel_buffer_samples[1][0];
  channel_start[2] = channel_walk[2] = &channel_buffer_samples[2][0];
  const int **channel_ptr = &channel_walk[0];
  Layout::channel_buffers_to_interleaved<int, CHANNELS, FRAMES>(channel_ptr,
                                                                output_ptr);

  std::string output_string = print_interleaved(output);
  std::string expected_string = print_interleaved(interleaved_samples);

  BOOST_CHECK_EQUAL(output_string, expected_string);
  for (size_t i = 0; i < CHANNELS; i++) {
    BOOST_CHECK_EQUAL(channel_walk[i] - channel_start[i], FRAMES);
  }
  BOOST_CHECK_EQUAL(output_ptr - &output[0], SAMPLES);
}

BOOST_AUTO_TEST_CASE(testChannelBuffersToFrames) {
  org::simple::NumArray<int, CHANNELS> output[FRAMES];

  for (size_t i = 0; i < FRAMES; i++) {
    output[i].zero();
  }

  org::simple::NumArray<int, CHANNELS> *output_ptr = &output[0];

  const int *channel_start[CHANNELS];
  const int *channel_walk[CHANNELS];
  channel_start[0] = channel_walk[0] = &channel_buffer_samples[0][0];
  channel_start[1] = channel_walk[1] = &channel_buffer_samples[1][0];
  channel_start[2] = channel_walk[2] = &channel_buffer_samples[2][0];
  const int **channel_ptr = &channel_walk[0];
  Layout::channel_buffers_to_frames<int, CHANNELS, FRAMES>(channel_ptr,
                                                                output_ptr);

  std::string output_string = print_interleaved(output);
  std::string expected_string = print_interleaved(interleaved_samples);

  BOOST_CHECK_EQUAL(output_string, expected_string);

  BOOST_CHECK_EQUAL(output_string, expected_string);
  for (size_t i = 0; i < CHANNELS; i++) {
    BOOST_CHECK_EQUAL(channel_walk[i] - channel_start[i], FRAMES);
  }
  BOOST_CHECK_EQUAL(output_ptr - &output[0], FRAMES);
}

BOOST_AUTO_TEST_CASE(testInterleavedArrayToChannelBuffers) {

  int channel_buffer_output[CHANNELS][FRAMES] = {
      { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }};

  int *channel_start[CHANNELS];
  int *channel_walk[CHANNELS];
  channel_start[0] = channel_walk[0] = &channel_buffer_output[0][0];
  channel_start[1] = channel_walk[1] = &channel_buffer_output[1][0];
  channel_start[2] = channel_walk[2] = &channel_buffer_output[2][0];
  int **channel_ptr = &channel_walk[0];

  const int * interleaved_input = &interleaved_samples[0];
  Layout::interleaved_to_channel_buffers<int, CHANNELS, FRAMES>(interleaved_input,
                                                                channel_ptr);

  std::string output_string = print_interleaved(const_cast<const int **>(&channel_start[0]));
  std::string expected_string = print_interleaved(interleaved_samples);

  BOOST_CHECK_EQUAL(output_string, expected_string);
  for (size_t i = 0; i < CHANNELS; i++) {
    BOOST_CHECK_EQUAL(channel_walk[i] - channel_start[i], FRAMES);
  }
  BOOST_CHECK_EQUAL(interleaved_input - interleaved_samples, SAMPLES);
}

BOOST_AUTO_TEST_CASE(testFramesToChannelBuffers) {
  int channel_buffer_output[CHANNELS][FRAMES] = {
      { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }};

  int *channel_start[CHANNELS];
  int *channel_walk[CHANNELS];
  channel_start[0] = channel_walk[0] = &channel_buffer_output[0][0];
  channel_start[1] = channel_walk[1] = &channel_buffer_output[1][0];
  channel_start[2] = channel_walk[2] = &channel_buffer_output[2][0];
  int **channel_ptr = &channel_walk[0];

  const org::simple::NumArray<int, CHANNELS> *input_ptr = &sample_frames[0];

  Layout::interleaved_to_channel_buffers<int, CHANNELS, FRAMES>(input_ptr,
                                                                channel_ptr);
  std::string output_string = print_interleaved(const_cast<const int **>(&channel_start[0]));
  std::string expected_string = print_interleaved(interleaved_samples);

  BOOST_CHECK_EQUAL(output_string, expected_string);
  for (size_t i = 0; i < CHANNELS; i++) {
    BOOST_CHECK_EQUAL(channel_walk[i] - channel_start[i], FRAMES);
  }
  BOOST_CHECK_EQUAL(input_ptr - &sample_frames[0], FRAMES);
}

BOOST_AUTO_TEST_SUITE_END()
