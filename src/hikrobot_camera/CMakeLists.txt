cmake_minimum_required(VERSION 3.8)
project(hikrobot_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)
include_directories(/opt/MVS/include)
link_directories(/opt/MVS/lib/64)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(camera_node src/camera_node.cpp src/main.cpp)
ament_target_dependencies(camera_node
  rclcpp
  sensor_msgs
  cv_bridge
  OpenCV
)

add_executable(image_sub_node src/sub_main.cpp src/image_sub_node.cpp)
ament_target_dependencies(image_sub_node
  rclcpp
  sensor_msgs
  cv_bridge
)

target_link_libraries(image_sub_node ${OpenCV_LIBS})

target_link_libraries(camera_node
  libMvCameraControl.so
  ${OpenCV_LIBS}
)

install(TARGETS
  camera_node
  image_sub_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  camera_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  include
  config
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
