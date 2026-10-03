# SABRE robotic hand simulation

This repository contains a ROS 2/Gazebo starter simulation for a five-finger,
15-joint robotic hand. The control path uses standard ROS 2 messages and a
`joint_trajectory_controller`, so the same C++ behaviour node can command the
simulator now and a real `ros2_control` hardware interface later.

## Target platform

- Ubuntu 24.04
- ROS 2 Jazzy
- Gazebo Harmonic
- `gz_ros2_control`

ROS 2 Jazzy is not natively supported on macOS. Develop on Ubuntu 24.04 (a VM
is fine on Apple Silicon) or use a Linux ROS workstation.

## Install dependencies

```bash
sudo apt update
sudo apt install \
  ros-jazzy-desktop \
  ros-jazzy-ros-gz \
  ros-jazzy-gz-ros2-control \
  ros-jazzy-ros2-controllers \
  ros-jazzy-xacro \
  python3-colcon-common-extensions
```

## Build

Run from the repository root:

```bash
source /opt/ros/jazzy/setup.bash
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
source install/setup.bash
colcon test --event-handlers console_direct+
colcon test-result --verbose
```

## Run

Terminal 1 launches Gazebo, the hand, and its controllers:

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 launch sabre_hand_bringup simulation.launch.py
```

Terminal 2 can command named poses:

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run sabre_hand_control hand_commander --ros-args -p preset:=fist
ros2 run sabre_hand_control hand_commander --ros-args -p preset:=open
ros2 run sabre_hand_control hand_commander --ros-args -p preset:=pinch
```

Or run a continuous demonstration:

```bash
ros2 run sabre_hand_control hand_demo
```

For programmatic control, publish any subset of joints. The commander retains
the other targets and clamps unsafe angles:

```bash
ros2 topic pub --once /hand/target_joint_states sensor_msgs/msg/JointState \
  "{name: [index_joint_1, index_joint_2, index_joint_3], position: [0.7, 1.0, 0.8]}"
```

## Reusing control code on the physical hand

`hand_commander` only publishes a standard
`trajectory_msgs/msg/JointTrajectory` to
`/hand_controller/joint_trajectory`. It has no Gazebo-specific code. For the
real hand, implement a `ros2_control` hardware plugin exposing these same 15
position-command joints and load the same controller YAML. The behaviour node,
joint names, poses, and command topic then remain unchanged.

The simulated joints are:

- `thumb_joint_1` through `thumb_joint_3`
- `index_joint_1` through `index_joint_3`
- `middle_joint_1` through `middle_joint_3`
- `ring_joint_1` through `ring_joint_3`
- `little_joint_1` through `little_joint_3`

## Useful launch options

```bash
# Start Gazebo paused
ros2 launch sabre_hand_bringup simulation.launch.py paused:=true

# Do not open the Gazebo GUI
ros2 launch sabre_hand_bringup simulation.launch.py gui:=false
```

## Project layout

```text
src/
  sabre_hand_description/  # Xacro/URDF model and Gazebo world
  sabre_hand_control/      # Reusable C++ command and pose library
  sabre_hand_bringup/      # ros2_control config and launch files
tools/smoke_check.py       # Dependency-free project structure/XML checks
```
