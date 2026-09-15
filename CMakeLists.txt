cmake_minimum_required(VERSION 3.10)
project(pcd_rgb_convert)

find_package(PCL REQUIRED COMPONENTS io)

add_executable(pcd_rgb_convert pcd_rgb_convert.cpp)
target_link_libraries(pcd_rgb_convert ${PCL_LIBRARIES})
target_include_directories(pcd_rgb_convert PRIVATE ${PCL_INCLUDE_DIRS})
target_compile_definitions(pcd_rgb_convert PRIVATE ${PCL_DEFINITIONS})
