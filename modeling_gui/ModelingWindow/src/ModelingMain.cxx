#include "modelingwindowstyle.h"
#include "modelingwindow.h"

#include <vtkObjectFactory.h>

/**
  * TH: Operations Overview
  *
  * Draw: after selecting an object (e.g. , point),
  * click on the scene to place that object in the scene.
  *
  * Zoom: select Zoom, and in the console enter either 0
  * or 1 to zoom on the left and right pose, respectively.
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
  * *Move: with an object in the scene selected, click and
  * drag the object to move the object in the scene.
  *
  * Request: select Request, and in the console enter the
  * pose number to pop up a new rendering window.
  *
  * Arrow Keys: use the left and right arrow buttons on
  * the bottom of the window to toggle between requested
  * poses (if they exist).
  *
  * Output: ??
  *
  * (*) --> Note: there's an underlying issue where a
  * placed object seems inverted or intersects with the
  * background image during manipulation. To resolve,
  * move the object using the "Move" operation, and
  * the object will snap itself into the appropriate
  * place.
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
