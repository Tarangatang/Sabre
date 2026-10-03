#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]] || [[ ! -r /etc/os-release ]]; then
  echo "This installer must run inside Ubuntu 24.04, not directly on macOS."
  echo "On a Mac, create an Ubuntu 24.04 ARM64 virtual machine first."
  exit 1
fi

source /etc/os-release
if [[ "${ID:-}" != "ubuntu" || "${VERSION_CODENAME:-}" != "noble" ]]; then
  echo "SABRE requires Ubuntu 24.04 (Noble). Detected: ${PRETTY_NAME:-unknown Linux}."
  exit 1
fi

sudo apt-get update
sudo apt-get install -y curl locales software-properties-common
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
sudo add-apt-repository universe -y

ros_source_version="$(
  curl -fsSL https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest \
    | sed -n 's/.*"tag_name": "\([^"]*\)".*/\1/p'
)"
if [[ -z "${ros_source_version}" ]]; then
  echo "Could not determine the latest ROS apt-source release."
  exit 1
fi

ros_source_deb="/tmp/ros2-apt-source.deb"
curl -fsSL -o "${ros_source_deb}" \
  "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ros_source_version}/ros2-apt-source_${ros_source_version}.noble_all.deb"
sudo dpkg -i "${ros_source_deb}"

sudo apt-get update
sudo apt-get install -y \
  ros-jazzy-desktop \
  ros-dev-tools \
  ros-jazzy-ros-gz \
  ros-jazzy-gz-ros2-control \
  ros-jazzy-ros2-controllers \
  ros-jazzy-xacro

sudo rosdep init 2>/dev/null || true
rosdep update

echo
echo "ROS 2 Jazzy and Gazebo dependencies are installed."
echo "Next: ./scripts/build.sh"

