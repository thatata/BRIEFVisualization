# BRIEF Modeling GUI

### Here is a basic overview of the functionality related specifically to the Modeling GUI

**Note:**
Code for the modeling gui implemented in the integrated system can be found in:
```
cameraarm/app/modeling_static/
```

#### Project Structure

##### ModelingMain

Main method to run the modeling GUI. User runs the ```ModelingWindow``` executable with an integer argument for the number of static images in the build folder. Creates a ModelingWindow object passing in the number of images, which starts the window.

##### ModelingWindow

Object that builds the render window, and holds the ```vtkRenderWindowInteractor``` object responsible for starting the render window, or ```vtkRenderWindow``` object. Responsible for creating a ```ModelingWindowStyle``` object that handles interaction in the render window. Each front-end element on the window (e.g. buttons, images) are represented by a ```vtkRenderer``` object that handles it's own interaction. Each renderer is mapped, as well as the static pose PNG's in the build folder, and passed into the ```ModelingWindowStyle``` to handle interaction.

##### ModelingWindowStyle

Object that is responsible for handling all interaction on the render window. User will interact with the window to build and manipulate 3D models. The ```ModelingWindowStyle``` object is responsible for all of the operations listed below, and quits the render window upon outputting the final model to the build folder.

### Overview of Manipulation Tools

##### Draw: after selecting the draw button followed by the object (e.g. cube, point, sphere), click on the scene to place that object in the scene.
Note: if cube is selected, user will be prompted to "snap" the cube into place.

##### Zoom: select Zoom, and in the console enter either 0 or 1 to zoom on the left and right pose, respectively.

##### Scale: with an object in the scene selected, use the up and down arrow keys to scale larger and smaller, respectively.

##### Stretch: with an object in the scene selected, use the arrow keys to stretch the object in the appropriate direction. To stretch about the z axis, press the z key and use the left and right arrow keys.

##### Rotate: with an object in the scene selected, use the arrow keys to rotate the object in the appropriate direction. To rotate about the z axis, press the z key and use the left and right arrow keys.

##### Move: with an object in the scene selected, click and drag the object to move the object in the scene.*

##### Request: select Request, and in the console enter the pose number to pop up a new rendering window.

##### Toggle: use the left and right arrow buttons on the bottom of the window to toggle between requested poses (if they exist).

##### Output: upon clicking the Output button, user will be prompted to select the desired output format and whether they'd like to output each object separately or combined into one model.

*Note: there's an underlying issue where a placed object seems inverted or intersects with the background image during manipulation. To resolve, move the object using the "Move" operation, and the object will snap itself into the appropriate place.
