cmake_minimum_required(VERSION 3.8)
project(sabre_hand_control)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)

add_library(hand_model src/hand_model.cpp)
target_compile_features(hand_model PUBLIC cxx_std_17)
target_include_directories(hand_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

add_executable(hand_commander src/hand_commander.cpp)
target_compile_features(hand_commander PUBLIC cxx_std_17)
target_link_libraries(hand_commander hand_model)
ament_target_dependencies(hand_commander rclcpp sensor_msgs trajectory_msgs)

add_executable(hand_demo src/hand_demo.cpp)
target_compile_features(hand_demo PUBLIC cxx_std_17)
target_link_libraries(hand_demo hand_model)
ament_target_dependencies(hand_demo rclcpp sensor_msgs)

install(TARGETS hand_model hand_commander hand_demo
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(hand_model_test test/hand_model_test.cpp)
  target_link_libraries(hand_model_test hand_model)
endif()

ament_export_include_directories(include)
ament_export_libraries(hand_model)
ament_package()
