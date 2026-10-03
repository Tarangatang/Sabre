#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"
#include "sabre_hand_control/hand_model.hpp"

namespace sabre_hand_control
{

TEST(HandModel, ExposesSixteenJointNames)
{
  EXPECT_EQ(joint_names().size(), 16U);
  EXPECT_EQ(joint_names().front(), "sabre_finger_1_rotatory_joint");
  EXPECT_EQ(joint_names().back(), "sabre_thumb_flexor_3_joint");
}

TEST(HandModel, NamedPosesHaveSafeValues)
{
  for (const auto * name : {"open", "relax", "fist", "pinch", "point"}) {
    const auto pose = named_pose(name);
    EXPECT_EQ(clamp_to_limits(pose), pose) << name;
  }
  EXPECT_THROW(named_pose("not_a_pose"), std::invalid_argument);
}

TEST(HandModel, PartialCommandsAreMergedAndClamped)
{
  const auto merged = merge_joint_command(
    {"sabre_finger_1_flexor_2_joint", "sabre_thumb_rotatory_joint"},
    {2.0, -0.5}, named_pose("open"));
  EXPECT_DOUBLE_EQ(merged[2], 1.709);
  EXPECT_DOUBLE_EQ(merged[12], 0.463);
  EXPECT_DOUBLE_EQ(merged[8], 0.0);
}

TEST(HandModel, InvalidCommandsAreRejected)
{
  const auto open = named_pose("open");
  EXPECT_THROW(
    merge_joint_command({"missing_joint"}, {0.2}, open), std::invalid_argument);
  EXPECT_THROW(
    merge_joint_command({"sabre_thumb_rotatory_joint"}, {}, open), std::invalid_argument);
  EXPECT_THROW(
    merge_joint_command(
      {"sabre_thumb_rotatory_joint", "sabre_thumb_rotatory_joint"}, {0.5, 0.6}, open),
    std::invalid_argument);
}

}  // namespace sabre_hand_control
