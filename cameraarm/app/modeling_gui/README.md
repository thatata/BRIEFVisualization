# BRIEF Modeling GUI

Here is a basic overview of the functionality related specifically to the Modeling GUI

**Note:**
Code for most up-to-date modeling gui in a static environment can be found in:
```
cameraarm/app/modeling_static/
```

## Building the project

Note: If not using lab machine, remove the build folder entirely to configure with your own system and follow these instructions:
```
$ cd cameraarm/app/modeling_gui/ModelingWindow
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=/home/workstation5/workplace/source/VTK5.10.1/build ..
$ make
```

Note: It's helpful to use ccmake to visualize the cmake build process and link the necessary libraries yourself (e.g. replacing ```DVTK_DIR``` with correct path to VTK install, same with OpenCV_DIR, etc.) by replacing the cmake line with ```ccmake ..``` If not already installed, run ```sudo apt-get install cmake-curses-gui``` to install.

## Running the GUI
```
$ ./ModelingWindow 44
```

## Project Structure

###### Refer to design document, and .cxx files for commented and more detailed descriptions of each function

#### ModelingMain

Main method to run the modeling GUI. User runs the ```ModelingWindow``` executable with an integer argument for the number of static images in the build folder (e.g. 44). Creates a ModelingWindow object passing in the number of images, which starts the window.

#### ModelingWindow

Object that builds the render window, and holds the ```vtkRenderWindowInteractor``` object responsible for starting the render window, or ```vtkRenderWindow``` object. Responsible for creating a ```ModelingWindowStyle``` object that handles interaction in the render window. Each front-end element on the window (e.g. buttons, images) are represented by a ```vtkRenderer``` object that handles it's own interaction. Each renderer is mapped, as well as the static pose PNG's in the build folder, and passed into the ```ModelingWindowStyle``` to handle interaction.

#### ModelingWindowStyle

Object that is responsible for handling all interaction on the render window. User will interact with the window to build and manipulate 3D models. The ```ModelingWindowStyle``` object is responsible for all of the operations listed below, and quits the render window upon outputting the final model to the build folder.

### Overview of Manipulation Tools

#### Draw

Select the Draw button followed by the object's button (e.g. cube, point, sphere), click on the scene to place that object in the scene.

Note: if cube is selected, user will be prompted to "snap" the cube into place.

#### Zoom

Select Zoom, and in the console enter either 0 or 1 to zoom on the left and right pose, respectively.

#### Scale

With an object in the scene selected (highlighted in red), use the up and down arrow keys to scale larger and smaller, respectively.

#### Stretch

With an object in the scene selected (highlighted in red), use the arrow keys to stretch the object in the appropriate direction. To stretch about the z axis, press the z key and use the left and right arrow keys.

#### Rotate

With an object in the scene selected (highlighted in red), use the arrow keys to rotate the object in the appropriate direction. To rotate about the z axis, press the z key and use the left and right arrow keys.

#### Move

With an object in the scene selected (highlighted in red), click and drag the object to move the object in the scene.

Note: there's an underlying issue where a placed object seems inverted or intersects with the background image during manipulation. To resolve this issue, use the Move tool to move the object slightly, and the object will pop itself into the appropriate place in front of the background image.

#### Request New Pose

Select Request, and either enter (1) the pose number (based on the PNG name) to retrieve in a static environment, or (2) the pose position to tell the calibration & actuation API to capture the new pose in the integrated system. Filenames of PNGs are passed into the function, new pose images are updated, and existing objects are transformed based on the pose info from the associated text file to reflect the pose change.

#### Toggle

Use the left and right arrow buttons on the bottom of the window to toggle between requested poses (if they exist).

#### Output

Click the Output button, and user will be prompted to select the desired output format (0 for .ply, 1 for .xyz, or 2 for .stl) and whether they'd like to output each object separately or combined into one model. Model(s) will be outputted to the build folder by utilizing a ```vtkSimplePointsWriter```, ```vtkPLYWriter```, ```vtkSTLWriter``` object to output the desired file format, and the window will close itself.

###### For more info on communication with Calibration & Actuation API
Refer to README outside of the ```cameraarm\``` folder.
