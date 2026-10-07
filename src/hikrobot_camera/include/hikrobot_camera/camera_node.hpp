#ifndef HIKROBOT_CAMERA__CAMERA_NODE_HPP_
#define HIKROBOT_CAMERA__CAMERA_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/opencv.hpp>

namespace hikrobot_camera
{
class CameraNode : public rclcpp::Node
{
public:
  explicit CameraNode();
  ~CameraNode() override;
  explicit CameraNode(const rclcpp::NodeOptions & options);

private:
  void timer_callback();

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  void* mvs_handle_=nullptr;

  // 作业要求的可配置参数
  bool mvs_camera_opened_ = false;
  int image_width_;
  int image_height_;
  double publish_fps_;
};
} // namespace hikrobot_camera

#endif // HIKROBOT_CAMERA__CAMERA_NODE_HPP_
