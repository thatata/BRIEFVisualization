#!/usr/bin/env python

# Karl Preisner
# December 31, 2016
# This is a server for the Raspberry Pi to accept motor movement commands from a client.

# The arduino is a slave of the Raspberry Pi through serial connection.
# NOTE: Opening/closing/plugging/unplugging the serial port on the Arduino will reset the Aruino application.
# This will return the motors their default positions.

# Serial ports
# port = '/dev/cu.usbmodem1411' # Karl's Mac left USB
# port = '/dev/cu.usbmodem1412' # Karl's Mac right USB
# port = '/dev/ttyACM0' # Raspberry Pi 3

import os
import sys
import socket
import time
import serial
import serial.tools.list_ports

sys.path.append('/home/karlpi3/RPi_code/stepperMotor')
from stepperMotor import *

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#     Raspberry Pi static IP
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IP_ADDRESS = "192.168.2.200"

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#             Port
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PORT = 8188


class rpiserver:
	def __init__(self, ipaddr, port):

		# Current value of each motor
		self.ServoGearbox_Value = -1   # initial value
		self.MiddleActuator_Value = -1 # initial value
		self.BottomActuator_Value = -1 # initial value
		# stepper motor does not store a value


		# Bind to 'ipaddr' on 'port'
		self.port = port
		self.serversocket = socket.socket(family = socket.AF_INET, type = socket.SOCK_STREAM)  
		self.serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
		self.addr = ipaddr

		print "- Binding to",self.addr, "on port", self.port
		self.serversocket.bind((self.addr,self.port))

		print "- Listening..."
		self.serversocket.listen(10)

		# Set up GPIO communication with stepper motor
		self.stepperMotor = stepperMotor()


	def accept_connection(self):
		print "\nWaiting for connection on %s: %d"%(self.addr, self.port)
		# this will block until a client connects 
		(self.clientsocket, self.client) = self.serversocket.accept()
		
		# a client has connected to the socket
		# self.clientsocket is a new socket object for reading/writing to client
		print "- Connection accepted from", self.clientsocket.getpeername()

		# get the file descriptor from the client socket object
		self.fn = self.clientsocket.fileno()
		
		# create a buffered stream (FILE *) using the file descriptor so that readline() will block for complete lines
		self.fd = os.fdopen(self.fn,'r+')
		print "- Opened buffered file descriptor"

		# Try to open serial connection to Arduino
		self.isArduinoConnected = False
		self.connectArduino()

		# Send message to client that connection is established.
		self.ImAlive_response()

		return True

	def connectArduino(self):
		if self.isArduinoConnected == False: # this line may not be necessary
			print "- Establishing serial connection with Arduino..."
			serialPorts = list(serial.tools.list_ports.comports())

			for p in serialPorts: # Cycle through all available serial ports
				if "usb" in p[0] or "ACM" in p[0]:
					print "  - Found possible serial port. Trying now..."
					try:
						# Open serial connection to arduino.
						self.arduino = serial.Serial(p[0], 115200, timeout = 0.1)
					except:
						print "  - Exception: Could not open serial connection on port '%s'" %p[0]
					time.sleep(2) # give the connection two seconds to settle
					# Check if Arduino is alive.
					if self.isArduinoAlive() == True:
						print "  - Arduino connection established on serial port = '%s'" %p[0]
						self.isArduinoConnected = True 
						break
			if self.isArduinoConnected == False:
				print "  - Arduino connection failed. No port found. (Check USB connection)"

	def isArduinoAlive(self):
		# Send a message, wait for a response.
		# Returns True/False 
		# If false, it sets self.isArduinoConnected = False
		try:
			self.arduino.write("Are you alive?")
			response = ""
			for i in range(1,1000): # allow 10 seconds.
				time.sleep(0.01)
				response = self.arduino.readline()
				if response == "I'm alive! -Arduino":
					return True
		except:
			print "- Exception: Arduino not alive."
		self.isArduinoConnected = False
		return False

	def getArduinoMotorPositions(self):
		# parse response
		print "- Get current motor positions from Arduino"
		self.arduino.write("Get motor positions")
		response = ""
		for i in range(1,1000): # allow 10 seconds.
			time.sleep(0.01)
			response = self.arduino.readline()
			if ":" in response:
				break
		# Parse the response and store them locally
		response = response.split(":") 
		self.ServoGearbox_Value = int(response[0])
		self.MiddleActuator_Value = int(response[1])
		self.BottomActuator_Value = int(response[2])

	def sendCurrentMotorValues(self):
		self.getArduinoMotorPositions()
		msg = str(self.ServoGearbox_Value)
		msg += ":" + str(self.MiddleActuator_Value)
		msg += ":" + str(self.BottomActuator_Value)
		self.send_response(msg)

	def get_command(self):
		return self.fd.readline()

	def ImAlive_response(self):
		self.fd.write("I'm alive!")
		self.fd.flush()

	def send_response(self, msg):
		print "-- Sending response: (%s)" %msg
		self.fd.write(msg)
		self.fd.flush()

	def moveMotor(self, command):
		cmdTokens = command.split(':', 1)
		motor = cmdTokens[0]
		value = int(cmdTokens[1])

		if motor == "Servo Gearbox" or motor == "Linear Actuator - Middle" or motor == "Linear Actuator - Bottom":
			self.arduinoMoveMotor(command)
		elif motor == "Stepper Motor (clockwise)" or motor == "Stepper Motor (counterclockwise)":
			self.stepperMoveMotor(motor, value)

	def arduinoMoveMotor(self, command):
		# NOTE: In the arduino program, 10 seconds is given to move servoGearbox, 24 seconds for each actuator.
		if self.isArduinoConnected == False:
			print "- Arduino not connected. Retrying connection..."
			self.connectArduino()

		# Check if arduino is still connected.
		if self.isArduinoAlive() == True:
			# Send command to arduino. Arduino tokenizes command.
			self.arduino.write(command)

			# Arduino sends to responses; wait for both.
			response1 = ""
			response2 = ""
			for i in range(1,500): # allow 5 seconds.
				# Wait for first response
				time.sleep(0.01)
				response1 = self.arduino.readline()
				if response1 == "Begin moving motor. -Arduino":
					self.send_response("Begin moving motor")
					break
			if response1 == "Begin moving motor. -Arduino":
				# Wait for second response.
				for i in range(1,3000): # allow 30 seconds.
					time.sleep(0.01)
					response2 = self.arduino.readline()
					if response2 == "Finished moving motor. -Arduino":
						self.send_response("Finished moving motor")
						break
				if response2 == "":
					print "- ERROR: Arduino move motor response 2 timed out."
			else:
				print "- ERROR: Arduino move motor response 1 timed out."
		else:
			self.send_response("Arduino not connected")

	def stepperMoveMotor(self, motor, value):
		# TODO
		self.send_response("Begin moving motor")
		if "(counterclockwise)" in motor:
			# move stepper motor "value" steps counterclockwise
			self.stepperMotor.moveMotor(value, "counterclockwise")
		elif "(clockwise)" in motor:
			# move stepper motor "value" steps clockwise
			self.stepperMotor.moveMotor(value, "clockwise")

		time.sleep(0.5) # min communication time 
		self.send_response("Finished moving motor")


if __name__ == "__main__":
	
	# Print the logo in a cool way. - Karl Preisner
	os.system("clear")
	print "           |                    |          "; time.sleep(0.05)
	print " ----------|   RASPBERRY PI 3   |----------"; time.sleep(0.05)
	print " ----------|  Motor Controller  |----------"; time.sleep(0.05)
	print "           |____________________|         \o"; time.sleep(0.05)
	print "                                         __|\\"; time.sleep(0.05)
	print "                     ______                /"; time.sleep(0.05)
	print "                     \X^^^^^               "; time.sleep(0.05)
	print "                    .<\\\\                   "; time.sleep(0.05)
	print "                 ./XX/^\\\\  //              "; time.sleep(0.05)
	print "              ./XX/^    \\\\//               "; time.sleep(0.05)
	print "         ___/XX/^                          "; time.sleep(0.05)
	print "        /P^V\^                             "; time.sleep(0.05)
	print "       |( o )|                             "; time.sleep(0.05)
	print "        \\2v4/.                             "; time.sleep(0.05)
	print "           \XXX\.                          "; time.sleep(0.05)
	print "             ^\XXX\.                       "; time.sleep(0.05)
	print "                ^\XXX\___                  "; time.sleep(0.05)
	print "__________________|  KP  |_________________"; time.sleep(0.5)

	if len(sys.argv) > 1:
		IP_ADDRESS = sys.argv[1]
	if len(sys.argv) > 2:
		PORT = int(sys.argv[2])
	print "\nAccepting connection on %s: %d"%(IP_ADDRESS, PORT)

	# call constructor for rpiserver class
	sv = rpiserver(IP_ADDRESS, PORT)
	
	# loop accepting new connections
	while True:
		sv.accept_connection()
		# loop until the readline returns empty line (not even a newline char)
		while True:
			cmd = sv.get_command()

			if len(cmd) <= 0: # client must have closed connection
				print "Closing client socket"
				# graceful shutdown of client socket
				try:
					sv.clientsocket.shutdown(socket.SHUT_RDWR)
				except:
					pass
				# break loop and wait for next connection
				break
			
			# Strip trailing newline from command
			cmd = cmd.rstrip() 

			# Interpret commands received from client
			if cmd == "Are you alive?":
				sv.ImAlive_response()
				continue
			
			if cmd == "Connect to Arduino":
				sv.connectArduino()
				continue

			if cmd == "Send client current motor values":
				sv.sendCurrentMotorValues()
				continue

			# Command Received
			print "\nCommand received: (%s)" %cmd
			# Move motor
			sv.moveMotor(cmd)
