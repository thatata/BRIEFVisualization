### Here is a basic overview of the functionality related specifically to Calibration & Actuation

**Note:**
Up to date code for calibration and actuation can be found in:
```
cameraarm/app/modeling_gui/ModelingWindow/src/

calibration_actuation.cxx
calibration_actuation.h
move.py
```

##### Refer to design document, and .cxx files for commented descriptions of each function
##### Move.py
This is an integrated script with a greedy scan that operates the Kinect via PCL to get photos (Functionality can be replaced and use obtainLiveImage function within API). It initiates these services and waits for API calls to move the robot
It takes in a string request in the form of [motorNum],[motorMove] i.e. ```"3,12"``` via the ```socket_request(string request)``` within calibration_actuation.cxx

##### Motor Values
TODO
- Motors:
  1. Servo Motor
  2. Middle Motor
  3. Lower Motor
  4. Stepper Motor (clockwise)
  5. Stepper Motor (counter_clockwise)

- Motor Constraints in Karl's world
  - Stepper Motor - Both directions [0,2500]
  - Lower Motor [25,140]
    - 1 step = 1 degree
  - Middle Motor [25,135]
    - 1 step = 1.07 degree
  - Servo Motor [37,145]
    - 1 step = 2.23 degrees

#### Supporting Images for Min and Max Angles
TODO

##### Logic behind Valid Pose
TODO
- Upload diagrams

##### Description of Various supporting Scripts
- getImage.sh
  - getImage queries the collected_data directory to get the most recent image from the Kinect and save within mostRecentImage.txt fore reference
- convertPNG
  - This is a PCL function that converts only the images of substance to PNG so that they can be displayed in the GUI
- test_main.cpp
  - Not used in functional application
  - Used for testing purposes only. Supplementary makefile also included for testing api functionality without GUI.

##### Printing additional Markers:
Markers that are saved and currently recognizable by the system are saved at
```
cameraarm/app/calibration_actuation/testing/aruco_markers/
```
**For proper analysis all markers must be .16m, or 16cm, when printed**

**Do not forget to update the code if more markers are added:**
In ```cameraarm/app/modeling_gui/ModelingWindow/src/calibration_actuation.cxx``` update to add ```Vec3d mki (x, y, z);``` for each marker ```i``` added with location in modeling world.

### Supporting Tests

#### Saving images:
Run `save` within `calibration_actuation/build`
with command: `$ ./save [fileName.png]`
Takes in live stream from Kinect to save images for static analysis

#### Camera Calibration
App created using API to get calibration intrinsics from the Kinect and saves to cameraCalibration.txt

#### load_from_directory
For independent location testing. It takes in an image path and views while also printing out location information associated.

#### testing/build
This folder has a variety of smaller tests performed during the initial phases of development to ensure pipeline with calibration and markers was working

#### testing/direct_mk0
This contains tests with direct angles to determine accuracy of Markers
Tests confirmed that data is more accurate at closer distances so we optimized to use the closest marker
