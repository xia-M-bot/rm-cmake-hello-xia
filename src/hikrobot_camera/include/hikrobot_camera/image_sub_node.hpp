#ifndef HIKROBOT_CAMERA__IMAGE_SUB_NODE_HPP
#define HIKROBOT_CAMERA__IMAGE_SUB_NODE_HPP

#if __has_include(<rclcpp/rclcpp.hpp>)
#include <rclcpp/rclcpp.hpp>
#endif

#if __has_include(<rclcpp/subscription.hpp>)
#include <rclcpp/subscription.hpp>
#endif

#if __has_include(<sensor_msgs/msg/image.hpp>)
#include <sensor_msgs/msg/image.hpp>
#endif

#if __has_include(<cv_bridge/cv_bridge.hpp>)
#include <cv_bridge/cv_bridge.hpp>
#endif

#if __has_include(<opencv2/opencv.hpp>)
#include <opencv2/opencv.hpp>
#endif

namespace hikrobot_camera
{
class ImageSubNode : public rclcpp::Node
{
public:
  explicit ImageSubNode();

private:
  void image_callback(const sensor_msgs::msg::Image::SharedPtr msg);
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_;
};
} // namespace hikrobot_camera

#endif
