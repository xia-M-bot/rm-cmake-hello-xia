cmake_minimum_required(VERSION 3.8)
project(hik_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ROS2依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV REQUIRED)

# MVS SDK路径，amd64
include_directories(/opt/MVS/include)
link_directories(/opt/MVS/lib/64)

add_executable(hik_camera_node src/hik_camera_node.cpp)

# 链接库：MVS库 + ROS库
target_link_libraries(hik_camera_node MvCameraControl)
ament_target_dependencies(hik_camera_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
  OpenCV
)

# 订阅节点
message(STATUS"==== TRY TO BUILD image_sub====")
add_executable(image_sub src/image_subscriber.cpp)
ament_target_dependencies(image_sub
  rclcpp
  sensor_msgs
  cv_bridge
  OpenCV
)

install(TARGETS
  hik_camera_node
  image_sub
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()

