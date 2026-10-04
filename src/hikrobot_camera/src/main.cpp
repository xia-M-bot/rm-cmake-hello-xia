#include <rclcpp/rclcpp.hpp>
#include "../include/hikrobot_camera/camera_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<hikrobot_camera::CameraNode>(options);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
