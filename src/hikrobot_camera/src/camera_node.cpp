#include "../include/hikrobot_camera/camera_node.hpp"
#include <std_msgs/msg/header.hpp>
#include <MvCameraControl.h>
#include <cstring>

namespace hikrobot_camera
{

CameraNode::CameraNode(const rclcpp::NodeOptions & options)
: Node("camera_node", options)
{
    // ======= 声明参数（作业要求，yaml里配置） =======
    this->declare_parameter<bool>("use_simulation", true);
    this->declare_parameter<int>("image_width", 640);
    this->declare_parameter<int>("image_height", 480);
    this->declare_parameter<double>("publish_fps", 30.0);

    // 创建图像发布者，话题 camera/image_raw
    image_pub_ = this->create_publisher<sensor_msgs::msg::Image>("camera/image_raw", 10);

    double publish_fps = this->get_parameter("publish_fps").as_double();
    int period_ms = static_cast<int>(1000.0 / publish_fps);
    timer_ = this->create_wall_timer(std::chrono::milliseconds(period_ms),
        std::bind(&CameraNode::timer_callback, this));

    bool use_simulation = this->get_parameter("use_simulation").as_bool();
    MV_CC_DEVICE_INFO_LIST device_list;
    memset(&device_list, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int ret = MV_OK;

    if (!use_simulation)
    {
        RCLCPP_WARN(this->get_logger(), "Real camera mode, requires MVS SDK");
        ret = MV_CC_EnumDevices(MV_USB_DEVICE, &device_list);
        if (ret != MV_OK || device_list.nDeviceNum == 0)
        {
            RCLCPP_ERROR(this->get_logger(), "No camera found, please check the connection");
            mvs_camera_opened_ = false;
            return;
        }

        mvs_handle_ = nullptr;
        ret = MV_CC_CreateHandle(&mvs_handle_, device_list.pDeviceInfo[0]);
        if (ret != MV_OK)
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to create MVS camera handle: %d", ret);
            mvs_camera_opened_ = false;
            return;
        }

        ret = MV_CC_OpenDevice(mvs_handle_);
        if (ret != MV_OK)
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to open MVS camera: %d", ret);
            mvs_camera_opened_ = false;
            return;
        }

        // 【修改点 1】：强制设置像素格式，防止出现 "Unsupported pixel format: 0"
        ret = MV_CC_SetEnumValue(mvs_handle_, "TriggerMode", MV_TRIGGER_MODE_OFF);
        // 这里强制设置为 Mono8（灰度），如果需要彩色改成 PixelType_Gvsp_BGR8_Packed
        ret = MV_CC_SetEnumValue(mvs_handle_, "PixelFormat", PixelType_Gvsp_Mono8);
        if (ret != MV_OK) {
            RCLCPP_WARN(this->get_logger(), "Failed to set PixelFormat, using camera default.");
        }

        ret = MV_CC_StartGrabbing(mvs_handle_);
        RCLCPP_INFO(this->get_logger(), "StartGrabbing return: %d", ret);
        
        MVCC_INTVALUE stW;
        memset(&stW, 0, sizeof(MVCC_INTVALUE));
        MV_CC_GetIntValue(mvs_handle_, "Width", &stW);
        int img_w = stW.nCurValue;
        MVCC_INTVALUE stH;
        memset(&stH, 0, sizeof(MVCC_INTVALUE));
        MV_CC_GetIntValue(mvs_handle_, "Height", &stH);
        int img_h = stH.nCurValue;

        if (ret == MV_OK)
        {
            mvs_camera_opened_ = true;
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to open MVS camera return: %d", ret);
            mvs_camera_opened_ = false;
        }
    }
    else
    {
        RCLCPP_WARN_ONCE(this->get_logger(), "Simulated camera mode is active");
        mvs_camera_opened_ = false;
    }
}

void CameraNode::timer_callback()
{
    bool use_simulation = this->get_parameter("use_simulation").as_bool();
    int image_width = this->get_parameter("image_width").as_int();
    int image_height = this->get_parameter("image_height").as_int();

    cv::Mat frame;
    if (use_simulation)
    {
        frame = cv::Mat(image_height, image_width, CV_8UC3, cv::Scalar(0, 0, 255));
        cv::putText(frame, "Simulated Image", cv::Point(50, image_height / 2),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    }
    else
    {
        if (!mvs_camera_opened_)
        {
            RCLCPP_ERROR(this->get_logger(), "MVS camera is not opened, cannot capture image");
            return;
        }
        
        // 【修改点 2】：彻底解决段错误！动态获取相机真实的 PayloadSize 来分配内存
        MVCC_INTVALUE_EX payload_size;
        memset(&payload_size, 0, sizeof(MVCC_INTVALUE_EX));
        MV_CC_GetIntValueEx(mvs_handle_, "PayloadSize", &payload_size);
        size_t buffer_size = payload_size.nCurValue;
        if (buffer_size == 0) {
            // 兜底保护，防止SDK未返回时崩溃
            buffer_size = 1440 * 1080 * 3; 
        }
        std::vector<unsigned char> image_buffer(buffer_size);

                MV_FRAME_OUT_INFO_EX frame_info;
        memset(&frame_info, 0, sizeof(MV_FRAME_OUT_INFO_EX));

        int ret = MV_CC_GetOneFrameTimeout(mvs_handle_, image_buffer.data(), 
                                           static_cast<unsigned int>(image_buffer.size()), 
                                           &frame_info, 3000);
        
        if (ret != MV_OK) {
            RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                                 "Failed to get frame from MVS camera: %d", ret);
            return; 
        }

        RCLCPP_INFO_ONCE(this->get_logger(), "Captured: %d x %d, Format: %d", 
                         frame_info.nWidth, frame_info.nHeight, static_cast<int>(frame_info.enPixelType));

        // ================= 【终极防弹方案：手动组装 ROS 图像消息】 =================
        // 彻底不再依赖 cv_bridge
        auto msg = std::make_unique<sensor_msgs::msg::Image>();
        msg->header.stamp = this->get_clock()->now();
        msg->header.frame_id = "camera_link";
        msg->height = frame_info.nHeight;
        msg->width = frame_info.nWidth;

        // 1. 如果相机强制输出 Mono8
        if (frame_info.enPixelType == PixelType_Gvsp_Mono8) {
            msg->encoding = "mono8";
            msg->step = frame_info.nWidth;
            msg->data.assign(image_buffer.data(), image_buffer.data() + (frame_info.nWidth * frame_info.nHeight));
        } 
        // 2. 如果是你当前的 BayerRG8 格式
        else if (frame_info.enPixelType == PixelType_Gvsp_BayerRG8 || 
                 frame_info.enPixelType == PixelType_Gvsp_BayerBG8 ||
                 frame_info.enPixelType == PixelType_Gvsp_BayerGR8 ||
                 frame_info.enPixelType == PixelType_Gvsp_BayerGB8) 
        {
            // 先构建单通道 Mat，随后安全转换为 BGR
            cv::Mat raw_bayer(frame_info.nHeight, frame_info.nWidth, CV_8UC1, image_buffer.data());
            cv::Mat bgr_frame;
            // 【注意】：如果颜色异常（比如偏红/偏蓝），把 BayerRG2BGR 换成 BayerBG2BGR
            cv::cvtColor(raw_bayer, bgr_frame, cv::COLOR_BayerRG2BGR); 

            // 手动拷贝安全的 BGR 数据给 ROS
            msg->encoding = "bgr8";
            msg->step = frame_info.nWidth * 3;
            msg->data.assign(bgr_frame.data, bgr_frame.data + (frame_info.nWidth * frame_info.nHeight * 3));
        }
        // 3. 如果相机直接输出 BGR8
        else if (frame_info.enPixelType == PixelType_Gvsp_BGR8_Packed) 
        {
            msg->encoding = "bgr8";
            msg->step = frame_info.nWidth * 3;
            msg->data.assign(image_buffer.data(), image_buffer.data() + (frame_info.nWidth * frame_info.nHeight * 3));
        }
        // 4. 其他未知格式直接丢弃
        else 
        {
            RCLCPP_ERROR_ONCE(this->get_logger(), "Unsupported pixel format: %d", static_cast<int>(frame_info.enPixelType));
            return; 
        }

        // 发布消息
        image_pub_->publish(std::move(msg));
    }
}

CameraNode::~CameraNode()
{
    if (mvs_handle_)
    {
        MV_CC_StopGrabbing(mvs_handle_);
        MV_CC_CloseDevice(mvs_handle_);
        MV_CC_DestroyHandle(mvs_handle_);
    }
}

} // namespace hikrobot_camera