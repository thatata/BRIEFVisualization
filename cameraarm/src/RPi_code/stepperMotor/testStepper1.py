#!/usr/bin/env python

# Karl Preisner
# December 23, 2016
# Test for stepperMotor.py

# Call moveMotor(num_steps, speed, direction)
# direction = "clockwise", "counterClockwise"


from stepperMotor import *
import time

motor1 = stepperMotor()

motor1.moveMotor(3000, "clockwise")
# time.sleep(2.0)
motor1.moveMotor(3000, "counterclockwise")