#include "sabre_hand_control/hand_model.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace sabre_hand_control
{

const std::array<std::string, kJointCount> & joint_names()
{
  static const std::array<std::string, kJointCount> names = {
    "thumb_joint_1", "thumb_joint_2", "thumb_joint_3",
    "index_joint_1", "index_joint_2", "index_joint_3",
    "middle_joint_1", "middle_joint_2", "middle_joint_3",
    "ring_joint_1", "ring_joint_2", "ring_joint_3",
    "little_joint_1", "little_joint_2", "little_joint_3"};
  return names;
}

HandPosition named_pose(const std::string & name)
{
  if (name == "open") {
    return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  }
  if (name == "fist") {
    return {1.1, 1.25, 1.15, 1.25, 1.40, 1.25, 1.25, 1.40, 1.25,
      1.25, 1.40, 1.25, 1.20, 1.35, 1.20};
  }
  if (name == "pinch") {
    return {0.95, 1.05, 0.80, 0.90, 1.00, 0.75, 0.15, 0.20, 0.15,
      0.15, 0.20, 0.15, 0.20, 0.25, 0.20};
  }
  if (name == "point") {
    return {0.85, 1.05, 0.90, 0.0, 0.0, 0.0, 1.25, 1.40, 1.25,
      1.25, 1.40, 1.25, 1.20, 1.35, 1.20};
  }
  if (name == "relax") {
    return {0.20, 0.25, 0.20, 0.18, 0.25, 0.20, 0.22, 0.30, 0.24,
      0.25, 0.34, 0.28, 0.30, 0.40, 0.32};
  }
  throw std::invalid_argument("unknown hand pose: " + name);
}

HandPosition clamp_to_limits(const HandPosition & requested)
{
  HandPosition result = requested;
  for (double & value : result) {
    value = std::clamp(value, 0.0, 1.45);
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

