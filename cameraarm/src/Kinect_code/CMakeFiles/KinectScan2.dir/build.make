# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/john/cameraarm/src/Kinect_code

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/john/cameraarm/src/Kinect_code

# Include any dependencies generated for this target.
include CMakeFiles/KinectScan2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/KinectScan2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/KinectScan2.dir/flags.make

CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o: CMakeFiles/KinectScan2.dir/flags.make
CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o: KinectScan2.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/john/cameraarm/src/Kinect_code/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o -c /home/john/cameraarm/src/Kinect_code/KinectScan2.cpp

CMakeFiles/KinectScan2.dir/KinectScan2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/KinectScan2.dir/KinectScan2.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/john/cameraarm/src/Kinect_code/KinectScan2.cpp > CMakeFiles/KinectScan2.dir/KinectScan2.cpp.i

CMakeFiles/KinectScan2.dir/KinectScan2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/KinectScan2.dir/KinectScan2.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/john/cameraarm/src/Kinect_code/KinectScan2.cpp -o CMakeFiles/KinectScan2.dir/KinectScan2.cpp.s

CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.requires:

.PHONY : CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.requires

CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.provides: CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.requires
	$(MAKE) -f CMakeFiles/KinectScan2.dir/build.make CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.provides.build
.PHONY : CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.provides

CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.provides.build: CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o


# Object files for target KinectScan2
KinectScan2_OBJECTS = \
"CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o"

# External object files for target KinectScan2
KinectScan2_EXTERNAL_OBJECTS =

KinectScan2: CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o
KinectScan2: CMakeFiles/KinectScan2.dir/build.make
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_system.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_thread.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_iostreams.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_chrono.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_atomic.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_regex.so
KinectScan2: /usr/lib/libpcl_common.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libflann_cpp_s.a
KinectScan2: /usr/lib/libpcl_kdtree.so
KinectScan2: /usr/lib/libpcl_octree.so
KinectScan2: /usr/lib/libpcl_search.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libqhull.so
KinectScan2: /usr/lib/libpcl_surface.so
KinectScan2: /usr/lib/libpcl_sample_consensus.so
KinectScan2: /usr/lib/libOpenNI.so
KinectScan2: /usr/lib/libOpenNI2.so
KinectScan2: /usr/lib/libpcl_io.so
KinectScan2: /usr/lib/libpcl_filters.so
KinectScan2: /usr/lib/libpcl_features.so
KinectScan2: /usr/lib/libpcl_keypoints.so
KinectScan2: /usr/lib/libpcl_registration.so
KinectScan2: /usr/lib/libpcl_segmentation.so
KinectScan2: /usr/lib/libpcl_recognition.so
KinectScan2: /usr/lib/libpcl_visualization.so
KinectScan2: /usr/lib/libpcl_people.so
KinectScan2: /usr/lib/libpcl_outofcore.so
KinectScan2: /usr/lib/libpcl_tracking.so
KinectScan2: /usr/lib/libpcl_apps.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_system.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_thread.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_iostreams.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_chrono.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_atomic.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libboost_regex.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libqhull.so
KinectScan2: /usr/lib/libOpenNI.so
KinectScan2: /usr/lib/libOpenNI2.so
KinectScan2: /usr/lib/x86_64-linux-gnu/libflann_cpp_s.a
KinectScan2: /usr/lib/libvtkGenericFiltering.so.5.8.0
KinectScan2: /usr/lib/libvtkGeovis.so.5.8.0
KinectScan2: /usr/lib/libvtkCharts.so.5.8.0
KinectScan2: /usr/lib/libpcl_common.so
KinectScan2: /usr/lib/libpcl_kdtree.so
KinectScan2: /usr/lib/libpcl_octree.so
KinectScan2: /usr/lib/libpcl_search.so
KinectScan2: /usr/lib/libpcl_surface.so
KinectScan2: /usr/lib/libpcl_sample_consensus.so
KinectScan2: /usr/lib/libpcl_io.so
KinectScan2: /usr/lib/libpcl_filters.so
KinectScan2: /usr/lib/libpcl_features.so
KinectScan2: /usr/lib/libpcl_keypoints.so
KinectScan2: /usr/lib/libpcl_registration.so
KinectScan2: /usr/lib/libpcl_segmentation.so
KinectScan2: /usr/lib/libpcl_recognition.so
KinectScan2: /usr/lib/libpcl_visualization.so
KinectScan2: /usr/lib/libpcl_people.so
KinectScan2: /usr/lib/libpcl_outofcore.so
KinectScan2: /usr/lib/libpcl_tracking.so
KinectScan2: /usr/lib/libpcl_apps.so
KinectScan2: /usr/lib/libvtkViews.so.5.8.0
KinectScan2: /usr/lib/libvtkInfovis.so.5.8.0
KinectScan2: /usr/lib/libvtkWidgets.so.5.8.0
KinectScan2: /usr/lib/libvtkVolumeRendering.so.5.8.0
KinectScan2: /usr/lib/libvtkHybrid.so.5.8.0
KinectScan2: /usr/lib/libvtkParallel.so.5.8.0
KinectScan2: /usr/lib/libvtkRendering.so.5.8.0
KinectScan2: /usr/lib/libvtkImaging.so.5.8.0
KinectScan2: /usr/lib/libvtkGraphics.so.5.8.0
KinectScan2: /usr/lib/libvtkIO.so.5.8.0
KinectScan2: /usr/lib/libvtkFiltering.so.5.8.0
KinectScan2: /usr/lib/libvtkCommon.so.5.8.0
KinectScan2: /usr/lib/libvtksys.so.5.8.0
KinectScan2: CMakeFiles/KinectScan2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/john/cameraarm/src/Kinect_code/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable KinectScan2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/KinectScan2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/KinectScan2.dir/build: KinectScan2

.PHONY : CMakeFiles/KinectScan2.dir/build

CMakeFiles/KinectScan2.dir/requires: CMakeFiles/KinectScan2.dir/KinectScan2.cpp.o.requires

.PHONY : CMakeFiles/KinectScan2.dir/requires

CMakeFiles/KinectScan2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/KinectScan2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/KinectScan2.dir/clean

CMakeFiles/KinectScan2.dir/depend:
	cd /home/john/cameraarm/src/Kinect_code && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/john/cameraarm/src/Kinect_code /home/john/cameraarm/src/Kinect_code /home/john/cameraarm/src/Kinect_code /home/john/cameraarm/src/Kinect_code /home/john/cameraarm/src/Kinect_code/CMakeFiles/KinectScan2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/KinectScan2.dir/depend
