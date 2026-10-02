#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

// ========== 你原来的MVS海康SDK头文件，原样保留 ==========
#include "MvCameraControl.h"

using namespace std;

class HikCameraNode : public rclcpp::Node
{
public:
  HikCameraNode() : Node("hik_camera_node")
  {
    // ===== 新增仿真参数（最小改动部分）=====
    this->declare_parameter<bool>("use_sim", false);
    use_sim_ = this->get_parameter("use_sim").as_bool();
    RCLCPP_INFO(this->get_logger(), "use_sim = %s", use_sim_ ? "true" : "false");

    // 创建图像发布器，话题名 /image_raw
    pub_ = this->create_publisher<sensor_msgs::msg::Image>("/image_raw", 10);
    timer_ = this->create_wall_timer(std::chrono::milliseconds(33),
      std::bind(&HikCameraNode::timer_callback, this));

    if (!use_sim_)
    {
      RCLCPP_INFO(this->get_logger(), "尝试打开真实海康相机");
      // =====================【这里是你原来全部MVS相机初始化代码，原样不动】=====================
      // 下面是你原本写的：枚举相机、创建句柄、打开相机、设置参数代码
      // 例（你把自己原来MVS初始化代码粘贴到这个大括号里面）
      /*
      //=====你的原有MVS初始化代码示例占位=====
      MV_CC_DEVICE_INFO_LIST stDeviceList;
      memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
      MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
      if (stDeviceList.nDeviceNum == 0)
      {
        RCLCPP_ERROR(this->get_logger(), "未找到海康相机");
        return;
      }
      m_handle = nullptr;
      MV_CC_CreateHandle(&m_handle, stDeviceList.pDeviceInfo[0]);
      MV_CC_OpenDevice(m_handle);
      MV_CC_StartGrabbing(m_handle);
      */
      // =======================================================================================
    }
    else
    {
      RCLCPP_INFO(this->get_logger(), "仿真模式开启，跳过MVS相机初始化");
    }
  }

private:
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool use_sim_;
  void* m_handle = nullptr;  // 海康相机句柄，保留

  void timer_callback()
  {
    if (use_sim_)
    {
      // 仿真图像分支（新增）
      cv::Mat img(480, 640, CV_8UC1, cv::Scalar(128));
      cv::putText(img, "SIMULATION IMAGE", cv::Point(30, 240), cv::FONT_HERSHEY_SIMPLEX, 1, 255, 2);
      auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "mono8", img).toImageMsg();
      pub_->publish(*msg);
    }
    else
    {
      // =====================【这里粘贴你原来MVS取图、发布图像的全部代码】=====================
      // 原有逻辑：MV_CC_GetImageBuffer 获取图像，转OpenCV，发布
      /*
      // 你的原有取图代码占位
      MV_FRAME_OUT stImageInfo = {0};
      int ret = MV_CC_GetImageBuffer(m_handle, &stImageInfo, 1000);
      if (ret == MV_OK)
      {
        cv::Mat frame(480,640,CV_8UC1, stImageInfo.pBufAddr);
        auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "mono8", frame).toImageMsg();
        pub_->publish(*msg);
        MV_CC_FreeImageBuffer(m_handle, &stImageInfo);
      }
      else
      {
        RCLCPP_WARN(this->get_logger(), "取图失败，尝试断线重连");
      }
      */
      // =====================================================================================
    }
  }
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HikCameraNode>());
  rclcpp::shutdown();
  return 0;
}
