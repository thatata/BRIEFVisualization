# import Karls API
    #TODO

import cmath

# import c++ class that extends opencv modules for pose estimation
# Connect via std i/o
import spam
status = spam.system.("ls -l")

'''
    README
    ------
    + All coordinate systems are measured in terms of meters
    + Everyone when modeling will work in a Cartesian coordinate system, current setup is using conversion to polar for movement using Karl's API
    + Center of shunk arm is origin. x-axis is when stepper motor is at 0, y-axis is perpendicular to both.
'''

# Cartesian coordinate set
class cartCoord:
    x = 0.0
    y = 0.0

    z = 0.0

# Polar coordinate set
class polarCoord:
    r = 0.0
    theta = 0.0
    z = 0.0

# Known values for converting polar to Karl's API
    # TODO: Fill in from written computed values


# Converting Tarek's cartesian coordinates to polar for analysis
def cartesian_to_polar( cartCoord c ):
    p = polarCoord()
    # z does not change
    p.z = c.z

    # TODO: Do the converstion
    # See if there is a library for this
    return p

# Converting polar coordinates back to cartesian for modeling
def polar_to_cartesian( polarCoord p ):
    c = cartCoord()
    # z does not change
    c.z = p.z

    # TODO: Do the converstion
    # See if there is a library for this
    return c

# Create new cartesian coordinate
# As of now that point can be converted to polar, no need to make polarCoord function
def new_cartCoord( int x, int y, int z ):
    c = cartCoord()
    c.x = x
    c.y = y
    c.z = z
    return c

# Given two polar coordinates (old and new)
# Perform the necessary movements of the robotic arms using Karl's API
def move( polarCoord old_pt, polarCoord new_pt ):
    #TODO: Get differences in theta and move stepper motor
    theta_diff = new_pt.theta - old_pt.theta

    #TODO: Get differences in r(radius)
    r_diff = new_pt.r - old_pt.r

    #TODO: Get differences in z
    z_diff = new_pt.z - old_pt.z

    #TODO: Use a combination of movements of other motors to get to correct r & z
        # Longer term

    #TODO: Ensure that moves are valid, if not change to valid, or throw error

    #TODO: Call Karl's API to move points

    #TODO: Call calibration function
    error_correct( new_pt )

'''
    TODO: Move this function to another .py file of it's own for cleanliness
    Given a point the robot is supposed to be at
    Determine it's actual location and move to that location again
    start with a condition to see if it is within an acceptable margin of error
'''
def error_correct( polarCoord desired_endpoint ):
    #TODO: Get actual location of camera (actual_pt)
        #TODO: Call upon Arucu & opencv libraries
    #TODO: Margin of error mesuring
    #TODO: If within, simply return | else continue
    #TODO: Not within error, call move function between points
    #TODO: move( actual_pt, desired_endpoint )
