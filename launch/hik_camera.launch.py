from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='hik_camera',
            executable='hik_camera_node',
            name='hik_camera',
            output='screen',
            parameters=[
                {"use_sim": True}, # 改成True直接仿真，不用相机
                {"exposure": 20000.0},
                {"gain": 1.0},
                {"fps": 30.0},
                {"pixel_format": "Mono8"}
            ]
        )
    ])
