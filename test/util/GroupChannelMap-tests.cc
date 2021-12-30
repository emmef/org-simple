//
// Created by michel on 30-12-21.
//

#include "test-helper.h"
#include <org-simple/util/GroupChannelMap.h>

template <int MAX_GROUPS, int MAX_CHANNELS, int... GR>
using GroupTopology =
    org::simple::util::GroupChannelMap<MAX_GROUPS, MAX_CHANNELS, GR...>;

struct Scenario {
  const int MAX_GROUPS;
  const int MAX_CHANNELS;
  const int GROUP_CHANNELS[16];
};

static constexpr Scenario testG4_C16__2_3_4_1{4, 16, {2, 3, 4, 1, 0}};

BOOST_AUTO_TEST_SUITE(test_org_simple_util_GroupChannelMapTests)

BOOST_AUTO_TEST_CASE(testExploreFirstTopology__G4_C16__2_3_4_1) {
  static constexpr int MAX_GROUPS = 4;
  static constexpr int MAX_CHANNELS = 16;
  static constexpr int GROUP_CHANNELS[] = {2, 3, 4, 1};
  GroupTopology<MAX_GROUPS, MAX_CHANNELS, GROUP_CHANNELS[0], GROUP_CHANNELS[1],
                GROUP_CHANNELS[2], GROUP_CHANNELS[3]>
      topology;

  int groupsForChannels[MAX_CHANNELS];
  int totalGroups = sizeof(GROUP_CHANNELS) / sizeof(int);
  int totalChannels = 0;
  for (int i = 0; i < totalGroups; i++) {
    totalChannels += GROUP_CHANNELS[i];
  }
  int availableGroups = MAX_GROUPS - totalGroups;
  int availableChannels = MAX_CHANNELS - totalChannels;

  BOOST_CHECK_EQUAL(totalGroups, topology.groups());
  BOOST_CHECK_EQUAL(totalChannels, topology.channels());
  BOOST_CHECK_EQUAL(availableGroups, topology.availableGroups());
  BOOST_CHECK_EQUAL(availableChannels, topology.availableChannels());
  for (int i = 0, beginChannel = 0; i < topology.groups(); i++) {
    BOOST_CHECK_EQUAL(GROUP_CHANNELS[i], topology.groupChannels(i));
    BOOST_CHECK_EQUAL(beginChannel, topology.beginChannel(i));
    int endCHannel = beginChannel + GROUP_CHANNELS[i];
    BOOST_CHECK_EQUAL(endCHannel, topology.endChannel(i));
    BOOST_CHECK_EQUAL(endCHannel - 1, topology.lastChannel(i));
    beginChannel += GROUP_CHANNELS[i];
  }

  for (int channel = 0; channel < totalChannels; channel++) {
    groupsForChannels[channel] = 0;
  }
  for (int group = 0, channel = 0; group < totalGroups; group++) {
    for (int j = 0; j < GROUP_CHANNELS[group]; j++, channel++) {
      groupsForChannels[channel] = group;
    }
  }

  for (int channel = 0; channel < totalChannels; channel++) {
    BOOST_CHECK_EQUAL(groupsForChannels[channel], topology.groupOf(channel));
  }
}

BOOST_AUTO_TEST_SUITE_END()
