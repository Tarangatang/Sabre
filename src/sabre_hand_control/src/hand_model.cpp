#include "sabre_hand_control/hand_model.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace sabre_hand_control
{

const std::array<std::string, kJointCount> & joint_names()
{
  static const std::array<std::string, kJointCount> names = {
    "sabre_finger_1_rotatory_joint", "sabre_finger_1_flexor_1_joint",
    "sabre_finger_1_flexor_2_joint", "sabre_finger_1_flexor_3_joint",
    "sabre_finger_2_rotatory_joint", "sabre_finger_2_flexor_1_joint",
    "sabre_finger_2_flexor_2_joint", "sabre_finger_2_flexor_3_joint",
    "sabre_finger_3_rotatory_joint", "sabre_finger_3_flexor_1_joint",
    "sabre_finger_3_flexor_2_joint", "sabre_finger_3_flexor_3_joint",
    "sabre_thumb_rotatory_joint", "sabre_thumb_flexor_1_joint",
    "sabre_thumb_flexor_2_joint", "sabre_thumb_flexor_3_joint"};
  return names;
}

HandPosition named_pose(const std::string & name)
{
  if (name == "open") {
    return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.50, 0.0, 0.0, 0.0};
  }
  if (name == "fist") {
    return {0.0, 1.20, 1.30, 1.20, 0.0, 1.20, 1.30, 1.20,
      0.0, 1.20, 1.30, 1.20, 1.10, 0.80, 1.10, 1.10};
  }
  if (name == "pinch") {
    return {-0.10, 0.75, 0.90, 0.65, 0.0, 0.10, 0.10, 0.10,
      0.0, 0.10, 0.10, 0.10, 1.15, 0.55, 0.85, 0.75};
  }
  if (name == "point") {
    return {0.0, 0.0, 0.0, 0.0, 0.0, 1.20, 1.30, 1.20,
      0.0, 1.20, 1.30, 1.20, 1.0, 0.70, 1.0, 0.90};
  }
  if (name == "relax") {
    return {0.0, 0.20, 0.25, 0.20, 0.0, 0.25, 0.30, 0.24,
      0.0, 0.30, 0.36, 0.28, 0.65, 0.15, 0.25, 0.20};
  }
  throw std::invalid_argument("unknown hand pose: " + name);
}

HandPosition clamp_to_limits(const HandPosition & requested)
{
  static constexpr HandPosition lower = {
    -0.47, -0.196, -0.174, -0.227,
    -0.47, -0.196, -0.174, -0.227,
    -0.47, -0.196, -0.174, -0.227,
    0.463, -0.105, -0.189, -0.162};
  static constexpr HandPosition upper = {
    0.47, 1.61, 1.709, 1.618,
    0.47, 1.61, 1.709, 1.618,
    0.47, 1.61, 1.709, 1.618,
    1.396, 1.163, 1.644, 1.719};
  HandPosition result = requested;
  for (std::size_t i = 0; i < result.size(); ++i) {
    result[i] = std::clamp(result[i], lower[i], upper[i]);
  }
  return result;
}

HandPosition merge_joint_command(
  const std::vector<std::string> & names,
  const std::vector<double> & positions,
  const HandPosition & current)
{
  if (names.size() != positions.size()) {
    throw std::invalid_argument("joint name and position counts do not match");
  }

  std::unordered_map<std::string, std::size_t> index;
  for (std::size_t i = 0; i < joint_names().size(); ++i) {
    index.emplace(joint_names()[i], i);
  }

  HandPosition result = current;
  std::unordered_set<std::string> seen;
  for (std::size_t i = 0; i < names.size(); ++i) {
    const auto location = index.find(names[i]);
    if (location == index.end()) {
      throw std::invalid_argument("unknown joint: " + names[i]);
    }
    if (!seen.insert(names[i]).second) {
      throw std::invalid_argument("duplicate joint: " + names[i]);
    }
    result[location->second] = positions[i];
  }
  return clamp_to_limits(result);
}

}  // namespace sabre_hand_control
