#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "trajectory_msgs/msg/joint_trajectory.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

#include "sabre_hand_control/hand_model.hpp"

using namespace std::chrono_literals;

namespace sabre_hand_control
{

class HandCommander : public rclcpp::Node
{
public:
  HandCommander()
  : Node("hand_commander"), target_(named_pose("open"))
  {
    trajectory_duration_ = declare_parameter<double>("trajectory_duration", 1.0);
    const auto preset = declare_parameter<std::string>("preset", "");
    const auto exit_after_preset = declare_parameter<bool>("exit_after_preset", true);
    if (trajectory_duration_ <= 0.0) {
      throw std::invalid_argument("trajectory_duration must be positive");
    }

    publisher_ = create_publisher<trajectory_msgs::msg::JointTrajectory>(
      "/hand_controller/joint_trajectory", rclcpp::QoS(10));
    subscription_ = create_subscription<sensor_msgs::msg::JointState>(
      "/hand/target_joint_states", rclcpp::QoS(10),
      [this](sensor_msgs::msg::JointState::ConstSharedPtr message) {
        try {
          target_ = merge_joint_command(message->name, message->position, target_);
          publish(target_);
        } catch (const std::exception & error) {
          RCLCPP_ERROR(get_logger(), "Rejected hand command: %s", error.what());
        }
      });

    if (!preset.empty()) {
      target_ = clamp_to_limits(named_pose(preset));
      startup_timer_ = create_wall_timer(500ms, [this]() {
        publish(target_);
        startup_timer_->cancel();
      });
      if (exit_after_preset) {
        shutdown_timer_ = create_wall_timer(1s, [this]() {
          shutdown_timer_->cancel();
          rclcpp::shutdown();
        });
      }
    }

    RCLCPP_INFO(
      get_logger(),
      "Ready: listening on /hand/target_joint_states and publishing to the hand controller");
  }

private:
  void publish(const HandPosition & target)
  {
    trajectory_msgs::msg::JointTrajectory command;
    command.header.stamp = now();
    command.joint_names.assign(joint_names().begin(), joint_names().end());

    trajectory_msgs::msg::JointTrajectoryPoint point;
    point.positions.assign(target.begin(), target.end());
    point.time_from_start = rclcpp::Duration::from_seconds(trajectory_duration_);
    command.points.push_back(std::move(point));
    publisher_->publish(command);
  }

  double trajectory_duration_;
  HandPosition target_;
  rclcpp::Publisher<trajectory_msgs::msg::JointTrajectory>::SharedPtr publisher_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr subscription_;
  rclcpp::TimerBase::SharedPtr startup_timer_;
  rclcpp::TimerBase::SharedPtr shutdown_timer_;
};

}  // namespace sabre_hand_control

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<sabre_hand_control::HandCommander>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("hand_commander"), "%s", error.what());
  }
  rclcpp::shutdown();
  return 0;
}
