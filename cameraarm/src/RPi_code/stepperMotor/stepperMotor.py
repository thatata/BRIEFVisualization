#/usr/bin/python

# run with 'sudo'

# Karl Preisner, Bob Forcha
# Feb 3, 2017

# 1. This is the setup for the Nema 23 stepper motor with M542T Driver. 
# 2. motorServer.py creates and instance of stepperMotor and calls functions from this class.


import RPi.GPIO as GPIO
import time
from sys import argv

# class stepperMotor contains all methods for running the stepper motor
class stepperMotor:

	def __init__(self): # constructor
		
		# adjustable global variables
		self.acceleration = 40.0  # magnitude of linear acceleration
		self.startSpeed = 10.0    # initial velocity to begin acceleration  # 10.0 is a good slow setting
		self.maxSpeed = 50.0    # max speed for camera arm

		##################################################################
		##################################################################
		##################################################################
		# GPIO pin assignments
		self.pulse_pin = 23 # GPIO 23 (RPi pin 16) --Blue wire
		self.dir_pin = 24   # GPIO 24 (RPi pin 18) --Yellow wire
		self.ena_pin = 25   # GPIO 25 (RPi pin 22) --Green wire

		# GPIO settings
		GPIO.setwarnings(False)
		GPIO.setmode(GPIO.BCM)
		GPIO.setup(self.pulse_pin, GPIO.OUT)
		GPIO.setup(self.dir_pin, GPIO.OUT)
		GPIO.setup(self.ena_pin, GPIO.OUT)
		GPIO.output(self.ena_pin, 0)
		GPIO.output(self.pulse_pin, 0)
		GPIO.output(self.dir_pin, 0)

		##################################################################
		################### Delete this eventually  ######################
		##################################################################
		# Significant numbers for calculations
		motorSteps = 400 # number of steps for 1 cycle of the motor
		pulleyTeeth = 40 # number of teeth on the pulley
		tableTeeth = 300 # number of teeth on the table gear
		stepsPerTooth = motorSteps / pulleyTeeth # = 10 steps
		tableSteps = tableTeeth * stepsPerTooth  # = 3000 steps
		motorCyclesPerArmCycle = 300/40          # = 7.5 motor cycles

		# Predefined SPEEDS
		#		 steps/sec		 sec/step	sec/table
		speed1 = 50.0			# 0.020s	60s
		speed2 = 62.5			# 0.016s	48s
		speed3 = 1/0.012 #83.3~	# 0.012s	36s
		speed4 = 125.0			# 0.008s	24s
		speed5 = 250.0			# 0.004s	12s
		speed6 = 500.0			# 0.002s	6s
		#
		#	10 steps/tooth
		#	300 teeth/table rotation
		#	120000 steps/table rotation
		#	
		#		1/t = s
		#		1/s = t	
		#
		#	Helpful math:
		#		Speed #1: step = 0.02s, 1 rev motor/pulley (400 steps/40 teeth) = 8.0s, 1 rev arm (300 teeth) = 60s
		##################################################################
		##################################################################

	# This function is used in stepMotor()
	def pulseMotor(self, speed):
		# speed = steps/sec
		# 1/speed = sec/step (what we need)
		# 1 step on motor takes "speed" time
		s = 1.0/speed # must be float!
		GPIO.output(self.pulse_pin, 1)
		time.sleep(s/2.0)
		GPIO.output(self.pulse_pin, 0)
		time.sleep(s/2.0)

	# This function is used in moveMotor()
	def stepMotor(self, num_steps, speed):
		for i in range(0, int(num_steps)):
			self.pulseMotor(speed)

	def setDirection(self, direction):
		# Set direction
		if direction == "clockwise":
			GPIO.output(self.dir_pin, 0)
		elif direction == "counterclockwise":
			GPIO.output(self.dir_pin, 1)
		else:
			print "Error - direction argument not valid"
			return -1

	# Move the motor - Only call this method.
	def moveMotor(self, num_steps, direction, acceleration = True): 
		# check values
		if isinstance(num_steps, int) == False or num_steps <= 0:
			print "Error in moveMotor(): num_steps must be integer greater than zero"
			return

		# set the direction
		if self.setDirection(direction) == -1:
			return

		# Move the motor
		if acceleration == False:
			self.stepMotor(num_steps, self.maxSpeed)
		elif acceleration == True:
			self.moveMotor_acceleration(num_steps)

	# This function is used in moveMotor()
	def moveMotor_acceleration(self, num_steps):
		#=============================
		accelDist = self.maxSpeed * self.maxSpeed / (2 * self.acceleration) # calculated distance to fully accel to self.maxSpeed
		constDist = num_steps - (accelDist * 2.0) # calculated distance moving at constant speed (self.maxSpeed)
		
		if (constDist < 0):
			accelDist = num_steps/2.0;
		
		decelDist = accelDist

		if(float(num_steps)%2.0 == 1):
			# print "---Odd number of steps:"		
			if constDist < 0:	
				# print "   ---No constant speed. distAccel++"			
				constDist = 0 # go straight from accel to decel
				accelDist = accelDist + 1
			else:
				constDist = num_steps - accelDist - decelDist # steps moved at constant speed
		else:
			constDist = num_steps - accelDist - decelDist # steps moved at constant speed
		

		# print "Begin move!"
		# print "---Acceleration: %d" % self.acceleration
		# print "---Speed:        %d" % self.maxSpeed
		# print "---Total steps:  %d" % num_steps
		# print "---Total acceleration step requirement:  %d steps" % accelDist
		# print "---Total deceleration step requirement:  %d steps" % accelDist
		# print "---Ratio (accelDist)/total steps): %f" % (accelDist / num_steps)
		
		
		#=============================
		# Acelerate
		t0 = time.time()
		totalTime = 0.0
		currSpeed = 0.0 # speed of step at stepCount & currTime	
		# print "\n1. Begin Acceleration for %d steps..." %accelDist
		for currStep in range(0, int(accelDist)):
			if totalTime == 0:
				currSpeed = self.startSpeed #start speed to accelerate
			else:
				currSpeed = (currStep / totalTime) + (self.acceleration * totalTime * 0.5)
			#print "%d: %d" %(currStep,currSpeed)		
			self.stepMotor(1, currSpeed)
			totalTime = totalTime + (1.0/currSpeed)
		# print "   ---Finished accelerating!"
		

		#=============================
		# Constant speed	
		if constDist > 0:
			# print "2. Moving at constant speed for %d steps..." %constDist	
			self.stepMotor(constDist, self.maxSpeed)
			# print "   ---Finished moving at constant speed!"
		# else:
			# print "2. Did not achieve full speed at this acceleration rate." 

			
		#=============================
		# Decelerate
		totalTime = 0
		# print "3. Begin Decelerating for %d steps..." %decelDist
		for currStep in range(0, int(decelDist)):
			if totalTime == 0:
				currSpeed = currSpeed #start speed to decelerate
			else:
				currSpeed = (currStep / totalTime) - (self.acceleration * totalTime * 0.5)
			#print "%d: %d" %(currStep,currSpeed)		
			self.stepMotor(1, currSpeed)
			totalTime = totalTime + (1.0/currSpeed)
		# print "   ---Finished decelerating!"
		
		# print "Run Time: %f seconds\n" % (time.time() - t0)
	#end method