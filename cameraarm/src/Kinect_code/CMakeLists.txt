cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(Kinect_code)

find_package(PCL 1.7 REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable (KinectScan KinectScan.cpp)
target_link_libraries (KinectScan ${PCL_LIBRARIES})

add_executable (KinectScan2 KinectScan2.cpp)
target_link_libraries (KinectScan2 ${PCL_LIBRARIES})