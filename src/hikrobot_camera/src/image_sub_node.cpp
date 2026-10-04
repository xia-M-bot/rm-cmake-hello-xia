#include "hikrobot_camera/image_sub_node.hpp"
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

namespace hikrobot_camera
{
ImageSubNode::ImageSubNode()
: Node("image_sub_node")
{
  sub_ = this->create_subscription<sensor_msgs::msg::Image>(
    "/camera/image_raw",
    10,
    std::bind(&ImageSubNode::image_callback, this, std::placeholders::_1)
  );
  RCLCPP_INFO(this->get_logger(), "Image subscriber node started");
}

void ImageSubNode::image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
{
  try
  {
    cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");
    cv::imshow("Subscribed Image", cv_ptr->image);
    cv::waitKey(1);
  }
  catch (const cv_bridge::Exception & e)
  {
    RCLCPP_ERROR(this->get_logger(), "cv_bridge error: %s", e.what());
  }
}

} // namespace hikrobot_camera
