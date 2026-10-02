# hik_camera：海康MVS SDK ROS2 Humble功能包
## 功能
1. 枚举、连接海康USB/GigE相机
2. 断线自动重连
3. 采集图像，发布sensor_msgs/Image话题 `/image_raw`
4. ROS参数动态配置曝光、增益、帧率、像素格式
5. 支持仿真模式，无实体相机也能运行

## 环境
Ubuntu22.04 + ROS2 Humble + MVS SDK

## 编译
```bash
cd ~/ros2_ws
colcon build --packages-select hik_camera
source install/setup.zsh
```
## 仿真模式
# 启动图像发布节点
```bash
ros2 run hik_camera hik_camera_node --ros-args -p use_sim:=true
```
# 启动图像订阅节点
```bash
source install/setup.zsh
ros2 run hik_camera image_sub
```
## 真实相机模式
# 启动图像发布节点
```bash
ros2 run hik_camera hik_camera_node --ros-args -p use_sim:=false
```
# 查看话题列表
```bash
ros2 topic list
```
# 查看话题消息
```bash
ros2 topic echo /image_raw