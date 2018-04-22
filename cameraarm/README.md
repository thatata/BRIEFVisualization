# How to properly use the system:
## How to turn the system on:
1. Power on the Ubuntu 14.04 machine and log in. 
2. Plug in the Raspberry Pi (RPi). Also, make sure that it is connected to the Ubuntu machine via ethernet cable. In the Wifi menu at the top right of the Ubuntu machine, make sure that 'RPi' is selected.
3. Plug in the XBox Kinect. A green LED on the Kinect should light up. If this light  does not appear, trace the wire and check its connection. Also, make sure that it is connected to the Ubuntu machine via USB.

Note: _Do **not**_ yet power on the power strip that supplies the linear actuators and stepper motor.

4. Open up **2** terminal windows on the Ubuntu machine.
5. In the first terminal, navigate to `/cameraarm/app/`.
6. Run `./sshRPi.sh` to ssh into the RPi. You will have to type the password (trumpet1).
7. On the RPi, run `./runServer.sh` and leave it alone.
8. In the second terminal window, navigate to `/cameraarm/app/`. This is where you will run your application (ex greedyScan.py).
9. Finally, power on the power strip that supplies the linear actuators and stepper motor.
## How to turn the system off:
1. Turn off the power strip that supplies the linear actuators and the stepper motor.
2. On the Ubuntu machine, in the terminal window that is logged into the RPi via ssh
	1. Kill the motorSever by hitting Ctrl+c.
	2. Run `./shutdown.sh`.
	3. Run `exit` to close the ssh connection.
	4. Close the terminal window on the Ubuntu machine.
	5. Wait 10 seconds, double check that you have completed Step 1, then unplug the RPi.
3. Unplug the XBox Kinect.
## Important Tips:
**1. Do not turn on/off linear actuator power strip without having RPi plugged in.** 
* The RPi powers the Arduino Uno via its USB connection. The Arduino's setup() method (which is called when the Arduino is first powered on) tells the linear actuators what their default positions should be. If the linear actuators do not have this constant signal from the Arduino Uno, the actuators default to 90 degrees.
* Likewise, if you turn off the RPi (which will also turn off the Arduino Uno) before shutting off the power to the actuators, the actuators will default to 90 degrees because they will not have the signal from the Arduino. 

**2. Follow proper steps for powering off the RPi.**
* If you do not do this, the RPi may be stuck in recovery mode. If this happens, you will need to plug a keyboard, monitor, and mouse into the RPi to diagnose the issue and escape recovery mode. 

-----
# How to run greedyscan
1. Turn the system on. **_Make sure to follow the steps above._**
2. Navigate to `/cameraarm/app/greedyScan/`.
3. Run `./greedyScan.py`.
	* Every time you run greedyScan, a new folder will be created inside of `/cameraarm/collected_data/new_folder` and all scanned image files are stored inside this new folder.
		* By default, `new_folder` will be named `output`.
		* Command line argument, `argv[1]` allows you to specify the name of the new folder that you would like to save your scanned images in. 
			* Again, leaving `argv[1]` blank will default save folder name to `/output`. This will save the files to `./cameraarm/collected_data/output`.
				* Ex: `./greedyScan.py`
			* To save files in a folder called `folder1`, make `argv[1]` `folder1`.
				* Ex: `./greedyScan.py folder1`
-----
# kinectScan.cpp
### What does it do?
Running kinectScan will start saving `.pcl` files in whatever directory you call kinectScan from. **Continue editing this a lot**
-----
# About:

Component list

Wiring diagram

What powers what
RPi power is separate plug
Arduino draws its power from RPi via USB. 
Hitec servo gearbox draws power from Arduino. 

Motor ranges

Describe how Kinect captures images 
-how to install necessary software
-how to compile KinectScan.cpp
-Openni Kinect image capture
-where images are stored

Applications:
Motorgui
-describe what it is used for
-it also opens up pcl viewer as subprocess
   -set argv[1]= noCam to start GUI without pcl viewer
-tkinter
   - how to install, what is required
-how it connects with motorseverclient

Greedyscan:
-what does it do
-how they can communicate with motorseverclient
-function calls in motorseverclient
-be careful of initial position of arm 
-describe necessary wait times



# Camera Arm Components
### Computers and microcontrollers:
* Ubuntu 14.04 machine
* Xbox Kinect
* Raspberry Pi
* Arduino Uno

### Motors
* Nema 23 stepper
* Servo City Linear Actuators (2x)
* Hitec HS-785HB servo with gearbox

### Drivers
* M542T stepper motor Driver
	* Used with Nema 23 stepper motor
	* https://www.omc-stepperonline.com/stepper-motor-driver/nema-23-stepper-motor-drive-24-50vdc-15a-45a-256-microstep-m542t-m542t.html
* Actobotics Dual Motor Controller
	* Used with the two Servo City Linear Actuators
	* https://www.servocity.com/actoboticsr-dual-motor-controller-assembled 
_____
# Wiring Guide:
### Raspberry Pi + Stepper Motor Driver (M542T)
![RPi_M542T](/resources/RPi_M542T.jpg)
### Nema23 Stepper Motor + Stepper Motor Driver (M542T)
![Nema23_M542T](/resources/Nema23_M542T.jpg)
### Arduino Uno + Dual Motor Controller
![Arduino_DualMotorController.jpg](/resources/Arduino_DualMotorController.jpg.jpg)
# Hardware settings
### Stepper Motor Driver (M542T)
(image of the switches)
also include diagram
### Dual Motor Controller Jumpers
(image of the jumpers)