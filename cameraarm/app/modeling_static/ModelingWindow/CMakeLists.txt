
cmake_minimum_required(VERSION 2.8)
 
PROJECT(ModelingWindow)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(PCL 1.8 REQUIRED COMPONENTS common io)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

find_package(OpenCV REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/libs)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(HEADERS ${PROJECT_SOURCE_DIR}/libs/modelingwindow.h ${PROJECT_SOURCE_DIR}/libs/modelingwindowstyle.h)
set(SOURCES ${PROJECT_SOURCE_DIR}/src/ModelingMain.cxx ${PROJECT_SOURCE_DIR}/src/modelingwindow.cxx ${PROJECT_SOURCE_DIR}/src/modelingwindowstyle.cxx)

add_executable(ModelingWindow MACOSX_BUNDLE ${SOURCES} ${HEADERS})
 
target_link_libraries(ModelingWindow ${VTK_LIBRARIES} ${OpenCV_LIBS} ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES})
