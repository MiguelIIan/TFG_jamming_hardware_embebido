#!/usr/bin/env bash

source /opt/ros/jazzy/setup.bash
source /home/miguelian/uma_environment_tools/scripts/uma_env/uma_env.sh
source ~/ros/TFG_ws/install/setup.bash

ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0