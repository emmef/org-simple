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
#include <numeric>

namespace org::simple::util {

template <int GROUPS, int CHANNELS>
static constexpr bool validGroupAndChannelCounts =
    GROUPS > 0 && CHANNELS >= GROUPS;

template <int GROUPS, int CHANNELS>
requires(validGroupAndChannelCounts<GROUPS, CHANNELS>) //
    static constexpr bool isValidInitializer(
        const std::initializer_list<int> &list, int *totalChannels = nullptr) {
  if (list.size() > GROUPS) {
    return false;
  }
  int accumulated = std::accumulate(list.begin(), list.end(), 0);
  if (accumulated > CHANNELS) {
    return false;
  }
  if (totalChannels) {
    *totalChannels = accumulated;
  }
  return true;
}

/**
 * Contains the raw data definition to be used. This should always be obtained
 * via a GroupChannelMapData.
 * @tparam GROUPS
 * @tparam CHANNELS
 */
template <int GROUPS, int CHANNELS> struct GroupChannelMapData {
  static_assert(validGroupAndChannelCounts<GROUPS, CHANNELS>);
  typedef std::array<int, GROUPS> GroupArray;
  typedef std::array<int, CHANNELS> ChannelArray;

  /**
   * Contains the number of mapped groups.
   */
  int groups;
  /**
   * Contains the total number of channels mapped by all groups.
   */
  int channels;
  /**
   * Contains the number of channels for each group.
   */
  GroupArray groupChannels;
  /**
   * Contains the begin-channel for each group.
   */
  GroupArray beginChannel;
  /**
   * Contains for each group the channel just past the last channel that
   * belongs to it. This can be used in iterations like:
   *
   * for (int i =
   * mapping.beginChannel[group]; i < mapping.endChannel[group; i++) {}
   */
  GroupArray endChannel;
  /**
   * Contains for each channel the group that it is mapped to.
   */
  ChannelArray groupForChannel;

  bool areValidGroupChannels(
      const std::initializer_list<int> &listOfGroupChannels) const {
    return isValidInitializer<GROUPS, CHANNELS>(listOfGroupChannels);
  }

  static bool createFromGroupChannels(
      GroupChannelMapData &result,
      const std::initializer_list<int> listOfGroupChannels) {
    int channels;
    if (isValidInitializer<GROUPS, CHANNELS>(listOfGroupChannels, &channels)) {
      result.channels = channels;
      result.groups = listOfGroupChannels.size();
      int start = 0;
      auto it = listOfGroupChannels.begin();
      for (int group = 0; group < result.groups; group++, it++) {
        result.groupChannels[group] = *it;
        result.beginChannel[group] = start;
        start += result.groupChannels[group];
        result.endChannel[group] = start;
      }
      for (int channel = 0, group = 0; channel < result.channels; channel++) {
        if (channel >= result.endChannel[group]) {
          group++;
        }
        result.groupForChannel[channel] = group;
      }
      return true;
    }
    return false;
  }
};

/**
 * Defines a group channel mapping with a maximum number of \c GROUPS groups
 * that map a total of \c CHANNELS channels consecutively.
 * @tparam GROUPS The maximum number of groups, a positive number.
 * @tparam CHANNELS The maximum number of channels, which is equal or bigger
 * than the number of groups.
 */
template <int GROUPS, int CHANNELS>
struct GroupChannelMap : private GroupChannelMapData<GROUPS, CHANNELS> {
  static_assert(validGroupAndChannelCounts<GROUPS, CHANNELS>);

  const GroupChannelMapData<GROUPS, CHANNELS> *operator->() const { return this; }

  GroupChannelMap(const std::initializer_list<int> list) { assign(list); }

  GroupChannelMap &operator=(const std::initializer_list<int> list) {
    assign(list);
  }

  void assign(const std::initializer_list<int> list) {
    if (!this->createFromGroupChannels(*this, list)) {
      throw std::invalid_argument(list.size() > GROUPS
                                      ? "Maximum number of groups exceeded"
                                      : "Maximum number of channels exceeded");
    }
  }

  const GroupChannelMap<GROUPS, CHANNELS> &data() { return *this; }


};

/**
 * Defines a topology with a maximum number of \c MAX_GROUPS groups that map a
 * total of \c MAX_SIZE channels consecutively.
 * The topology aims to prepare a lot of logic compile-time.
 * @tparam MAX_GROUPS
 * @tparam MAX_CHANNELS
 * @tparam GR A list with the number of channels for each group.
 */
template <int MAX_GROUPS, int MAX_CHANNELS, int... GR>
class CompileTimeGenerator {
  static_assert(validGroupAndChannelCounts<MAX_GROUPS, MAX_CHANNELS>);

protected:
  static constexpr int groups = 0;
  static constexpr int channels = 0;
};

template <int MAX_GROUPS, int MAX_CHANNELS, int GROUP_CHANNELS, int... GR>
class CompileTimeGenerator<MAX_GROUPS, MAX_CHANNELS, GROUP_CHANNELS, GR...>
    : public CompileTimeGenerator<MAX_GROUPS, MAX_CHANNELS, GR...> {
  static_assert(GROUP_CHANNELS > 0);

  using Relation = CompileTimeGenerator<MAX_GROUPS, MAX_CHANNELS, GR...>;

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
  static constexpr std::array<int, groups> groupChannels = {GROUP_CHANNELS,
                                                            GR...};

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
   * Contains for each group the channel just past the last channel that
   * belongs to it. This can be used in iterations like:
   *
   * for (int i =
   * mapping.beginChannel[group]; i < mapping.endChannel[group; i++) {}
   */
  static constexpr auto endChannel = generateEndChannels<groups - 1>();

  static constexpr GroupChannelMap<groups, channels> createData() {
    return GroupChannelMap<groups, channels>(GR...);
  }
};

} // namespace org::simple::util

#endif // ORG_SIMPLE_UTIL_M_GROUP_TOPOLOGY_H
