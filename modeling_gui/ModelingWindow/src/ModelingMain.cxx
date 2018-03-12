#include "modelingwindowstyle.h"
#include "modelingwindow.h"

/**
  * TH: Operations Overview
  *
  * Draw: after selecting an object (e.g. cube, point),
  * click on the scene to place that object in the scene.
  *
  * Zoom: left click on the scene to zoom in (X%), right
  * click on the scene to zoom out.
  *
  * Scale: with an object in the scene selected, use the
  * up and down arrow keys to scale larger and smaller,
  * respectively.
  *
  * Stretch: with an object in the scene selected, use the
  * arrow keys to stretch the object in the appropriate
  * direction. To stretch about the z axis, press the z
  * key and use the left and right arrow keys.
  *
  * Rotate: with an object in the scene selected, use the
  * arrow keys to rotate the object in the appropriate
  * direction. To rotate about the z axis, press the z
  * key and use the left and right arrow keys.
  *
  * Move: with an object in the scene selected, click and
  * drag the object to move the object in the scene.
  *
  * Request: click on the request button, and enter the
  * desired pose number in the console to open a new
  * modeling window.
  *
  * Output: ??
  *
  **/

vtkStandardNewMacro(ModelingWindowStyle);

int main(int argc, char *argv[])
{
    // check input arguments
    if ( argc != 2 )
    {
      std::cout << "Usage: " << argv[0]
                << " InputNumberOfImages" << std::endl;
      return EXIT_FAILURE;
    }

    // convert string to integer for number of images
    int numImages = atoi(argv[1]);

    // create new window with numImages as parameter
    ModelingWindow *window = new ModelingWindow(numImages);

    return EXIT_SUCCESS;
}
