//
// Created by michel on 30-12-21.
//

#include <org-simple/util/dsp/GroupChannelMap.h>
#include "test-helper.h"

template <size_t MAX_GROUPS, size_t MAX_CHANNELS, size_t... GR>
using Generator =
    org::simple::util::dsp::GroupChannelMapCT<MAX_GROUPS, MAX_CHANNELS, GR...>;
template <size_t MAX_GROUPS, size_t MAX_CHANNELS>
using Mapping =
    org::simple::util::dsp::GroupChannelMap<MAX_GROUPS, MAX_CHANNELS>;

BOOST_AUTO_TEST_SUITE(test_org_simple_util_GroupChannelMapTests)

BOOST_AUTO_TEST_CASE(testExploreFirstTopology__G4_C16__2_3_4_1) {
  static constexpr size_t MAX_GROUPS = 4;
  static constexpr size_t MAX_CHANNELS = 16;
  static constexpr size_t GROUP_CHANNELS[] = {2, 3, 4, 1};
  Generator<MAX_GROUPS, MAX_CHANNELS, GROUP_CHANNELS[0], GROUP_CHANNELS[1],
                GROUP_CHANNELS[2], GROUP_CHANNELS[3]>
      topology;

  std::cout << std::endl;
  size_t groupsForChannels[MAX_CHANNELS];
  size_t totalGroups = sizeof(GROUP_CHANNELS) / sizeof(size_t);
  size_t totalChannels = 0;
  for (size_t i = 0; i < totalGroups; i++) {
    totalChannels += GROUP_CHANNELS[i];
  }
  BOOST_CHECK_EQUAL(totalGroups, topology.groups);
  BOOST_CHECK_EQUAL(totalChannels, topology.channels);
  for (size_t i = 0, beginChannel = 0; i < topology.groups; i++) {
    BOOST_CHECK_EQUAL(GROUP_CHANNELS[i], topology.groupChannels[i]);
    BOOST_CHECK_EQUAL(beginChannel, topology.beginChannel[i]);
    size_t endChannel = beginChannel + GROUP_CHANNELS[i];
    BOOST_CHECK_EQUAL(endChannel, topology.endChannel[i]);
    beginChannel += GROUP_CHANNELS[i];
  }

  for (size_t channel = 0; channel < totalChannels; channel++) {
    groupsForChannels[channel] = 0;
  }
  for (size_t group = 0, channel = 0; group < totalGroups; group++) {
    for (size_t j = 0; j < GROUP_CHANNELS[group]; j++, channel++) {
      groupsForChannels[channel] = group;
    }
  }

  for (size_t channel = 0; channel < totalChannels; channel++) {
    BOOST_CHECK_EQUAL(groupsForChannels[channel], topology.groupForChannel[channel]);
  }
}

BOOST_AUTO_TEST_CASE(testExploreFirstTopology_Instance__G4_C16__2_3_4_1) {
  static constexpr size_t MAX_GROUPS = 4;
  static constexpr size_t MAX_CHANNELS = 16;
  static constexpr size_t GROUP_CHANNELS[] = {2, 3, 4, 1};

  const Mapping<MAX_GROUPS, MAX_CHANNELS> topology {2, 3, 4, 1};

  size_t groupsForChannels[MAX_CHANNELS];
  size_t totalGroups = sizeof(GROUP_CHANNELS) / sizeof(size_t);
  size_t totalChannels = 0;
  for (size_t i = 0; i < totalGroups; i++) {
    totalChannels += GROUP_CHANNELS[i];
  }

  BOOST_CHECK_EQUAL(totalGroups, topology->groups);
  BOOST_CHECK_EQUAL(totalChannels, topology->channels);
  for (size_t i = 0, beginChannel = 0; i < topology->groups; i++) {
    BOOST_CHECK_EQUAL(GROUP_CHANNELS[i], topology->groupChannels[i]);
    BOOST_CHECK_EQUAL(beginChannel, topology->beginChannel[i]);
    size_t endChannel = beginChannel + GROUP_CHANNELS[i];
    BOOST_CHECK_EQUAL(endChannel, topology->endChannel[i]);
    beginChannel += GROUP_CHANNELS[i];
  }

  for (size_t channel = 0; channel < totalChannels; channel++) {
    groupsForChannels[channel] = 0;
  }
  for (size_t group = 0, channel = 0; group < totalGroups; group++) {
    for (size_t j = 0; j < GROUP_CHANNELS[group]; j++, channel++) {
      groupsForChannels[channel] = group;
    }
  }

  for (size_t channel = 0; channel < totalChannels; channel++) {
    BOOST_CHECK_EQUAL(groupsForChannels[channel], topology->groupForChannel[channel]);
  }
}

BOOST_AUTO_TEST_CASE(testExploreFirstTopology_Copy__G4_C16__2_3_4_1) {
  static constexpr size_t MAX_GROUPS = 4;
  static constexpr size_t MAX_CHANNELS = 16;
  static constexpr size_t GROUP_CHANNELS[] = {2, 3, 4, 1};

  const Mapping<MAX_GROUPS, MAX_CHANNELS> original {2, 3, 4, 1};
  auto topology = original;

  size_t groupsForChannels[MAX_CHANNELS];
  size_t totalGroups = sizeof(GROUP_CHANNELS) / sizeof(size_t);
  size_t totalChannels = 0;
  for (size_t i = 0; i < totalGroups; i++) {
    totalChannels += GROUP_CHANNELS[i];
  }

  BOOST_CHECK_EQUAL(totalGroups, topology->groups);
  BOOST_CHECK_EQUAL(totalChannels, topology->channels);
  for (size_t i = 0, beginChannel = 0; i < topology->groups; i++) {
    BOOST_CHECK_EQUAL(GROUP_CHANNELS[i], topology->groupChannels[i]);
    BOOST_CHECK_EQUAL(beginChannel, topology->beginChannel[i]);
    size_t endChannel = beginChannel + GROUP_CHANNELS[i];
    BOOST_CHECK_EQUAL(endChannel, topology->endChannel[i]);
    beginChannel += GROUP_CHANNELS[i];
  }

  for (size_t channel = 0; channel < totalChannels; channel++) {
    groupsForChannels[channel] = 0;
  }
  for (size_t group = 0, channel = 0; group < totalGroups; group++) {
    for (size_t j = 0; j < GROUP_CHANNELS[group]; j++, channel++) {
      groupsForChannels[channel] = group;
    }
  }

  for (size_t channel = 0; channel < totalChannels; channel++) {
    BOOST_CHECK_EQUAL(groupsForChannels[channel], topology->groupForChannel[channel]);
  }

}



BOOST_AUTO_TEST_SUITE_END()
