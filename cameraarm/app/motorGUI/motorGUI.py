#!/usr/bin/env python

# Karl Preisner
# January 19, 2017
# GUI class for moving all four motors

# if argv[1] == 1, this will not spawn openni viewer for Kinect Camera

# ssh karlpi3@192.168.2.200

from Tkinter import *
import tkFont
from PIL import Image, ImageTk # sudo pip install Pillow (might need to also install pip)
import time, sys, subprocess, signal, os

pwd = os.path.dirname(os.path.realpath(__file__))
sys.path.append(pwd+"/../../src/")
import motorServerClient

# Class motorGUI builds the GUI and has methods for button actions.
class motorGUI:

	def __init__(self, master, default_IP, default_PORT): # constructor
		self.master = master # this is important.

		# Current motor values
		self.ServoGearbox_Value = -1   # initial value
		self.MiddleActuator_Value = -1 # initial value
		self.BottomActuator_Value = -1 # initial value
		# stepper motor does not store a value

		# Add all components/widgets to window
		self.addGUIcomponents(self.master)

		# Set default ip address and port on GUI text boxes
		self.ip_Entry.insert(END, default_IP)
		self.port_Entry.insert(END, default_PORT)

		self.stepNum = 1 # for moveMotor method
		
		# Set initial mode 
		self.setMode("Connect to RPi")

		# Begin idleLoop
		self.idleLoop()

	def idleLoop(self):
		# This method runs when there are no other tasks running.
		if self.mode == "Connect to RPi":
			pass 
		elif self.mode == "Motor interaction":
			self.motorInteraction_mode()
		elif self.mode == "Moving motor":
			self.moveMotor_mode()

		# Refresh every 100ms (Does this actually work every 100ms?)
		self.master.after(100, self.idleLoop) 

	def setMode(self, mode):
		print "--Set mode =",mode
		self.mode = mode

		# Modes for GUI display
		if mode == "Connect to RPi":
			# Enable components associated with Connecting to RPi
			for c in self.connectionComponentList:
				self.enableWidget(c)
			# Disable components associated with motor interaction
			self.value_Entry.delete(0, END)
			for c in self.motorComponentList:
				self.disableWidget(c)
			self.currentValue_Label.config(text = "")
			self.infoBox_Label.config(text = "")
			self.infoBox_Message.config(text = "\n\n\n\n\n")
			self.status_Label.config(text = "")
			self.status_Message.config(text = "")
			self.unit_Label.config(text = "")
			self.selectedMotor.set(0)

		elif mode == "Motor interaction":
			# Enable components associated with motor interaction
			for c in self.motorComponentList:
				self.enableWidget(c)
			self.selectMotor_radioButton()
			# Disable components associated with Connecting to RPi
			for c in self.connectionComponentList:
				self.disableWidget(c)
		elif mode == "Moving motor":
			# disable moveMotor_Button, value_Entry, motorButtonList
			self.disableWidget(self.moveMotor_Button)
			self.disableWidget(self.value_Entry)
			self.disableWidget(self.disconnect_Button)
			for m in self.motorButtonList: # disable the motor selection buttons
				self.disableWidget(m)

	def motorInteraction_mode(self):
		# This does several things:
		# -- Enables/disables the moveMotor_Button if entry is selected motor's range
		# -- Checks value_Entry for a valid entry
		# -- Sets the status message box on above conditions

		# Set GUI components for this mode
		self.enableWidget(self.value_Entry)
		self.enableWidget(self.disconnect_Button)
		for m in self.motorButtonList: # enable the motor selection buttons
			self.enableWidget(m)

		# Get value from entry field.
		v = self.value_Entry.get()

		# Check if v is empty
		if len(v) == 0: # when entryField is empty
			self.disableWidget(self.moveMotor_Button)
			if self.moveCompleteMsg == False:
				self.clearStatusMsg()
		else:
			# Try to convert v to int
			try:
				value = int(v) # this line requires 'try:'
				motor = self.selectedMotor.get() # Get selected motor

				if self.clientSocket.valueInMotorRange(motor, value):
					if self.moveCompleteMsg == False:
						self.clearStatusMsg()
					self.enableWidget(self.moveMotor_Button)
				else:
					self.disableWidget(self.moveMotor_Button)
					self.setStatusMsg("Value not in range.", statusType = "Error:", color = "red")
					self.moveCompleteMsg = False
			except:
				self.disableWidget(self.moveMotor_Button)
				self.moveCompleteMsg = False
				self.setStatusMsg("Value must be a non-negative integer.", statusType = "Error:", color = "red")
			
	def moveMotor_mode(self):
		# this method is called from idleLoop to prevent stalling.
		if self.stepNum == 1: # Step 1: Send command to RPi
			print "\nMove motor begin"
			self.t_i = time.time() # keep track of time elapsed

			# check if still connected to RPi
			if self.clientSocket.isConnected() != True:
				print "--Connection failed. Move motor aborted."
				self.connectionStatus_Label.config(fg = "red", text = "Connection Failed")
				self.setMode("Connect to RPi")
				return

			 # change GUI display
			self.setMode("Moving motor")
			self.setStatusMsg("Sending command to RPi...")

			# create command to send to RPi
			motor = self.selectedMotor.get()
			value = int(self.value_Entry.get()) # NOTE: typing -0 works. might need to fix that
			# send command to RPi
			self.clientSocket.moveMotorCommand(motor, value)
			
			self.stepNum = 2 # move to step 2 in this method.

		elif self.stepNum == 2: # Step 2: Wait for first RPi response
			# get first response from RPi
			self.setStatusMsg("Waiting for RPi response...")
			response = self.clientSocket.receive()
				
			if response == "Begin moving motor":
				print "--RPi response: (%s)" %response
				self.stepNum = 3 # move to step 3 in this method	
			elif response == "Arduino not connected":
				print "--RPi response: (%s)" %response
				self.setStatusMsg("Arduino not connected. Check connection and try again.", statusType = "Error:", color = "red")
				self.stepNum = 4 # skip to step 4

		elif self.stepNum == 3: # Step 3: Wait for second RPi response
			# wait for RPi response that motor movement is complete
			self.setStatusMsg("Moving motor...")
			response = self.clientSocket.receive()
			if response == "Finished moving motor":
				print "--RPi response: (%s)" %response
				elapsed_time = str(int(time.time() - self.t_i))
				self.setStatusMsg("Motor command sent successfully.")
				# update motor values
				self.clientSocket.updateMotorValues()
				self.stepNum = 4 # go to final step

		elif self.stepNum == 4: # Step 4: Some stuff to finish up the method. 
			self.stepNum = 1 # for next time
			self.moveCompleteMsg = True # Used in idleLoop()
			self.value_Entry.selection_range(0, END) # highlight the text in the value field
			# Do NOT enable GUI components here. They must be enabled after method returns. Trust me.
			self.setMode("Motor interaction")
		
	def connectRPi_button(self):
		ip_address = self.ip_Entry.get()
		port = int(self.port_Entry.get())

		# Update GUI. Disable wigits while trying to connect. Change label.
		self.disableWidget(self.connect_Button)
		self.disableWidget(self.ip_Entry)
		self.disableWidget(self.port_Entry)
		self.connectionStatus_Label.config(fg = "black", text = "Establishing connection...")
		self.master.update_idletasks()

		# Create new client Socket
		self.clientSocket = motorServerClient.motorServerClient()

		# Try to connect to socket with IP_ADDRESS and PORT
		if self.clientSocket.connect(ip_address, port):
			print "\nConnected"
			self.connectionStatus_Label.config(fg = "green3", text = "Connected")
			self.selectedMotor.set(1) # default motor selection to first radio button
			self.setMode("Motor interaction")
		else:
			print "\nUnable to connect"
			self.connectionStatus_Label.config(fg = "red", text = "Unable to connect")
			self.setMode("Connect to RPi")

	def disconnectRPi_button(self):
		if self.clientSocket.disconnect():
			self.clientSocket = None
			self.connectionStatus_Label.config(fg = "red", text = "Disconnected")
			self.setMode("Connect to RPi")

	def selectMotor_radioButton(self):
		self.value_Entry.focus_set() # focus on entryField
		motor = self.selectedMotor.get()

		# Get motor's range and current value
		motorRange = str(self.clientSocket.getMotorRange(motor))
		motorCurrentValue = str(self.clientSocket.getCurrentMotorValue(motor))

		# set unit_Label and currentValue
		if motor in [1, 2, 3]:
			self.unit_Label.config(text = "degrees")
			self.currentValue_Label.config(text = "Current Value: "+motorCurrentValue)
		else:
			self.unit_Label.config(text = "steps")
			self.currentValue_Label.config(text = "Current Value: N/A")

		# set Message Box Title and Message
		if motor == 1:
			self.infoBox_Label.config(text = "Servo Gearbox:")
			self.infoBox_Message.config(text = "- Move the gearbox from its current position to the goal position entered in the 'Value' field.\n\n- Range: " + motorRange + " degrees.\n\n")
		elif motor == 2:
			self.infoBox_Label.config(text = "Linear Actuator - Middle:")
			self.infoBox_Message.config(text = "- Move the actuator from its current position to the goal position entered in the 'Value' field.\n\n- Range: " + motorRange + " degrees.\n\n")
		elif motor == 3:
			self.infoBox_Label.config(text = "Linear Actuator - Bottom:")
			self.infoBox_Message.config(text = "- Move the actuator from its current position to the goal position entered in the 'Value' field.\n\n- Range: " + motorRange + " degrees.\n\n")
		elif motor == 4:
			self.infoBox_Label.config(text = "Stepper Motor (clockwise):")
			self.infoBox_Message.config(text = "- Move the stepper motor clockwise the number of steps entered in the 'Value' field.\n\n- Range: " + motorRange + " degrees.\n\n")
		elif motor == 5:
			self.infoBox_Label.config(text = "Stepper Motor (counterclockwise):")
			self.infoBox_Message.config(text = "- Move the stepper motor counterclockwise the number of steps entered in the 'Value' field.\n\n- Range: " + motorRange + " degrees.\n\n")

	def setStatusMsg(self, statusMessage, statusType = "Status:", color = "green3"):
		# Do NOT call self.clearStatusMsg() here.
		self.status_Label.config(text = statusType, fg = color)
		self.status_Message.config(text = statusMessage, fg = color)

	def clearStatusMsg(self):
		self.status_Label.config(text = "")
		self.status_Message.config(text = "")

	def disableWidget(self, widget):
		if widget['state'] != "disabled":
			widget['state'] = "disabled"

	def enableWidget(self, widget):
		if widget['state'] == "disabled":
			widget['state'] = "normal"

	def addGUIcomponents(self, master):
		# Set Window Title
		master.title("Camera Arm Motors")

		# Set background color
		master.configure(background="white")
		
		# Set Window Dimensions
		master.minsize(width = 700, height = 700)
		master.maxsize(width = 000, height = 700)

		# -------- Image: 256x256
		self.armImage = ImageTk.PhotoImage(Image.open("arm_256x256.jpg"))
		self.armImage_Label = Label(master, image = self.armImage, bd = 0)
		self.armImage_Label.grid(row = 2, column = 20, columnspan = 30, rowspan = 14, sticky = "", padx = 30, pady = 0)

		# -------- Title Label
		self.title_Label = Label(master, text = "Camera Arm Motors", fg = "blue", relief = "solid", bg = '#e6e6ff', borderwidth = 5, font = "Helvetica 19 bold")
		self.title_Label.grid(row = 0, column = 0, rowspan = 2, columnspan = 50, pady = 20, padx = 190, ipadx = 40, ipady = 10)

		# -------- IP and port connection 
		self.connectRPi_Label = Label(master, text = "Connect to Raspberry Pi:", font = "Helvetica 15 bold underline", pady = 8, bg = "white")
		self.connectRPi_Label.grid(row = 2, column = 2, columnspan = 17, sticky = "sw")
		self.ip_Label = Label(master, text = "IP:", font = "Helvetica 12", bg = "white")
		self.ip_Label.grid(row = 3, column = 3, columnspan = 1, sticky = "e")
		self.port_Label = Label(master, text = "Port:", font = "Helvetica 12", bg = "white")
		self.port_Label.grid(row = 4, column = 3, columnspan = 1, sticky = "e")
		self.ip_Entry = Entry(master, width = 12, bg = "white")
		self.ip_Entry.grid(row = 3, column = 4, columnspan = 10, sticky = "w")
		self.port_Entry = Entry(master, width = 12, bg = "white") # casted to int() in connectRPi_button() method 
		self.port_Entry.grid(row = 4, column = 4, columnspan = 10, sticky = "w")

		# -------- Connection Status Label
		self.connectionStatus_Label = Label(master, text = "Not connected", fg = "red", font = "Helvetica 12", bg = "white")
		self.connectionStatus_Label.grid(row = 3, column = 14, columnspan = 18, sticky = "w")

		# -------- Connect Button
		self.connect_Button = Button(master, text = "Connect!", command = self.connectRPi_button, bg = "white")
		self.connect_Button['font'] = tkFont.Font(family = 'Helvetica', size = 12, weight = 'bold')
		self.connect_Button.grid(row = 4, column = 14, columnspan = 9, sticky = "w") # place 'Move motor!' button

		# -------- Disconnect Button
		self.disconnect_Button = Button(master, text = "Disconnect", command = self.disconnectRPi_button, bg = "white")
		self.disconnect_Button['font'] = tkFont.Font(family = 'Helvetica', size = 12, weight = 'bold')
		self.disconnect_Button.grid(row = 4, column = 23, columnspan = 9, sticky = "w") # place 'Move motor!' button

		# -------- Select Motor Label
		self.selectMotor_Label = Label(master, text = "Select a motor:", font = "Helvetica 15 bold underline", pady = 8, bg = "white")
		self.selectMotor_Label.grid(row = 5, column = 2, columnspan = 12, sticky = "sw")

		# -------- Radio Buttons for motor selection
		self.selectedMotor = IntVar() # default = 0
		self.mb1 = Radiobutton(master, variable = self.selectedMotor, value = 1, command = self.selectMotor_radioButton, pady = 4, font = "Helvetica 12", bg = "white", relief = FLAT, offrelief = FLAT)
		self.mb2 = Radiobutton(master, variable = self.selectedMotor, value = 2, command = self.selectMotor_radioButton, pady = 4, font = "Helvetica 12", bg = "white", bd = 0)
		self.mb3 = Radiobutton(master, variable = self.selectedMotor, value = 3, command = self.selectMotor_radioButton, pady = 4, font = "Helvetica 12", bg = "white", bd = 0)
		self.mb4 = Radiobutton(master, variable = self.selectedMotor, value = 4, command = self.selectMotor_radioButton, pady = 4, font = "Helvetica 12", bg = "white", bd = 0)
		self.mb5 = Radiobutton(master, variable = self.selectedMotor, value = 5, command = self.selectMotor_radioButton, pady = 4, font = "Helvetica 12", bg = "white", bd = 0)
		self.mb1.config(text = "Servo Gearbox")
		self.mb2.config(text = "Linear Actuator - middle")
		self.mb3.config(text = "Linear Actuator - bottom")
		self.mb4.config(text = "Stepper Motor (clockwise)")
		self.mb5.config(text = "Stepper Motor (counterclockwise)")
		self.mb1.grid(row = 6, column = 3, columnspan = 10, sticky = "w")
		self.mb2.grid(row = 7, column = 3, columnspan = 15, sticky = "w")
		self.mb3.grid(row = 8, column = 3, columnspan = 15, sticky = "w")
		self.mb4.grid(row = 9, column = 3, columnspan = 16, sticky = "w")
		self.mb5.grid(row = 10, column = 3, columnspan = 18, sticky = "w")
		
		# -------- Entry Field with Value and Unit Labels
		self.value_Label = Label(master, text = "Enter Value:", font = "Helvetica 15 bold underline", bg = "white")
		self.value_Label.grid(row = 11, column = 2, columnspan = 6, sticky = "w")
		self.value_Entry = Entry(master, width = 10, bg = "white")
		self.value_Entry.selection_range(0, END)
		self.value_Entry.selection_clear()
		self.value_Entry.grid(row = 11, column = 8, columnspan = 6, sticky = "w", pady = 10, padx = 2)
		self.unit_Label = Label(master, text = "", fg = "blue", bg = "white")
		self.unit_Label.grid(row = 11, column = 14, columnspan = 6, sticky = "w", pady = 10)

		# -------- Move Motor Button
		self.moveMotor_Button = Button(master, text = "Move motor!", command = self.moveMotor_mode, bg = "white")
		self.moveMotor_Button['font'] = tkFont.Font(family = 'Helvetica', size = 12, weight = 'bold')
		self.moveMotor_Button.grid(row = 11, column = 18, columnspan = 9, sticky = "w") # place 'Move motor!' button

		# -------- Current Value Label
		self.currentValue_Label = Label(master, text = "Current Value: 140", font = "Helvetica 12", bg = "white")
		self.currentValue_Label.grid(row = 13, column = 3, columnspan = 20, sticky = "w")

		# -------- Info Box
		self.infoBox_Label = Label(master, text = "", fg = "blue", font = "Helvetica 14 underline bold", pady = 8, bg = "white")
		self.infoBox_Label.grid(row = 15, column = 2, rowspan = 2, columnspan = 20, sticky = "sw")
		self.infoBox_Message = Message(master, text = "\n\n\n\n\n", width = 600, bg = "white")
		self.infoBox_Message.grid(row = 18, column = 2, rowspan = 10, columnspan = 46, sticky = "nw" )
		
		# -------- Status Bar
		self.status_Label = Label(master, text = "", fg = "green3", font = "Helvetica 12 underline", bg = "white")
		self.status_Label.grid(row = 28, column = 2, columnspan = 2, sticky = "nw")
		self.status_Message = Message(master, text = "", width = 600, fg = "green3", font = "Helvetica 13", bg = "white")
		self.status_Message.grid(row = 28, column = 4, columnspan = 46, rowspan = 3, sticky = "nw")
		self.moveCompleteMsg = False # for displaying motor movement completion message

		# Lists of components used to iteratively enable/disable
		# Note: disable not available for Tkinter Message() widget
		self.motorButtonList = [self.mb1, self.mb2, self.mb3, self.mb4, self.mb5] # list of the motor radio_buttons for efficient access when enabling/disabling
		self.connectionComponentList = [self.connectRPi_Label, self.ip_Label, self.ip_Entry, self.port_Label, self.port_Entry, self.connect_Button]
		self.motorComponentList = [self.disconnect_Button, self.selectMotor_Label, self.mb1, self.mb2, self.mb3, self.mb4, self.mb5, self.value_Label, self.value_Entry, self.unit_Label, self.moveMotor_Button, self.currentValue_Label, self.infoBox_Label, self.status_Label]


if __name__ == "__main__":
	# loopback ip
	# default_IP = "127.0.0.1" 

	if len(sys.argv) > 1 and sys.argv[1] == "noCam": # if folder is specified
		pass
	else:
		# Run pcl_openni_viewer to see what the Kinect is seeing
		pcl_openni_viewer = subprocess.Popen("pcl_openni_viewer", stdout=subprocess.PIPE,shell=True,preexec_fn=os.setsid)

	# RPi static IP
	default_IP = "192.168.2.200" # string
	default_PORT = 8188 # int

	# Build and the motorGUI
	root = Tk()
	motorGUI = motorGUI(root, default_IP, default_PORT) # make an instance of motorGUI

	# Run motorGUI
	root.mainloop()

	# Kill pcl_openni_viewer subprocess (if it is still running)
	os.killpg(os.getpgid(pcl_openni_viewer.pid), signal.SIGTERM)