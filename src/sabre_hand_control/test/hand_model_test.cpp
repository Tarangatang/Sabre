#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"
#include "sabre_hand_control/hand_model.hpp"

namespace sabre_hand_control
{

TEST(HandModel, ExposesFifteenJointNames)
{
  EXPECT_EQ(joint_names().size(), 15U);
  EXPECT_EQ(joint_names().front(), "thumb_joint_1");
  EXPECT_EQ(joint_names().back(), "little_joint_3");
}

TEST(HandModel, NamedPosesHaveSafeValues)
{
  for (const auto * name : {"open", "relax", "fist", "pinch", "point"}) {
    for (const double position : named_pose(name)) {
      EXPECT_GE(position, 0.0) << name;
      EXPECT_LE(position, 1.45) << name;
    }
  }
  EXPECT_THROW(named_pose("not_a_pose"), std::invalid_argument);
}

TEST(HandModel, PartialCommandsAreMergedAndClamped)
{
  const auto merged = merge_joint_command(
    {"index_joint_2", "thumb_joint_1"}, {2.0, -0.5}, named_pose("open"));
  EXPECT_DOUBLE_EQ(merged[4], 1.45);
  EXPECT_DOUBLE_EQ(merged[0], 0.0);
  EXPECT_DOUBLE_EQ(merged[8], 0.0);
}

TEST(HandModel, InvalidCommandsAreRejected)
{
  const auto open = named_pose("open");
  EXPECT_THROW(
    merge_joint_command({"missing_joint"}, {0.2}, open), std::invalid_argument);
  EXPECT_THROW(
    merge_joint_command({"thumb_joint_1"}, {}, open), std::invalid_argument);
  EXPECT_THROW(
    merge_joint_command(
      {"thumb_joint_1", "thumb_joint_1"}, {0.2, 0.3}, open),
    std::invalid_argument);
}

}  // namespace sabre_hand_control

