#!/bin/bash

# Karl Preisner
# March 25, 2017


# Note:
#    1. This script is called by another script!
#    2. This script runs KinectScan in the proper location.

# This script runs the Kinect scanning program and places all scanned
# images in /cameraarm/collected_data/folder1

# If no argument, all scanned images will be placed in /cameraarm/collected_data/output.
# All contents of this /output will be overwritten.

# If argument, scanned items will be placed in a directory with that title.
# If this folder exists, it will be overwritten.

# The program KinectScan will run in a background process.
# This script waits for stdin == "Stop scanning"
# Once it receives that input, it kills the process (KinectScan).

# example execution:
#    ./scan.sh folder1
#    >Stop scanning


BLUE="\033[0;34m"
GREEN="\033[0;32m"
MAGENTA="\033[0;35m"
NC="\033[0m"

# move to location where we will store files from scan
cd /home/workstation5/Tom/cameraarm

argv="$1" # get first argument
directory="collected_data"

if [ "$argv" == "" ]; then
	echo -e "${MAGENTA}Overwriting default scan file destination: \n${GREEN}'/home/workstation5/workplace/source/cameraarm/3DScanner/scans/${MAGENTA}$directory'${NC} \nNOTE: existing scan files will be overwritten."
	if [ -d "$directory" ]; then
		rm -rf $directory
	fi
	mkdir $directory
	cd $directory

elif [ ! -d "$argv" ]; then
	directory="$argv"
	echo -e "${MAGENTA}Creating new scan file destination: \n${GREEN}'/home/workstation5/workplace/source/cameraarm/3DScanner/scans/${MAGENTA}$directory'${NC}"
	mkdir $directory
	cd $directory

elif [ -d "$argv" ]; then
	directory="$argv"
	echo -e "${MAGENTA}Overwriting scan file destination: \n${GREEN}'/home/workstation5/workplace/source/cameraarm/3DScanner/scans/${MAGENTA}$directory'${NC}"
	if [ -d "$directory" ]; then
		rm -rf $directory
	fi
	mkdir $directory
	cd $directory
fi

# Reset Kinect camera
echo -e "\n${BLUE}Reset Kinect Camera:${NC}"
killall XnSensorServer

# Begin scan
echo -e -n "\n${BLUE}Start camera feed:${NC}"
/home/workstation5/Tom/cameraarm/src/Kinect_code/build/KinectScan &
scanPID=$!

# wait for command from stdin to stop scanning.
while true; do
	read var
	if [ "$var" == "Stop scanning" ]; then
		kill $scanPID
		break
	fi
done

# remove the last 1 or 2 files (depending on if they are the same file number)
output="$(ls -t | head -1)"
echo "${output}"

fileext=${output##*.}
if [ $fileext == "pcd" ]; then
	rm $output
else
	#statements
	rm $output
	output="$(ls -t | head -1)"
    echo "${output}"
    rm $output
fi

echo -e "${BLUE}Finished scanning images.${NC}"
echo -e "${MAGENTA}\nScans saved to: \n${GREEN}'/home/workstation5/workplace/source/cameraarm/${MAGENTA}$directory'${NC}"
