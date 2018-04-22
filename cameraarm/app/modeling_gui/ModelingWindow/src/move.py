#!/usr/bin/env python

# Karl Preisner
# March 25, 2017

# Thomas Magnan - Additions
# 23 January 2017

# Run this script to scan automatically perform a greedy scan of an object.
# Use a command line argument to specify the folder in which the scans will be placed.
# example:
#     ./greedyScan.py folder1


# motor == 1: "Servo Gearbox:"
# motor == 2: "Linear Actuator - Middle:"
# motor == 3: "Linear Actuator - Bottom:"
# motor == 4: "Stepper Motor (clockwise):"
# motor == 5: "Stepper Motor (counterclockwise):"



import sys, os
import time
import subprocess
import socket #TODO Tom make sure it works

import os, sys
pwd = os.path.dirname(os.path.realpath(__file__))
sys.path.append(pwd+"/../../../../src/")
from motorServerClient import *


# Location of the bash script to run. scan.sh takes an optional argument that specifies the folder for placing scans.
# scans are placed in CameraArm/collected_data/
SCAN_SCRIPT = pwd+"/../../../../src/Kinect_code/scan.sh"
folder = "collected_data" # default scan location

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Initialize camera arm joint motors. Set to default values.
motor1_value = 100 # Servo Gearbox
motor2_value = 30  # Linear Actuator - Middle
motor3_value = 50  # Linear Actuator - Bottom

num_steps = 2500   # max number of steps around the table = 2500
direction = "counterclockwise" # clockwise / counterclockwise
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Uncomment the setting you wish to use
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# # Blue Desk Chair
# motor1_value = 70   # servo gearbox
# motor2_value = 40   # middle actuator
# motor3_value = 120  # bottom actuator

# # Einstein bobble head doll
# motor1_value = 100
# motor2_value = 30
# motor3_value = 20

# # Schunk Arm
# motor1_value = 57
# motor2_value = 100
# motor3_value = 70

# # Schunk Arm Far
# motor1_value = 42
# motor2_value = 100
# motor3_value = 120

# Tarek's box
num_steps = 1500
motor1_value = 50 #Change back to 79 later
motor2_value = 25
motor3_value = 80


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def main():
	folder = "collected_data"
	if len(sys.argv) > 1: # if folder is specified
		folder = sys.argv[1]

	# Colored text
	NC = "\033[0;0m"
	BLUE = "\033[0;34m"
	GREEN = "\033[0;32m"
	UNDERLINE_GREEN = "\033[4;32m"

	# Opening monologue
	print UNDERLINE_GREEN+"                                                                        "+NC
	print UNDERLINE_GREEN+"Initiating Connection with RPi                     "+NC
	print BLUE+"Before we begin, complete these steps:"+NC
	print "    1. Plug RPi and Kinect camera into power. Wait 20 seconds."
	print "    2. Plug RPi into this computer via ethernet cable."
	print "    3. Plug Kinect into this computer via USB cable."
	print "    4. Manually move camera arm to \"Start\" position on table."
	print "    5. Switch motor power block on."
	print "    6. ssh into RPi and execute \'./runServer.sh\'"
	print BLUE+"Once you have completed these steps, press "+GREEN+"\"Enter\""+BLUE+" or exit with "+GREEN+"\"Ctrl+C\""+NC
	print UNDERLINE_GREEN+"                                                                        "+NC

	# raw_input()


	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# 1. Connect to RPi
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ip_address = "192.168.2.200"
	port = 8188

	clientSocket = motorServerClient()

	print BLUE+"Establishing connection with RPi..."+NC
	if clientSocket.connect(ip_address, port):
		print "--Connection established with RPi!"
	else:
		sys.exit("--Connection with RPi failed.")


	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# 2. Move camera arm joints to initial position.
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	print BLUE+"\nMove camera arm joint motors into position:"+NC
	# time.sleep(10)

	# Move Linear Actuator - Middle
	if clientSocket.moveMotorCommand(2, motor2_value) == False:
		print "Error: RPi move motor command."
		sys.exit("--Error moving motor. Value out of range.")
	if clientSocket.moveMotorResponse() == False:
		print "Error: RPi move motor response."
		sys.exit("--Error moving motor.")

	# Move Linear Actuator - Bottom
	if clientSocket.moveMotorCommand(3, motor3_value) == False:
		print "Error: RPi move motor command."
		sys.exit("--Error moving motor. Value out of range.")
	if clientSocket.moveMotorResponse() == False:
		print "Error: RPi move motor response."
		sys.exit("--Error moving motor.")

	# Move Servo Gearbox
	if clientSocket.moveMotorCommand(1, motor1_value) == False:
		print "Error: RPi move motor command."
		sys.exit("--Error moving motor. Value out of range.")
	if clientSocket.moveMotorResponse() == False:
		print "Error: RPi move motor response."
		sys.exit("--Error moving motor.")


	# Wait!
	t = 15
	print BLUE+"Moving motors. Please wait (%d seconds)..." %t
	for i in range(t,0,-1):
		print "%d \r" %i,
		sys.stdout.flush()
		time.sleep(1.0)

	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# 3. Begin taking images.
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	print BLUE+"\nBegin scanning images:"+NC
	scanProcess = subprocess.Popen([SCAN_SCRIPT, folder], stdin=subprocess.PIPE)
	time.sleep(3.0) # allow for the program to load. # do not adjust this value

	time.sleep(5.0) # sleep 5 seconds to get a good starting view of the object

	# # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# # 4. Move camera arm around table.
	# # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# # Move stepper motor
	# print BLUE+"Begin moving camera arm:"+NC
    #
	# if direction == "clockwise":
	# 	clientSocket.moveMotorCommand(4, num_steps) # clockwise
	# elif direction == "counterclockwise":
	# 	clientSocket.moveMotorCommand(5, num_steps) # counterclockwise
    #
	# if clientSocket.moveMotorResponse() == False:
	# 	print "Error: RPi move motor response."
    #
	# print BLUE+"Camera arm finished moving."+NC

	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# 4. Initiate Server and pe
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	HOST = '127.0.0.1' # '' #specifies that the socket is reachable by any address the machine happens to have
	PORT = 5000

	# create and INET, ICP socket
	serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	# Bind the socket to a public host, a well-known port
	serversocket.bind((HOST, PORT))

	# Initiate and become server socket
	serversocket.listen(2)
	print BLUE+"\nServer Listening"+NC

	# Read/Write Loop
	while True:
		# accepts connections
		(connsocket, address) = serversocket.accept()
		print("Connection established with: ", address)

		# Read data
		while True:
			data = connsocket.recv(1024)
			if not data: break
			if data == 'Disconnect': break
			print("From connected user: ", data.decode()) # Decode from byte to string to desplay for proof of transfer

			# Do what needs to be done
			print("Request: ", data)

			# Parse for motor number and num_steps
			d_list = data.split(',')
			motor = int(d_list[0])
			num_steps = int(d_list[1])
			# Move stepper motor
			print BLUE+"Begin moving camera arm:"+NC
			clientSocket.moveMotorCommand(motor, num_steps)

			if clientSocket.moveMotorResponse() == False:
				print "Error: RPi move motor response."

			print BLUE+"Camera arm finished moving."+NC

			connsocket.send(b'Echo=>' + data) # Rencode the data into the byte stream and send back to client

		connsocket.close()

	serversocket.close()
	print "About to disconnect"

	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# 5. Stop taking images.
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	time.sleep(0.5) # we need this sleep here. trust me.
	scanProcess.communicate("Stop scanning")


	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# 6. Disconnect RPi
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	time.sleep(0.5) # we need this sleep here. trust me.
	print BLUE+"\nDisconnected from RPi."+NC
	clientSocket.disconnect()

	print UNDERLINE_GREEN+"                                                                        "+NC
	print UNDERLINE_GREEN+"                             Scan complete!                             \n"+NC

	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# End main()
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~





# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Run main()
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if __name__ == '__main__':
	main()
