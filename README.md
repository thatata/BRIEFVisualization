# BRIEF Visualization

Authors: A. Tarek Hatata & Thomas Magnan

![BRIEF](BRIEF_modeling.png)

This proposed project is an extension of a pre-existing “human-in-the-loop” application utilizing two robots and a turntable to image and grasp objects within a predetermined region. This project is referred to as the [Biological Robotic Imaging Experimentation Framework (BRIEF)](https://github.com/gw-cs-sd/sd-2017-BRIEF-Crandall).

This project focuses on advancement of the applications of the optical robotic arm. The end goal is to add functionality of the controlling GUI to easily move the arm and obtain images from other perspectives to build a refined 3D model of the object being imaged. Ideally this process can be automated through machine learning and analyzing an image stream to determine where another image is required.

The next stage of the project includes taking a series of images of this object over time and connecting the 3D models to create a 4D model where one can see the changes over time (the 4th dimension). An ideal object to model in three dimensional space over time is a plant, specifically a flower.

## Current Stage of Development

## Installation

### Hardware Components

### Software & System Requirements

## Source

Please refer to each respective subfolder for a deeper look into the functionality of each

For a deeper look into the architecture of our system please refer to our [Design Document](https://docs.google.com/document/d/15gEI3p_kkgORwE-nCUjNehDNtyYY_CgFskas9DObcVs/edit?usp=sharing)

## Modeling GUI integrated with BRIEF System
#### 1. Building

Note: Update ```DVTK_DIR``` with correct path if not using lab machine
```
$ cd cameraarm/app/modeling_gui/ModelingWindow
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=/home/workstation5/workplace/source/VTK5.10.1/build ..
$ make
```

#### 2. Connecting to the Raspberry Pi
1. Power on the Ubuntu 14.04 machine and log in.
2. Plug in the Raspberry Pi (RPi). Also, make sure that it is connected to the Ubuntu machine via ethernet cable. In the Wifi menu at the top right of the Ubuntu machine, make sure that 'RPi' is selected.
3. Plug in the XBox Kinect. A green LED on the Kinect should light up. If this light  does not appear, trace the wire and check its connection. Also, make sure that it is connected to the Ubuntu machine via USB.

Note: _Do **not**_ yet power on the power strip that supplies the linear actuators and stepper motor.

4. Open up **2** terminal windows on the Ubuntu machine.
5. In the first terminal, navigate to `/cameraarm/app/`.
6. Run `./sshRPi.sh` to ssh into the RPi. You will have to type the password (trumpet1).
7. On the RPi, run `./runServer.sh` and leave it alone.
8. In the second terminal window, navigate to `/cameraarm/app/modeling_gui/ModelingWindow/build/`. This is where you will run your application.
9. Finally, power on the power strip that supplies the linear actuators and stepper motor.

#### 3. Executing GUI
```
$ ./ModelingWindow 44
```
Upon initialization of the GUI ```move.py``` is called from within ```ModelingWindow/src/``` which runs the socket communication with the arm via the Raspberry Pi. The app will crash if Step 2 is not followed properly.

#### 4. Turning off the system

1. Begin by saving your model and exiting the application
2. **Important** Currently ```KinectScan and move.py``` are being run in the background. Go to System Monitor and find the processes and kill them. Otherwise images will continue to be taken.
3. Turn off the Hardware
  1. Turn off the power strip that supplies the linear actuators and the stepper motor.
  2. On the Ubuntu machine, in the terminal window that is logged into the RPi via ssh
  	1. Kill the motorSever by hitting Ctrl+c.
  	2. Run `./shutdown.sh`.
  	3. Run `exit` to close the ssh connection.
  	4. Close the terminal window on the Ubuntu machine.
  	5. Wait 10 seconds, double check that you have completed Step 1, then unplug the RPi.
  3. Unplug the XBox Kinect.
  ###### Important Tips:
  **1. Do not turn on/off linear actuator power strip without having RPi plugged in.**
  * The RPi powers the Arduino Uno via its USB connection. The Arduino's setup() method (which is called when the Arduino is first powered on) tells the linear actuators what their default positions should be. If the linear actuators do not have this constant signal from the Arduino Uno, the actuators default to 90 degrees.
  * Likewise, if you turn off the RPi (which will also turn off the Arduino Uno) before shutting off the power to the actuators, the actuators will default to 90 degrees because they will not have the signal from the Arduino.

  **2. Follow proper steps for powering off the RPi.**
  * If you do not do this, the RPi may be stuck in recovery mode. If this happens, you will need to plug a keyboard, monitor, and mouse into the RPi to diagnose the issue and escape recovery mode.

## Using the Modeling GUI with static images

## Tips

### Working on the Project from Home :: Remote Desktop, SSH, Remote Atom
For the purposes of the project it is best to keep the source code on the one lab machine that operates the robot. This can make things difficult to work on the project and using a [**Remote Desktop**](https://www.tecmint.com/enable-desktop-sharing-in-ubuntu-linux-mint/) or [**SSH combined with remote atom**](https://atom.io/packages/remote-atom) makes it much easier.

### Other

## Furthering the Research

###### Understanding the code base
Read through all of the code in ```cameraarm``` directory and read through our [Design Document](https://docs.google.com/document/d/15gEI3p_kkgORwE-nCUjNehDNtyYY_CgFskas9DObcVs/edit?usp=sharing)

###### Printing additional Markers:
Markers that are saved and currently recognizable by the system are saved at
```
cameraarm/app/calibration_actuation/testing/aruco_markers/
```
**For proper analysis all markers must be .16m, or 16cm, when printed**

**Do not forget to update the code if more markers are added:**
In ```cameraarm/app/modeling_gui/ModelingWindow/src/calibration_actuation.cxx``` update to add ```Vec3d mki (x, y, z);``` for each marker ```i``` added with location in modeling world.
