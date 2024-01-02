#ifndef ORG_SIMPLE_M_SAMPLE_LAYOUT_H
#define ORG_SIMPLE_M_SAMPLE_LAYOUT_H
/*
 * org-simple/SampleLayout.h
 *
 * Added by michel on 2021-02-16
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

#include <org-simple/NumArray.h>

namespace org::simple {

struct SampleLayout {

  /**
   * Moves samples from CHANNEL channel-buffers with each FRAME samples to an
   * interleaved output, where samples are ordered to FRAME and then CHANNEL.
   * For example, if there are two channels with two frames :
   * <pre>
   *   channel 0: 11 12
   *   channel 1: 21 22
   * </pre>
   * The output would look like
   * <pre>
   *   11 21 12 22
   * </pre>
   * @tparam S Type of samples.
   * @tparam CHANNELS The number of channels per frame.
   * @tparam FRAMES The number of frames.
   * @param channel_ptr pointer to CHANNEL pointers to buffers of FRAME samples
   * each. The channel pointer are updated such that this method can be invoked
   * in a loop.
   * @param interleaved_output a consecutive buffer of CHANNEL * FRAME samples.
   * The output pointer is updates such that this method can be invoked in a
   * loop.
   */
  template <typename S, size_t CHANNELS, size_t FRAMES>
  static void channel_buffers_to_interleaved(S const **channel_ptr,
                                             S *&interleaved_output) {
    for (size_t frame = 0; frame < FRAMES; frame++) {
      for (size_t channel = 0; channel < CHANNELS; channel++) {
        *interleaved_output++ = *channel_ptr[channel]++;
      }
    }
  }

  /**
   * Moves samples from CHANNEL channel-buffers with each FRAME samples to a
   * consecutive array of FRAME frames with each CHANNELS samples. For example,
   * if there are two channels with two frames :
   * <pre>
   *   channel 0: 11 12
   *   channel 1: 21 22
   * </pre>
   * The output would look like
   * <pre>
   *   Frame 0: 11 21
   *   Frame 1: 12 22
   * </pre>
   * @tparam S Type of samples.
   * @tparam CHANNELS The number of channels per frame.
   * @tparam FRAMES The number of frames.
   * @param channel_ptr pointer to CHANNEL pointers to buffers of FRAME samples
   * each. The channel pointer are updated such that this method can be invoked
   * in a loop.
   * @param frames a consecutive buffer of FRAME frames with each CHANNELS
   * samples. The buffer is updates such that this method can be invoked in a
   * loop.
   */
  template <typename S, size_t CHANNELS, size_t FRAMES>
  static void channel_buffers_to_frames(S const **channel_ptr,
                                        NumArray<S, CHANNELS> *&frames) {
    for (size_t frame = 0; frame < FRAMES; frame++) {
      NumArray<S, CHANNELS> &output = *frames++;
      for (size_t channel = 0; channel < CHANNELS; channel++) {
        output[channel] = *channel_ptr[channel]++;
      }
    }
  }

  /**
   * Moves samples from an interleaved input, where samples are ordered by frame
   * and then channel, to CHANNEL channel-buffers with each FRAME samples. For
   * example, if the input looks like :
   * <pre>
   *   11 21 12 22
   * </pre>
   * The channel buffers would look like:
   * <pre>
   *   channel 0: 11 12
   *   channel 1: 21 22
   * </pre>
   *
   * @tparam S Type of samples.
   * @tparam CHANNELS The number of channels per frame.
   * @tparam FRAMES The number of frames.
   * @param interleaved_input a consecutive buffer of CHANNEL * FRAME samples.
   * The output pointer is updates such that this method can be invoked in a
   * loop.
   * @param channel_ptr channel_ptr pointer to CHANNEL pointers to buffers of
   * FRAME samples each. The channel pointer are updated such that this method
   * can be invoked in a loop.
   */
  template <typename S, size_t CHANNELS, size_t FRAMES>
  static void interleaved_to_channel_buffers(const S *&interleaved_input,
                                             S **channel_ptr) {
    for (size_t frame = 0; frame < FRAMES; frame++) {
      for (size_t channel = 0; channel < CHANNELS; channel++) {
        *channel_ptr[channel]++ = *interleaved_input++;
      }
    }
  }

  /**
   * Move sampples from a consecutive array of FRAME frames with each CHANNELS
   * samples, to CHANNEL channel-buffers with each FRAME samples. For example,
   * if the input looks like : <pre> Frame 0: 11 21 Frame 1: 12 22
   * </pre>
   * The channel buffer would look like:
   * <pre>
   *   channel 0: 11 12
   *   channel 1: 21 22
   * </pre>
   * @tparam S Type of samples.
   * @tparam CHANNELS The number of channels per frame.
   * @tparam FRAMES The number of frames.
   * @param interleaved_input
   * @param channel_ptr channel_ptr pointer to CHANNEL pointers to buffers of
   * FRAME samples each. The channel pointer are updated such that this method
   * can be invoked in a loop.
   */
  template <typename S, size_t CHANNELS, size_t FRAMES>
  static void interleaved_to_channel_buffers(
      const NumArray<S, CHANNELS> *&interleaved_input, S **channel_ptr) {
    for (size_t frame = 0; frame < FRAMES; frame++) {
      const NumArray<S, CHANNELS> &input = *interleaved_input++;
      for (size_t channel = 0; channel < CHANNELS; channel++) {
        *channel_ptr[channel]++ = input[channel];
      }
    }
  }
};

} // namespace org::simple

#endif // ORG_SIMPLE_M_SAMPLE_LAYOUT_H
