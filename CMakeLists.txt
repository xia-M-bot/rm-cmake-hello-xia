cmake_minimum_required(VERSION 3.16)
project(rm_vision)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(task1_image src/task1_image/main.cpp)
target_link_libraries(task1_image ${OpenCV_LIBS})

add_executable(task2_fit src/task2_fit/task2.cpp)
target_link_libraries(task2_fit ${OpenCV_LIBS})

add_executable(task3_windmill src/task3_windmill/task3.cpp)
target_link_libraries(task3_windmill ${OpenCV_LIBS})