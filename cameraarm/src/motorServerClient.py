# Karl Preisner
# February 25, 2017

# Class for opening client socket connection with RPi motor server.
# - Communitate with RPi motorServer.py
# - Check if values for motor are in motors' ranges.

# Note: all commands sent to the socket must have \n character at the end.

import socket
import time


class motorServerClient:

	def __init__(self): #constructor
		# create client socket
		self.clientSocket = socket.socket(family = socket.AF_INET, type = socket.SOCK_STREAM)

		self.ServoGearbox_Range = [37,145]   # degrees
		self.MiddleActuator_Range = [25,135] # degrees
		self.BottomActuator_Range = [25,140] # degrees
		self.StepperMotor_Range = [1,2500]   # steps

		self.ServoGearbox_Value = -1   # initial value
		self.MiddleActuator_Value = -1 # initial value
		self.BottomActuator_Value = -1 # initial value
		# stepper motor does not store a value

	def connect(self, ip_address, port):
		try:
			# connect to client
			self.clientSocket.connect((ip_address, port))
			# wait for "I'm alive" response
			response = self.clientSocket.recv(1000) # blocking 
			print "--RPi response: (%s)" %response
			if response == "I'm alive!":
				# Update motor values
				self.updateMotorValues()
				return True
		except:
			return False

	def disconnect(self):
		
		try:
			self.clientSocket.shutdown(socket.SHUT_RDWR)
			self.clientSocket = None
			print "--Socket shutdown complete."
			return True
		except:
			print "--Socket shutdown failed."
			return False
		
	def isConnected(self):
		# See if the RPi is alive
		try:
			self.clientSocket.send("Are you alive?\n")
			response = self.clientSocket.recv(1000) # receive up to 1000 characters (bytes)
			return True
		except:
			return False

	def send(self, command):
		self.clientSocket.send(command)

	def receive(self):
		response = ""
		self.clientSocket.setblocking(0) # set to non-blocking
		try:
			response = self.clientSocket.recv(1000)
		except:
			pass # nothing received from socket
		self.clientSocket.setblocking(1) # set to blocking
		return response

	def getMotorRange(self, motor):
		if motor == 1:
			return self.ServoGearbox_Range
		elif motor == 2:
			return self.MiddleActuator_Range
		elif motor == 3:
			return self.BottomActuator_Range
		elif motor == 4:
			return self.StepperMotor_Range
		elif motor == 5:
			return self.StepperMotor_Range

	def updateMotorValues(self):
		# Send a message to the RPi and get a response with the motor values. Then parse.
		print "--Update current motor values"
		self.send("Send client current motor values\n")
		# Get the response from RPi
		timeout = 500.0
		while timeout > 0.0:
			response = self.receive()
			if ":" in response:
				break
			time.sleep(0.01)
			timeout = timeout-0.01
		if timeout <= 0:
			print "--RPi response timeout!"

		# Parse the values and store them locally
		response = response.split(":") # Parse the response
		self.ServoGearbox_Value = int(response[0])
		self.MiddleActuator_Value = int(response[1])
		self.BottomActuator_Value = int(response[2])

	def getCurrentMotorValue(self, motor):
		if motor == 1:
			return self.ServoGearbox_Value
		elif motor == 2:
			return self.MiddleActuator_Value
		elif motor == 3:
			return self.BottomActuator_Value
		elif motor == 4:
			return 0
		elif motor == 5:
			return 0

	def valueInMotorRange(self, motor, value):
		if motor == 1:
			if value < self.ServoGearbox_Range[0] or value > self.ServoGearbox_Range[1]:
				return False
		elif motor == 2:
			if value < self.MiddleActuator_Range[0] or value > self.MiddleActuator_Range[1]:
				return False
		elif motor == 3:
			if value < self.BottomActuator_Range[0] or value > self.BottomActuator_Range[1]:
				return False
		elif motor == 4:
			if value < self.StepperMotor_Range[0] or value > self.StepperMotor_Range[1]:
				return False
		elif motor == 5:
			if value < self.StepperMotor_Range[0] or value > self.StepperMotor_Range[1]:
				return False
		return True

	# Send command to RPi server to move motor.
	def moveMotorCommand(self, motor, value):
		# Check if value is in range
		if self.valueInMotorRange(motor, value) == False:
			print "--Value not in motor range. Aborting moveMotor."
			return False
		
		# Formulate command to send to RPi
		m = ""
		if motor == 1:
			m = "Servo Gearbox:"
		elif motor == 2:
			m = "Linear Actuator - Middle:"
		elif motor == 3:
			m = "Linear Actuator - Bottom:"
		elif motor == 4:
			m = "Stepper Motor (clockwise):"
		elif motor == 5:
			m = "Stepper Motor (counterclockwise):"

		command = m + str(value) + "\n" # '\n' needed for socket command
		print "Sending command: (%s)" %command.rstrip()
		self.send(command)
		return True

	# Receive response from server after it was commanded to move a motor
	def moveMotorResponse(self):
		# Get first response from RPi
		timeout = 100.0
		while timeout > 0.0:
			response = self.receive()
			if response == "Begin moving motor":
				print "--RPi response: (%s)" %response
				break
			elif response == "Arduino not connected":
				print "--RPi response: (%s)" %response
				return False
			time.sleep(0.05)
			timeout = timeout-0.05

		if timeout <= 0:
			print "--RPi response 1 timeout!"
			return False

		# Get the second response from RPi
		timeout = 500.0
		while timeout > 0.0:
			response = self.receive()
			if response == "Finished moving motor":
				print "--RPi response: (%s)" %response
				break
			time.sleep(0.05)
			timeout = timeout-0.05

		if timeout <= 0:
			print "--RPi response 2 timeout!"
			return False

		# Update motor values
		self.updateMotorValues()

		return True
