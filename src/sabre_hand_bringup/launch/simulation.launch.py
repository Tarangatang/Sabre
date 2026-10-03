"""Launch Gazebo Harmonic with the SABRE hand and ros2_control."""

from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    IncludeLaunchDescription,
    RegisterEventHandler,
)
from launch.conditions import IfCondition, UnlessCondition
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    description_share = Path(get_package_share_directory("sabre_hand_description"))
    bringup_share = Path(get_package_share_directory("sabre_hand_bringup"))
    ros_gz_share = Path(get_package_share_directory("ros_gz_sim"))

    model = description_share / "urdf" / "sabre_hand.urdf.xacro"
    world = description_share / "worlds" / "empty.sdf"
    controllers = bringup_share / "config" / "controllers.yaml"

    gui = LaunchConfiguration("gui")
    paused = LaunchConfiguration("paused")

    robot_description = ParameterValue(
        Command([
            "xacro ", str(model),
            " controller_config:=", str(controllers),
        ]),
        value_type=str,
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description, "use_sim_time": True}],
    )

    gazebo_gui = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(str(ros_gz_share / "launch" / "gz_sim.launch.py")),
        launch_arguments={"gz_args": ["-r -v 3 ", str(world)]}.items(),
        condition=IfCondition(gui),
    )
    gazebo_headless = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(str(ros_gz_share / "launch" / "gz_sim.launch.py")),
        launch_arguments={"gz_args": ["-s -r -v 3 ", str(world)]}.items(),
        condition=UnlessCondition(gui),
    )

    spawn_hand = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-name", "sabre_hand", "-topic", "robot_description"],
        output="screen",
    )

    clock_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=["/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock"],
        output="screen",
    )

    joint_state_broadcaster = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
        output="screen",
    )
    hand_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["hand_controller", "--controller-manager", "/controller_manager"],
        output="screen",
    )

    pause_world = ExecuteProcess(
        cmd=[
            "gz", "service", "-s", "/world/sabre_world/control",
            "--reqtype", "gz.msgs.WorldControl",
            "--reptype", "gz.msgs.Boolean",
            "--timeout", "3000",
            "--req", "pause: true",
        ],
        condition=IfCondition(paused),
        output="screen",
    )

    start_after_spawn = RegisterEventHandler(
        OnProcessExit(
            target_action=spawn_hand,
            on_exit=[joint_state_broadcaster, hand_controller, pause_world],
        )
    )

    return LaunchDescription([
        DeclareLaunchArgument("gui", default_value="true", description="Open Gazebo's GUI"),
        DeclareLaunchArgument(
            "paused", default_value="false",
            description="Pause physics shortly after Gazebo starts",
        ),
        gazebo_gui,
        gazebo_headless,
        robot_state_publisher,
        clock_bridge,
        spawn_hand,
        start_after_spawn,
    ])
