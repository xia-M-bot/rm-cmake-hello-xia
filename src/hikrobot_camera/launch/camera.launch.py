from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_name = "hikrobot_camera"
    pkg_share = get_package_share_directory(pkg_name)
    param_file = os.path.join(pkg_share, "config", "camera.yaml")

    # 保留原版use_sim_time参数！！作业要求
    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use simulation clock'
    )

    camera_node = Node(
        package=pkg_name,
        executable="camera_node",
        name="camera_node",
        parameters=[{"use_simulation": False}, param_file],
        output="screen"
    )

    image_sub_node = Node(
    package=pkg_name,
    executable="image_sub_node",
    name="image_sub_node",
    output="screen"
    )


    return LaunchDescription([
        use_sim_time_arg,
        camera_node,
        image_sub_node
    ])
