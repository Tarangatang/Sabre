#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd "${script_dir}/.." && pwd)"

if [[ ! -r /opt/ros/jazzy/setup.bash || ! -r "${project_dir}/install/setup.bash" ]]; then
  echo "SABRE is not built yet. Run ./scripts/install_ubuntu.sh and ./scripts/build.sh first."
  exit 1
fi

source /opt/ros/jazzy/setup.bash
source "${project_dir}/install/setup.bash"
exec ros2 launch sabre_hand_bringup simulation.launch.py demo:=true

