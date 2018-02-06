
cmake_minimum_required(VERSION 2.8)
 
PROJECT(ModelingWindow)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/libs)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(HEADERS ${PROJECT_SOURCE_DIR}/libs/modelingwindow.h ${PROJECT_SOURCE_DIR}/libs/modelingwindowstyle.h)
set(SOURCES ${PROJECT_SOURCE_DIR}/src/ModelingMain.cxx ${PROJECT_SOURCE_DIR}/src/modelingwindow.cxx ${PROJECT_SOURCE_DIR}/src/modelingwindowstyle.cxx)

add_executable(ModelingWindow MACOSX_BUNDLE ${SOURCES} ${HEADERS})
 
target_link_libraries(ModelingWindow ${VTK_LIBRARIES})
