#include <rclcpp/rclcpp.hpp>
#include "hikrobot_camera/image_sub_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<hikrobot_camera::ImageSubNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
