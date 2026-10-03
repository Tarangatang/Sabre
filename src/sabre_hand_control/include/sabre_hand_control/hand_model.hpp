#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace sabre_hand_control
{

constexpr std::size_t kJointCount = 15;
using HandPosition = std::array<double, kJointCount>;

const std::array<std::string, kJointCount> & joint_names();

// Returns open, fist, pinch, point, or relax. Throws for an unknown pose.
HandPosition named_pose(const std::string & name);

// Clamps all joints to the safe software limits used by this starter model.
HandPosition clamp_to_limits(const HandPosition & requested);

// Maps a partial name/value command onto the current pose. Unknown or duplicate
// names and mismatched vector sizes throw std::invalid_argument.
HandPosition merge_joint_command(
  const std::vector<std::string> & names,
  const std::vector<double> & positions,
  const HandPosition & current);

}  // namespace sabre_hand_control

