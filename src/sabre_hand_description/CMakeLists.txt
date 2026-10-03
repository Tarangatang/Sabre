cmake_minimum_required(VERSION 3.8)
project(sabre_hand_description)

find_package(ament_cmake REQUIRED)

install(DIRECTORY urdf worlds rviz
  DESTINATION share/${PROJECT_NAME}
)

ament_package()

