#!/usr/bin/env bash
set -u

failures=0
if [[ "$(uname -s)" == "Darwin" ]]; then
  echo "[FAIL] macOS detected. The full ROS 2 control simulation requires Ubuntu 24.04."
  echo "       Run this repository inside an Ubuntu ARM64 VM on Apple Silicon."
  failures=$((failures + 1))
else
  echo "[ OK ] Linux detected."
fi

if [[ -r /opt/ros/jazzy/setup.bash ]]; then
  source /opt/ros/jazzy/setup.bash
fi

for command_name in ros2 colcon gz xacro; do
  if command -v "${command_name}" >/dev/null 2>&1; then
    echo "[ OK ] ${command_name}: $(command -v "${command_name}")"
  else
    echo "[FAIL] ${command_name} is not available."
    failures=$((failures + 1))
  fi
done

if [[ -r /opt/ros/jazzy/setup.bash ]]; then
  echo "[ OK ] ROS 2 Jazzy installation found."
else
  echo "[FAIL] /opt/ros/jazzy/setup.bash is missing."
  failures=$((failures + 1))
fi

if [[ "${failures}" -eq 0 ]]; then
  echo "System check passed."
else
  echo "System check found ${failures} problem(s)."
fi
exit "${failures}"
