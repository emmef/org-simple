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
  static constexpr int groupCount_ = 0;
  static constexpr int totalChannels_ = 0;
};

template <int MAX_GROUPS, int MAX_CHANNELS, int GROUP_CHANNELS, int... GR>
class GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GROUP_CHANNELS, GR...>
    : public GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GR...> {
  using Relation = GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GR...>;

protected:
  static_assert(GROUP_CHANNELS > 0);
  static constexpr int groupCount_ = 1 + Relation::groupCount_;
  static constexpr int totalChannels_ =
      GROUP_CHANNELS + Relation::totalChannels_;
  static_assert(groupCount_ <= MAX_GROUPS);
  static_assert(totalChannels_ <= MAX_CHANNELS);
  static constexpr int channels_ = GROUP_CHANNELS;

  static constexpr int groupChannels_(int group) {
    if (group == 0) {
      return channels_;
    } else if constexpr (groupCount_ > 1) {
      return Relation::groupChannels_(group - 1);
    } else {
      return 0;
    }
  }

  template <int TOTAL_CHANNELS> static constexpr int beginChannel_(int group) {
    if (group == 0) {
      return TOTAL_CHANNELS - totalChannels_;
    }
    if constexpr (groupCount_ > 1) {
      return Relation::template beginChannel_<TOTAL_CHANNELS>(group - 1);
    }
  }

  template <int TOTAL_CHANNELS, int GROUP_INDEX>
  static constexpr int groupOff_(int channel) {
    const int offset = beginChannel_<TOTAL_CHANNELS>(GROUP_INDEX);
    const int max = offset + groupChannels_(GROUP_INDEX);
    if (channel >= offset && channel < max) {
      return GROUP_INDEX;
    }
    if constexpr (GROUP_INDEX < groups() - 1) {
      return groupOff_<TOTAL_CHANNELS, GROUP_INDEX + 1>(channel);
    }
    return -1;
  }

public:
  /**
   * Returns the total number of groups, which are zero-based numbered.
   * @return the total number of groups
   */
  static constexpr int groups() { return groupCount_; };

  /**
   * Returns the total number of channels, which are zero-based numbered.
   * @return the total number of channels
   */
  static constexpr int channels() { return totalChannels_; }

  /**
   * Returns the first channel that belongs to this group (offset).
   * @param group The zero-based group-number.
   * @return the zero-based channel number.
   */
  static constexpr int beginChannel(int group) {
    return beginChannel_<totalChannels_>(group);
  }

  template <int group> static constexpr int beginChannel() {
    return beginChannel(group);
  }

  /**
   * Returns the number of channels in this group.
   * @param group The zero-based group-number.
   * @return the zero-based channel number.
   */
  static constexpr int groupChannels(int group) {
    return groupChannels_(group);
  }

  template <int group> static constexpr int groupChannels() {
    return groupChannels(group);
  }

  /**
   * Returns the number of the channel just past this group. For the last group
   * this is equal to the total number of channels.
   * @param group The zero-based group-number.
   * @return the zero-based channel number.
   */
  static constexpr int endChannel(int group) {
    return beginChannel(group) + groupChannels_(group);
  }

  template <int group> static constexpr int endChannel() {
    return endChannel(group);
  }

  /**
   * Returns the number of the last channel in this group (inclusive).
   * @param group The zero-based group-number.
   * @return the zero-based channel number.
   */
  static constexpr int lastChannel(int group) { return endChannel(group) - 1; }

  template <int group> static constexpr int lastChannel() {
    return lastChannel(group);
  }

  /**
   * Returns to which group this channel belongs.
   * @param index The zero-based channel number.
   * @return the zero-based group-number.
   */
  static constexpr int groupOf(int index) {
    return groupOff_<totalChannels_, 0>(index);
  }

  template <int CHANNEL> static constexpr int groupOf() {
    return groupOf(CHANNEL);
  }

  // For builders

  /**
   * Returns the number of groups that is available to add (for an extended
   * topology, based on this one).
   * @return the number of available groups, which can be zero.
   */
  static constexpr int availableGroups() { return MAX_GROUPS - groupCount_; };

  /**
   * Returns the number channels that is available to add (for an extended
   * topology, based on this one).
   * @return the number of available channels, which can be zero.
   */
  static constexpr int availableChannels() {
    return MAX_CHANNELS - totalChannels_;
  }
};
} // namespace org::simple::util

#endif // ORG_SIMPLE_UTIL_M_GROUP_TOPOLOGY_H
