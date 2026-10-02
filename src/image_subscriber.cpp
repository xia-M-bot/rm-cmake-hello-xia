#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

class ImageSubscriberNode : public rclcpp::Node
{
public:
    ImageSubscriberNode() : Node("image_subscriber")
    {
        // 创建图像订阅器，订阅 /image_raw
        sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/image_raw",
            10,
            std::bind(&ImageSubscriberNode::image_callback, this, std::placeholders::_1)
        );
        RCLCPP_INFO(this->get_logger(), "图像订阅节点启动，等待 /image_raw 图像消息");
    }

private:
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_;

    void image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        try
        {
            // ROS图像消息转OpenCV图像
            auto cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::MONO8);
            cv::imshow("Subscriber Image Window", cv_ptr->image);
            cv::waitKey(1);
        }
        catch (cv_bridge::Exception & e)
        {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge 转换失败: %s", e.what());
        }
    }
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ImageSubscriberNode>());
    rclcpp::shutdown();
    cv::destroyAllWindows();
    return 0;
}
