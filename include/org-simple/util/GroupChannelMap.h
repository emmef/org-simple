#ifndef ORG_SIMPLE_UTIL_M_GROUP_TOPOLOGY_H
#define ORG_SIMPLE_UTIL_M_GROUP_TOPOLOGY_H
/*
 * org-simple/util/GroupTopology.h
 *
 * Added by michel on 2021-12-29
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

#include <array>
#include <utility>

namespace org::simple::util {

/**
 * Defines a topology with a maximum number of \c MAX_GROUPS groups that map a
 * total of \c MAX_SIZE channels consecutively.
 * The topology aims to prepare a lot of logic compile-time.
 * @tparam MAX_GROUPS
 * @tparam MAX_CHANNELS
 * @tparam GR A list with the number of channels for each group.
 */
template <int MAX_GROUPS, int MAX_CHANNELS, int... GR> class GroupChannelMap {
protected:
  static constexpr int groups = 0;
  static constexpr int channels = 0;
};

template <int MAX_GROUPS, int MAX_CHANNELS, int GROUP_CHANNELS, int... GR>
class GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GROUP_CHANNELS, GR...>
    : public GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GR...> {
  using Relation = GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GR...>;
  static_assert(GROUP_CHANNELS > 0);

public:
  /**
   * Contains the number of mapped groups.
   */
  static constexpr int groups = 1 + Relation::groups;
  /**
   * Contains the total number of channels mapped by all groups.
   */
  static constexpr int channels = GROUP_CHANNELS + Relation::channels;

private:
  static_assert(groups <= MAX_GROUPS);
  static_assert(channels <= MAX_CHANNELS);

  template <int TOTAL_CHANNELS, int GROUP_INDEX>
  static constexpr int calculateGroupOff(int channel) {
    const int offset = beginChannel[GROUP_INDEX];
    const int max = offset + groupChannels[GROUP_INDEX];
    if (channel >= offset && channel < max) {
      return GROUP_INDEX;
    }
    if constexpr (GROUP_INDEX < groups - 1) {
      return calculateGroupOff<TOTAL_CHANNELS, GROUP_INDEX + 1>(channel);
    }
    return -1;
  }

  template <int INDEX, int COUNT, int SUM, int... A>
  static constexpr std::array<int, COUNT> generateBeginChannels() {
    if constexpr (INDEX > 0) {
      const int newSum = SUM + groupChannels[COUNT - INDEX];
      return generateBeginChannels<INDEX - 1, COUNT, newSum, A..., SUM>();
    } else {
      return {A...};
    }
  }

  template <int INDEX, int COUNT, int... A>
  static constexpr std::array<int, channels> generateGroupForChannels() {
    if constexpr (INDEX > 0) {
      const int newValue = calculateGroupOff<channels, 0>(COUNT - INDEX);
      return generateGroupForChannels<INDEX - 1, COUNT, A..., newValue>();
    } else {
      return {A...};
    }
  }

  template <int INDEX, int... A>
  static constexpr std::array<int, groups> generateEndChannels() {
    if constexpr (INDEX >= 0) {
      return generateEndChannels<
          INDEX - 1, beginChannel[INDEX] + groupChannels[INDEX], A...>();
    } else {
      return {A...};
    }
  }

public:
  /**
   * Contains the number of channels for each group.
   */
  static constexpr int groupChannels[groups] = {GROUP_CHANNELS, GR...};

  /**
   * Contains the begin-channel for each group.
   */
  static constexpr auto beginChannel =
      generateBeginChannels<groups, groups, 0>();

  /**
   * Contains for each channel the group that it is mapped to.
   */
  static constexpr auto groupForChannel =
      generateGroupForChannels<channels, channels>();

  /**
   * Contains for each group the channel just past the last channel that belongs
   * to it. This can be used in iterations like:
   *
   * for (int i =
   * mapping.beginChannel[group]; i < mapping.endChannel[group; i++) {}
   */
  static constexpr auto endChannel = generateEndChannels<groups - 1>();
};
} // namespace org::simple::util

#endif // ORG_SIMPLE_UTIL_M_GROUP_TOPOLOGY_H
