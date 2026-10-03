#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd "${script_dir}/.." && pwd)"

if [[ ! -r /opt/ros/jazzy/setup.bash ]]; then
  echo "ROS 2 Jazzy is not installed. Run ./scripts/install_ubuntu.sh first."
  exit 1
fi

source /opt/ros/jazzy/setup.bash
cd "${project_dir}"
rosdep install --from-paths src --ignore-src -r -y --rosdistro jazzy
colcon build --symlink-install --event-handlers console_direct+
source install/setup.bash
colcon test --event-handlers console_direct+
colcon test-result --verbose

echo
echo "Build and tests passed. Next: ./scripts/run_demo.sh"

