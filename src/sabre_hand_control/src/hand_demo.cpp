#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

#include "sabre_hand_control/hand_model.hpp"

using namespace std::chrono_literals;

namespace sabre_hand_control
{

class HandDemo : public rclcpp::Node
{
public:
  HandDemo()
  : Node("hand_demo")
  {
    publisher_ = create_publisher<sensor_msgs::msg::JointState>(
      "/hand/target_joint_states", rclcpp::QoS(10));
    timer_ = create_wall_timer(3s, [this]() {publish_next_pose();});
    one_shot_ = create_wall_timer(750ms, [this]() {
      publish_next_pose();
      one_shot_->cancel();
    });
  }

private:
  void publish_next_pose()
  {
    const std::string & pose_name = poses_[pose_index_++ % poses_.size()];
    const auto positions = named_pose(pose_name);
    sensor_msgs::msg::JointState command;
    command.header.stamp = now();
    command.name.assign(joint_names().begin(), joint_names().end());
    command.position.assign(positions.begin(), positions.end());
    publisher_->publish(command);
    RCLCPP_INFO(get_logger(), "Commanding pose: %s", pose_name.c_str());
  }

  const std::vector<std::string> poses_{"open", "relax", "fist", "open", "pinch", "point"};
  std::size_t pose_index_{0};
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::TimerBase::SharedPtr one_shot_;
};

}  // namespace sabre_hand_control

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<sabre_hand_control::HandDemo>());
  rclcpp::shutdown();
  return 0;
}

