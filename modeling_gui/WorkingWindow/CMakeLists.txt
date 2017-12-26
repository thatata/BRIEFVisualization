
cmake_minimum_required(VERSION 2.8)
 
PROJECT(WorkingWindow)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(WorkingWindow MACOSX_BUNDLE WorkingWindow.cxx )
 
target_link_libraries(WorkingWindow ${VTK_LIBRARIES})
