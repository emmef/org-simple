#ifndef ORG_SIMPLE_UTIL_DSP_M_GROUP_CHANNEL_MAP_H
#define ORG_SIMPLE_UTIL_DSP_M_GROUP_CHANNEL_MAP_H
/*
 * org-simple/util/dsp/GroupChannelMap.h
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
#include <functional>

namespace org::simple::util::dsp {

template <size_t GROUPS, size_t CHANNELS>
static constexpr bool validGroupAndChannelCounts =
    GROUPS > 0 && CHANNELS >= GROUPS;

template <size_t GROUPS, size_t CHANNELS>
requires(validGroupAndChannelCounts<GROUPS, CHANNELS>) //
    static constexpr bool isValidInitializer(
        const std::initializer_list<size_t> &list,
        size_t *totalChannels = nullptr) {
  if (list.size() > GROUPS) {
    return false;
  }
  size_t accumulated = std::accumulate(list.begin(), list.end(), 0);
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
template <size_t GROUPS, size_t CHANNELS> struct GroupChannelMapData {
  static_assert(validGroupAndChannelCounts<GROUPS, CHANNELS>);
  typedef std::array<size_t, GROUPS> GroupArray;
  typedef std::array<size_t, CHANNELS> ChannelArray;

  /**
   * Contains the number of mapped groups.
   */
  size_t groups;
  /**
   * Contains the total number of channels mapped by all groups.
   */
  size_t channels;
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
   * for (size_t i =
   * mapping.beginChannel[group]; i < mapping.endChannel[group; i++) {}
   */
  GroupArray endChannel;
  /**
   * Contains for each channel the group that it is mapped to.
   */
  ChannelArray groupForChannel;

  bool areValidGroupChannels(
      const std::initializer_list<size_t> &listOfGroupChannels) const {
    return isValidInitializer<GROUPS, CHANNELS>(listOfGroupChannels);
  }

  static bool createFromGroupChannels(
      GroupChannelMapData &result,
      const std::initializer_list<size_t> listOfGroupChannels) {
    size_t channels;
    if (isValidInitializer<GROUPS, CHANNELS>(listOfGroupChannels, &channels)) {
      result.channels = channels;
      result.groups = listOfGroupChannels.size();
      size_t start = 0;
      auto it = listOfGroupChannels.begin();
      for (size_t group = 0; group < result.groups; group++, it++) {
        result.groupChannels[group] = *it;
        result.beginChannel[group] = start;
        start += result.groupChannels[group];
        result.endChannel[group] = start;
      }
      for (size_t channel = 0, group = 0; channel < result.channels;
           channel++) {
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
template <size_t GROUPS, size_t CHANNELS>
struct GroupChannelMap : private GroupChannelMapData<GROUPS, CHANNELS> {
  static_assert(validGroupAndChannelCounts<GROUPS, CHANNELS>);

  static constexpr size_t maxGroups = GROUPS;
  static constexpr size_t maxChannels = CHANNELS;

  const GroupChannelMapData<GROUPS, CHANNELS> *operator->() const {
    return this;
  }

  GroupChannelMap(const std::initializer_list<size_t> list) { assign(list); }

  GroupChannelMap &operator=(const std::initializer_list<size_t> list) {
    assign(list);
  }

  void assign(const std::initializer_list<size_t> list) {
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
template <size_t MAX_GROUPS, size_t MAX_CHANNELS, size_t... GR>
class GroupChannelMapCT {
  static_assert(validGroupAndChannelCounts<MAX_GROUPS, MAX_CHANNELS>);

protected:
  static constexpr size_t groups = 0;
  static constexpr size_t channels = 0;
};

template <size_t MAX_GROUPS, size_t MAX_CHANNELS, size_t GROUP_CHANNELS,
          size_t... GR>
class GroupChannelMapCT<MAX_GROUPS, MAX_CHANNELS, GROUP_CHANNELS, GR...>
    : public GroupChannelMapCT<MAX_GROUPS, MAX_CHANNELS, GR...> {
  static_assert(GROUP_CHANNELS > 0);

  using Relation = GroupChannelMapCT<MAX_GROUPS, MAX_CHANNELS, GR...>;

public:
  /**
   * Contains the number of mapped groups.
   */
  static constexpr size_t groups = 1 + Relation::groups;
  /**
   * Contains the total number of channels mapped by all groups.
   */
  static constexpr size_t channels = GROUP_CHANNELS + Relation::channels;

private:
  static_assert(groups <= MAX_GROUPS);
  static_assert(channels <= MAX_CHANNELS);

  template <size_t TOTAL_CHANNELS, size_t GROUP_INDEX>
  static constexpr size_t calculateGroupOff(size_t channel) {
    const size_t offset = beginChannel[GROUP_INDEX];
    const size_t max = offset + groupChannels[GROUP_INDEX];
    if (channel >= offset && channel < max) {
      return GROUP_INDEX;
    }
    if constexpr (GROUP_INDEX < groups - 1) {
      return calculateGroupOff<TOTAL_CHANNELS, GROUP_INDEX + 1>(channel);
    }
    return -1;
  }

  template <size_t INDEX, size_t COUNT, size_t SUM, size_t... A>
  static constexpr std::array<size_t, COUNT> generateBeginChannels() {
    if constexpr (INDEX > 0) {
      const size_t newSum = SUM + groupChannels[COUNT - INDEX];
      return generateBeginChannels<INDEX - 1, COUNT, newSum, A..., SUM>();
    } else {
      return {A...};
    }
  }

  template <size_t INDEX, size_t COUNT, size_t... A>
  static constexpr std::array<size_t, channels> generateGroupForChannels() {
    if constexpr (INDEX > 0) {
      const size_t newValue = calculateGroupOff<channels, 0>(COUNT - INDEX);
      return generateGroupForChannels<INDEX - 1, COUNT, A..., newValue>();
    } else {
      return {A...};
    }
  }

  template <ssize_t INDEX, size_t... A>
  // The constexpr check is the reason that size_t won't suffice
  static constexpr std::array<size_t, groups> generateEndChannels() {
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
  static constexpr std::array<size_t, groups> groupChannels = {GROUP_CHANNELS,
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
   * for (size_t i =
   * mapping.beginChannel[group]; i < mapping.endChannel[group; i++) {}
   */
  static constexpr auto endChannel = generateEndChannels<groups - 1>();

  static constexpr GroupChannelMap<groups, channels> createData() {
    return GroupChannelMap<groups, channels>(GR...);
  }

  typedef GroupChannelMap<groups, channels> Map;

  template <typename T, class BinaryOperation>
  static void accumulateGroupValue(std::array<T, groups> &groupsValues,
                            const std::array<T, channels> &channelValues,
                            T initialValue,
                            BinaryOperation op) {
    for (int group = 0; group < groups; group++) {
      const auto p = channelValues.begin();
      groupsValues[group] = std::accumulate(p + beginChannel[group],
                                      p + endChannel[group], initialValue, op);
    }
  }

  template <typename T, class Function>
  static void applyGroupValueToChannels(
                                 std::array<T, channels> &channelValues,
                                 const std::array<T, groups> &groupValues,
                                 Function function) {
    for (int group = 0; group < groups; group++) {
      const T &groupValue = groupValues[group];
      auto p = channelValues.begin();
      for (auto v = p + beginChannel[group]; v < p + endChannel[group];
           v++) {
        *v = function(*static_cast<const T*>(v), groupValue);
      }
    }
  }


};
// Look into https://en.wikipedia.org/wiki/Expression_templates

template <typename T, size_t G, size_t C, class BinaryOperation>
void accumulateGroupValue(const GroupChannelMap<G, C> &map,
                          std::array<T, G> &groups,
                          const std::array<T, C> &channels,
                          T initialValue,
                          BinaryOperation op) {
  const auto p = channels.begin();
  for (int group = 0; group < map->groups; group++) {
    groups[group] = std::accumulate(p + map->beginChannel[group],
                                    p + map->endChannel[group], initialValue, op);
  }
}

template <typename T, size_t G, size_t C, class Function>
void applyGroupValueToChannels(const GroupChannelMap<G, C> &map,
                               std::array<T, C> &channels,
                               const std::array<T, G> &groups,
                               Function function) {
  auto p = channels.begin();
  for (int group = 0; group < map->groups; group++) {
    const T &groupValue = groups[group];
    for (auto v = p + map->beginChannel[group]; v < p + map->endChannel[group];
         v++) {
      *v = function(*static_cast<const T*>(v), groupValue);
    }
  }
}

} // namespace org::simple::util::dsp

#endif // ORG_SIMPLE_UTIL_DSP_M_GROUP_CHANNEL_MAP_H
