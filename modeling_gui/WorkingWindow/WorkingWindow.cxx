#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkJPEGReader.h>
#include <vtkPNGReader.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleUser.h>
#include <vtkObjectFactory.h>
#include <vtkAbstractPicker.h>
#include <vtkImageData.h>
#include <vtkImageSincInterpolator.h>
#include <vtkImageResize.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <vtkImageReader2Factory.h>
#include <vtkInteractorStyleUnicam.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkInteractorObserver.h>
#include <vtkCamera.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkRendererCollection.h>
#include <vtkProperty.h>
#include <vtkPlanes.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkPointSource.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkAreaPicker.h>
#include <vtkExtractGeometry.h>
#include <vtkDataSetMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkIdFilter.h>
#include <vtkActorCollection.h>

/**
  * TH: Operations Overview
  *
  * Draw: with an object button selected (e.g. cube, point),
  * click on the scene to place that object in the scene.
  *
  * Zoom: left click on the scene to zoom in (X%), right
  * click on the scene to zoom out.
  *
  * Scale: with an object in the scene selected, use the
  * up and down arrow keys to scale larger and smaller,
  * respectively.
  *
  * Stretch: ??
  *
  * Rotate: with an object in the scene selected, use the
  * arrow keys to rotate the object in the appropriate
  * direction. To rotate about the z-axis, press the z
  * key and use the left and right arrow keys.
  *
  * Move: with an object in the scene selected, click and
  * drag the object to move the object in the scene.
  *
  * Request: ??
  *
  * Output: ??
  *
  **/

// structure holding actor and cube source of each cube object
struct CubeData {
    // actor object
    vtkSmartPointer<vtkActor> actor;

    // cube source object
    vtkSmartPointer<vtkCubeSource> cubeSource;

    // variables to store scale factor, rotation angles, translation values
    double scaleFactor, rotationX, rotationY, rotationZ, translateX, translateY;
};

class MyInteractorStyle : public vtkInteractorStyleTrackballActor {
    public:
        static MyInteractorStyle* New();
        vtkTypeMacro(MyInteractorStyle, vtkInteractorStyleTrackballActor);

        MyInteractorStyle() {
            // set default values
            Cube = false;
            Point = false;
            Other = false;
            Draw = false;
            Rotate = false;
            RotateZ = false;
            Selected = false;
            Scale = false;
            Move = false;
            Moving = false;
            Stretch = false;
            Zoom = false;
        }

        virtual void OnLeftButtonDown() {
            // get position of the click
            int x = this->Interactor->GetEventPosition()[0];
            int y = this->Interactor->GetEventPosition()[1];

            // set the appropriate renderer based on the click position
            this->FindPokedRenderer(x, y);

            // perform operation based on the renderer selected
            PerformAction();
        }

        virtual void OnKeyPress() {
            // check if an object was selected
            if (Selected) {
                // if so, check if rotate was selected
                if (Rotate) {
                    // if so, rotate based on key pressed
                    std::string key = this->Interactor->GetKeySym();

                    // check if z was pressed for z-axis rotation
                    if (key == "z") {
                        // if so, switch flag
                        RotateZ = !RotateZ;

                        // alert user in the console
                        std::cout << "Z Rotation " << (RotateZ ? "Enabled!" : "Disabled!")
                                  << std::endl;

                        return;
                    }

                    // vars for axis and angle of rotation
                    int axis;
                    double angle;

                    // check if z rotation has been enabled
                    if (RotateZ) {
                        // if so, axis = 2
                        axis = 2;

                        // check for left/right arrow keys only
                        if (key == "Left") {
                            // rotate in native direction
                            angle = 5;
                        } else if (key == "Right") {
                            // rotate in native direction
                            angle = -5;
                        } else {
                            // otherwise don't rotate and return
                            return;
                        }
                    } else {
                        // based on arrow key, rotate x/y axes in native direction
                        if (key == "Left") {
                            axis = 1;
                            angle = -5;
                        } else if (key == "Right") {
                            axis = 1;
                            angle = 5;
                        } else if (key == "Up") {
                            axis = 0;
                            angle = -5;
                        } else if (key == "Down") {
                            axis = 0;
                            angle = 5;
                        } else {
                            // otherwise don't rotate and return
                            return;
                        }
                    }

                    // rotate cube using axis and angle
                    RotateCube(axis, angle);
                }
            }
        }

        void OnLeftButtonUp() {
            // check if moving, if so finish
            if (Moving) {
                // end pan
                this->EndPan();

                // change moving to false
                Moving = false;

                // get cube data
                CubeData *data = GetCube(selectedActor);

                // update translation values (in world coordinates)
                data->translateX = selectedActor->GetPosition()[0];
                data->translateY = selectedActor->GetPosition()[1];
            }
        }

        void OnMouseMove() {
            // check if an actor is moving, if so just return
            if (!Moving) {
                return;
            }

            // otherwise, forward events
            vtkInteractorStyleTrackballActor::OnMouseMove();
        }

        void MoveCube() {
            // check if selected actor was under the mouse click
            vtkSmartPointer<vtkActor> actor = GetActorUnderClick();

            // check if actor isn't null and is the selected actor
            if (actor != NULL && actor == selectedActor) {
                // if so, moving
                Moving = true;

                // start pan
                this->StartPan();

                // change interaction prop to the actor selected
                this->InteractionProp = selectedActor;
            }
        }

        void RotateCube(int axis, double angle) {
            // create transform object
            vtkSmartPointer<vtkTransform> transform =
                vtkSmartPointer<vtkTransform>::New();

            // get CubeData object from the selected actor
            CubeData *data = GetCube(selectedActor);

            // get the center position of the cube from CubeData
            double *center = data->cubeSource->GetCenter();

            // apply initial translation to center the cube at the origin
            transform->Translate(center[0], center[1], center[2]);

            // update rotation angles based on selected axis
            switch(axis) {
                case 0:
                    // update x angle rotation
                    data->rotationX += angle;
                    break;
                case 1:
                    // update y angle rotation
                    data->rotationY += angle;
                    break;
                case 2:
                    // update z angle rotation
                    data->rotationZ += angle;
                default:
                    break;
            }

            // apply appropriate rotations
            transform->RotateX(data->rotationX);
            transform->RotateY(data->rotationY);
            transform->RotateZ(data->rotationZ);

            // apply final translation
            transform->Translate(-center[0], -center[1], -center[2]);

            // set up transform filter
            vtkSmartPointer<vtkTransformFilter> filter =
              vtkSmartPointer<vtkTransformFilter>::New();

            // add cube and transform, and update
            filter->SetInputConnection(data->cubeSource->GetOutputPort());
            filter->SetTransform(transform);
            filter->Update();

            // set up new mapper
            vtkSmartPointer<vtkPolyDataMapper> mapper =
              vtkSmartPointer<vtkPolyDataMapper>::New();

            mapper->SetInputConnection(filter->GetOutputPort());

            // set mapper to existing actor
            this->selectedActor->SetMapper(mapper);

            // reset camera and update window
            this->Interactor->Render();
        }

        void PerformAction() {
            // perform appropriate action based on which renderer is selected
            if (this->CurrentRenderer == rendererMap[0]) {
                // cube selected
                CubeSelected();
            } else if (this->CurrentRenderer == rendererMap[1]) {
                // point selected
                PointSelected();
            } else if (this->CurrentRenderer == rendererMap[2]) {
                // other selected
                OtherSelected();
            } else if (this->CurrentRenderer == rendererMap[3]) {
                // scene selected
                SceneSelected();
            } else if (this->CurrentRenderer == rendererMap[4]) {
                // draw selected
                DrawSelected();
            } else if (this->CurrentRenderer == rendererMap[5]) {
                // zoom selected
                ZoomSelected();
            } else if (this->CurrentRenderer == rendererMap[6]) {
                // scale selected
                ScaleSelected();
            } else if (this->CurrentRenderer == rendererMap[7]) {
                // stretch selected
                StretchSelected();
            } else if (this->CurrentRenderer == rendererMap[8]) {
                // rotate selected
                RotateSelected();
            } else if (this->CurrentRenderer == rendererMap[9]) {
                // move selected
                MoveSelected();
            } else if (this->CurrentRenderer == rendererMap[10]) {
                // request selected
                RequestSelected();
            } else if (this->CurrentRenderer == rendererMap[11]) {
                // output selected
                OutputSelected();
            }

            // otherwise, perform no action
        }

        void SceneSelected() {
            // check if draw is selected
            if (Draw) {
                // check if cube is selected, if so draw a cube
                if (Cube) {
                    // if so, draw cube
                    DrawCubeOntoImage();

                    // re-render
                    this->Interactor->Render();

                    // done, so return
                    return;
                }
            }

            // otherwise, check if object was selected
            if (Selected) {
                // if so, check if move tool was selected
                if (Move) {
                    // if so, move cube
                    MoveCube();
                } else {
                    // otherwise, deselect the actor

                    // change actor color to white
                    selectedActor->GetProperty()->SetColor(1,1,1);

                    // change flag
                    Selected = false;

                    // re-render
                    this->Interactor->Render();

                }

            } else {
                // if no object was selected, check if user tried to select an object
                vtkSmartPointer<vtkActor> actor = GetActorUnderClick();

                // if actor is not null, user selected that actor
                if (actor != NULL) {
                    // change flag
                    Selected = true;

                    // save address for future use
                    selectedActor = actor;

                    // change color to red to denote selected
                    actor->GetProperty()->SetColor(1,0,0); // red

                    // re-render to update color
                    this->Interactor->Render();
                }
            }
        }

        vtkSmartPointer<vtkActor> GetActorUnderClick() {
            // get click position
            double *click = GetClickPosition();

            // get actors from the renderer
            vtkSmartPointer<vtkActorCollection> actors = this->CurrentRenderer->GetActors();

            // initialize traversal
            actors->InitTraversal();

            // loop through each actor to see if it's under the mouse click
            for (unsigned int i = 0; i < actors->GetNumberOfItems(); i++) {
                // get next actor
                vtkSmartPointer<vtkActor> actor = actors->GetNextActor();

                // get the bounds of that actor (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
                double *bounds = actor->GetBounds();

                // check if the click position is within the bounds
                if ((click[0] >= bounds[0] && click[0] <= bounds[1]) &&
                        click[1] >= bounds[2] && click[1] <= bounds[3]) {
                    // if so, actor is under the click
                    return actor;
                }
            }

            // if no actor was under the click, return null
            return NULL;
        }

        void CubeSelected() {
            // change flag depending on selected or not
            if (Cube) {
                // change back to false
                Cube = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Cube = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void DrawSelected() {
            // change flag depending on selected or not
            if (Draw) {
                // change back to false
                Draw = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Draw = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void RotateSelected() {
            // change flag depending on selected or not
            if (Rotate) {
                // change back to false
                Rotate = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Rotate = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void ZoomSelected() {
            // change flag depending on selected or not
            if (Zoom) {
                // change back to false
                Zoom = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Zoom = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void ScaleSelected() {
            // change flag depending on selected or not
            if (Scale) {
                // change back to false
                Scale = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Scale = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void StretchSelected() {
            // change flag depending on selected or not
            if (Stretch) {
                // change back to false
                Stretch = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Stretch = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void MoveSelected() {
            // change flag depending on selected or not
            if (Move) {
                // change back to false
                Move = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Move = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void PointSelected() {
            // change flag depending on selected or not
            if (Point) {
                // change back to false
                Point = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Point = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void OtherSelected() {
            // change flag depending on selected or not
            if (Other) {
                // change back to false
                Other = false;

                // change renderer to be gray
                ChangeRenderer(.86,.86,.86);
            } else {
                // change to true
                Other = true;

                // change renderer to be green
                ChangeRenderer(0,1,0);
            }
        }
        void RequestSelected() {}
        void OutputSelected() {}
        void ChangeRenderer(double r, double g, double b) {
            // based on rgb values, set background of current renderer
            this->CurrentRenderer->SetBackground(r, g, b);

            // reset camera
            this->CurrentRenderer->ResetCamera();

            // render the window to reflect this change
            this->Interactor->Render();
        }

        double *GetClickPosition() {
            // get x and y from mouse event
            int x = this->Interactor->GetEventPosition()[0];
            int y = this->Interactor->GetEventPosition()[1];


            // use picker to select correct pixel location
            this->Interactor->GetPicker()->Pick(x,
                                                y,
                                                0, // always zero
                                                this->CurrentRenderer);

            // get the pick position
            double *position = this->Interactor->GetPicker()->GetPickPosition();

            return position;
        }

        void DrawCubeOntoImage() {
            // get click position (x,y)
            double *position = GetClickPosition();

            // Create a cube
            vtkSmartPointer<vtkCubeSource> cubeSource =
              vtkSmartPointer<vtkCubeSource>::New();

            // set center based on click (z = 10)
            cubeSource->SetCenter(position[0], position[1], 10);

            // set basic length/width/height
            cubeSource->SetXLength(25.0);
            cubeSource->SetYLength(25.0);
            cubeSource->SetZLength(25.0);

            // map cube source to data set mapper
            vtkSmartPointer<vtkDataSetMapper> mapper =
              vtkSmartPointer<vtkDataSetMapper>::New();
            mapper->SetInputConnection(cubeSource->GetOutputPort());

            // create actor and set mapper
            vtkSmartPointer<vtkActor> actor =
              vtkSmartPointer<vtkActor>::New();
            actor->SetMapper(mapper);

            // add actor to the current renderer
            this->CurrentRenderer->AddActor(actor);

            // create CubeData object to store cube source and actor
            CubeData *cube = new CubeData();
            cube->actor = actor;
            cube->cubeSource = cubeSource;

            // add to vector of cubes
            cubes.push_back(cube);
        }

        CubeData *GetCube(vtkSmartPointer<vtkActor> actor) {
            // traverse through cubes and compare actors
            for (unsigned int i = 0; i < cubes.size(); i++) {
                if (cubes[i]->actor == actor) {
                    return cubes[i];
                }
            }

            // otherwise, return null
            return NULL;
        }

        // setter for renderer map
        void SetRendererMap(std::map<int,vtkRenderer*> map) { rendererMap = map; }

    private:
        // private vars
        bool Cube;
        bool Point;
        bool Other;
        bool Draw;
        bool Rotate;
        bool RotateZ;
        bool Selected;
        bool Scale;
        bool Move;
        bool Moving;
        bool Stretch;
        bool Zoom;

        // selected actor
        vtkSmartPointer<vtkActor> selectedActor;

        // map vtkRenderer addresses to integers for processing
        std::map<int,vtkRenderer*> rendererMap;

        // store vector of CubeData objects
        std::vector<CubeData*> cubes;
};

vtkStandardNewMacro(MyInteractorStyle);

vtkSmartPointer<vtkRenderer> CreateImageRenderer(vtkSmartPointer<vtkPNGReader> reader);
vtkSmartPointer<vtkRenderer> CreateTitleRenderer();
vtkSmartPointer<vtkRenderer> CreateObjectsRenderer();
vtkSmartPointer<vtkRenderer> CreateCubeButtonRenderer();
vtkSmartPointer<vtkRenderer> CreatePointButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateOtherButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateSceneRenderer();
vtkSmartPointer<vtkRenderer> CreateOperationsRenderer();
vtkSmartPointer<vtkRenderer> CreateDrawButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateScaleButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateZoomButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateStretchButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateRotateButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateMoveButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateRequestButtonRenderer();
vtkSmartPointer<vtkRenderer> CreateOutputButtonRenderer();
std::vector<vtkSmartPointer<vtkPNGReader>> GetReaders(int numImages);
void SetImageCamera(vtkSmartPointer<vtkPNGReader> reader, vtkSmartPointer<vtkRenderer> renderer);

int main(int argc, char *argv[])
{
    // check input arguments
    if ( argc != 2 )
    {
      std::cout << "Usage: " << argv[0]
                << " InputNumberOfImages" << std::endl;
      return EXIT_FAILURE;
    }

    // array for size of the window
    int size[2] = {1200, 800};

    // convert string to integer for number of images
    int numImages = atoi(argv[1]);

    // create vector of PNG readers to store images
    std::vector<vtkSmartPointer<vtkPNGReader>> readers = GetReaders(numImages);

    // create render window, set size and number of layers
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();

    renderWindow->SetNumberOfLayers(2);
    renderWindow->SetSize(size[0],size[1]);

    // create map object to map renderer addresses to integers
    std::map<int,vtkRenderer*> rendererMap;

    // create renderer for the cube button
    vtkSmartPointer<vtkRenderer> cubeRenderer = CreateCubeButtonRenderer();
    renderWindow->AddRenderer(cubeRenderer);
    rendererMap[0] = cubeRenderer;

    // create renderer for the point button
    vtkSmartPointer<vtkRenderer> pointRenderer = CreatePointButtonRenderer();
    renderWindow->AddRenderer(pointRenderer);
    rendererMap[1] = pointRenderer;

    // create renderer for the others button
    vtkSmartPointer<vtkRenderer> otherRenderer = CreateOtherButtonRenderer();
    renderWindow->AddRenderer(otherRenderer);
    rendererMap[2] = otherRenderer;

    // create renderer for the image (display first image for now)
    vtkSmartPointer<vtkRenderer> imageRenderer = CreateImageRenderer(readers[0]);
    renderWindow->AddRenderer(imageRenderer);
    rendererMap[3] = imageRenderer;

    // create renderer for the operations text
    vtkSmartPointer<vtkRenderer> operationsRenderer = CreateOperationsRenderer();
    renderWindow->AddRenderer(operationsRenderer);

    // create renderer for the title
    vtkSmartPointer<vtkRenderer> titleRenderer = CreateTitleRenderer();
    renderWindow->AddRenderer(titleRenderer);

    // create renderer for the objects text
    vtkSmartPointer<vtkRenderer> objectsRenderer = CreateObjectsRenderer();
    renderWindow->AddRenderer(objectsRenderer);

    // create renderer for the draw button
    vtkSmartPointer<vtkRenderer> drawRenderer = CreateDrawButtonRenderer();
    renderWindow->AddRenderer(drawRenderer);
    rendererMap[4] = drawRenderer;

    // create renderer for the zoom button
    vtkSmartPointer<vtkRenderer> zoomRenderer = CreateZoomButtonRenderer();
    renderWindow->AddRenderer(zoomRenderer);
    rendererMap[5] = zoomRenderer;

    // create renderer for the scale button
    vtkSmartPointer<vtkRenderer> scaleRenderer = CreateScaleButtonRenderer();
    renderWindow->AddRenderer(scaleRenderer);
    rendererMap[6] = scaleRenderer;

    // create renderer for the stretch button
    vtkSmartPointer<vtkRenderer> stretchRenderer = CreateStretchButtonRenderer();
    renderWindow->AddRenderer(stretchRenderer);
    rendererMap[7] = stretchRenderer;

    // create renderer for the rotate button
    vtkSmartPointer<vtkRenderer> rotateRenderer = CreateRotateButtonRenderer();
    renderWindow->AddRenderer(rotateRenderer);
    rendererMap[8] = rotateRenderer;

    // create renderer for the move button
    vtkSmartPointer<vtkRenderer> moveRenderer = CreateMoveButtonRenderer();
    renderWindow->AddRenderer(moveRenderer);
    rendererMap[9] = moveRenderer;

    // create renderer for the request button
    vtkSmartPointer<vtkRenderer> requestRenderer = CreateRequestButtonRenderer();
    renderWindow->AddRenderer(requestRenderer);
    rendererMap[10] = requestRenderer;

    // create renderer for the output button
    vtkSmartPointer<vtkRenderer> outputRenderer = CreateOutputButtonRenderer();
    renderWindow->AddRenderer(outputRenderer);
    rendererMap[11] = outputRenderer;

    // define viewports (in pixels)
    double xmin[15] = {0,0,200,400,600,0,800,800,800,800,800,800,800,800,800};
    double ymin[15] = {700,0,0,0,0,100,700,612.5,525,437.5,350,262.5,175,87.5,0};
    double xmax[15] = {800,200,400,600,800,800,1200,1200,1200,1200,1200,1200,1200,1200,1200};
    double ymax[15] = {800,100,100,100,100,700,800,700,612.5,525,437.5,350,262.5,175,87.5};

    // convert to (0,1) range
    for (unsigned int i = 0; i < 15; i++) {
        xmin[i] = xmin[i] / size[0];
        xmax[i] = xmax[i] / size[0];
        ymin[i] = ymin[i] / size[1];
        ymax[i] = ymax[i] / size[1];
    }

    // set the appropriate viewport for each renderer
    titleRenderer->SetViewport(xmin[0], ymin[0], xmax[0], ymax[0]);
    objectsRenderer->SetViewport(xmin[1], ymin[1], xmax[1], ymax[1]);
    cubeRenderer->SetViewport(xmin[2], ymin[2], xmax[2], ymax[2]);
    pointRenderer->SetViewport(xmin[3], ymin[3], xmax[3], ymax[3]);
    otherRenderer->SetViewport(xmin[4], ymin[4], xmax[4], ymax[4]);
    imageRenderer->SetViewport(xmin[5], ymin[5], xmax[5], ymax[5]);
    operationsRenderer->SetViewport(xmin[6], ymin[6], xmax[6], ymax[6]);
    drawRenderer->SetViewport(xmin[7], ymin[7], xmax[7], ymax[7]);
    zoomRenderer->SetViewport(xmin[8], ymin[8], xmax[8], ymax[8]);
    scaleRenderer->SetViewport(xmin[9], ymin[9], xmax[9], ymax[9]);
    stretchRenderer->SetViewport(xmin[10], ymin[10], xmax[10], ymax[10]);
    rotateRenderer->SetViewport(xmin[11], ymin[11], xmax[11], ymax[11]);
    moveRenderer->SetViewport(xmin[12], ymin[12], xmax[12], ymax[12]);
    requestRenderer->SetViewport(xmin[13], ymin[13], xmax[13], ymax[13]);
    outputRenderer->SetViewport(xmin[14], ymin[14], xmax[14], ymax[14]);

    // set colors of each renderer to gray (rgb = .86, .86, .86)
    titleRenderer->SetBackground(.86,.86,.86);
    imageRenderer->SetBackground(.86,.86,.86);
    objectsRenderer->SetBackground(.86,.86,.86);
    cubeRenderer->SetBackground(.86,.86,.86);
    pointRenderer->SetBackground(.86,.86,.86);
    otherRenderer->SetBackground(.86,.86,.86);
    operationsRenderer->SetBackground(.86,.86,.86);
    drawRenderer->SetBackground(.86,.86,.86);
    zoomRenderer->SetBackground(.86,.86,.86);
    scaleRenderer->SetBackground(.86,.86,.86);
    stretchRenderer->SetBackground(.86,.86,.86);
    rotateRenderer->SetBackground(.86,.86,.86);
    moveRenderer->SetBackground(.86,.86,.86);
    requestRenderer->SetBackground(.86,.86,.86);
    outputRenderer->SetBackground(.86,.86,.86);

    // create render window interactor and set render window
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();

    renderWindowInteractor->SetRenderWindow(renderWindow);

    // create interactor style to handle events
    vtkSmartPointer<MyInteractorStyle> style =
        vtkSmartPointer<MyInteractorStyle>::New();

    renderWindowInteractor->SetInteractorStyle(style);

    // set renderer map
    style->SetRendererMap(rendererMap);

    // render the window to figure out where image camera is
    renderWindow->Render();

    // fill image in the image renderer
    SetImageCamera(readers[0], imageRenderer);

    // re-render to reflect camera change
    renderWindow->Render();

    // start interactor
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}

std::vector<vtkSmartPointer<vtkPNGReader>> GetReaders(int numImages) {
    // create vector of PNG readers to store images
    std::vector<vtkSmartPointer<vtkPNGReader>> readers;

    // loop through each image to get the reader and add to vector
    for (unsigned int i = 13; i <= numImages; i++) {
        // NOTE: looping from 13 to skip all images of the same position

        // create PNG reader to read each file
        vtkSmartPointer<vtkPNGReader> reader =
            vtkSmartPointer<vtkPNGReader>::New();

        // get filename
        std::stringstream filename;
        filename << i << ".png";

        // read image by setting filename
        reader->SetFileName(filename.str().c_str());

        // update reader
        reader->Update();

        // push into vector of PNG reader objects
        readers.push_back(reader);
    }

    return readers;
}

void SetImageCamera(vtkSmartPointer<vtkPNGReader> reader, vtkSmartPointer<vtkRenderer> renderer) {
    // Set up the camera to fill the image renderer with the image
    vtkImageData *imageData = reader->GetOutput();

    // get origin/spacing/extent to set camera
    double origin[3];
    double spacing[3];
    int extent[6];
    imageData->GetOrigin( origin );
    imageData->GetSpacing( spacing );
    imageData->GetExtent( extent );

    // get active camera of the renderer
    vtkCamera* camera = renderer->GetActiveCamera();
    camera->ParallelProjectionOn();

    // calculate and set parallel scale/focal point/position of camera
    double xc = origin[0] + 0.5*(extent[0] + extent[1])*spacing[0];
    double yc = origin[1] + 0.5*(extent[2] + extent[3])*spacing[1];
    //double xd = (extent[1] - extent[0] + 1)*spacing[0];
    double yd = (extent[3] - extent[2] + 1)*spacing[1];
    double d = camera->GetDistance();
    camera->SetParallelScale(0.5*yd);
    camera->SetFocalPoint(xc,yc,0.0);
    camera->SetPosition(xc,yc,d);
}

vtkSmartPointer<vtkRenderer> CreateImageRenderer(vtkSmartPointer<vtkPNGReader> reader) {
    // put in its own image actor
    vtkSmartPointer<vtkImageActor> imageActor =
        vtkSmartPointer<vtkImageActor>::New();

    imageActor->GetMapper()->SetInputConnection(reader->GetOutputPort());

    // create renderer to hold image
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // add image actor to image renderer
    renderer->AddActor(imageActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateOperationsRenderer() {
    // create operations renderer to manipulate objects
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create operations in the top right corner
    vtkSmartPointer<vtkTextActor> operationsText =
        vtkSmartPointer<vtkTextActor>::New();

    // configure text
    operationsText->SetInput("Operations");
    operationsText->SetPosition(20,0);
    operationsText->GetTextProperty()->BoldOn();
    operationsText->GetTextProperty()->SetFontSize(30);
    operationsText->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer
    renderer->AddActor2D(operationsText);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateTitleRenderer() {
    // create renderers for the top part of the screen
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store title
    vtkSmartPointer<vtkTextActor> titleActor =
        vtkSmartPointer<vtkTextActor>::New();

    // configure title text
    titleActor->SetInput("BRIEF Modeling");
    titleActor->SetPosition(175, 10);
    titleActor->GetTextProperty()->BoldOn();
    titleActor->GetTextProperty()->SetFontSize(60);
    titleActor->GetTextProperty()->SetColor(0,0,0); // black

    // add actor to renderer and renderer to the window
    renderer->AddActor2D(titleActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateObjectsRenderer() {
    // create renderer for the bottom left part of the screen
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Objects" text
    vtkSmartPointer<vtkTextActor> objectsTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    // configure objects text
    objectsTextActor->SetInput("Objects:");
    objectsTextActor->SetPosition(10,15);
    objectsTextActor->GetTextProperty()->SetFontSize(20);
    objectsTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(objectsTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateCubeButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Cube" text
    vtkSmartPointer<vtkTextActor> cubeTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    // configure cube text
    cubeTextActor->SetInput("Cube");
    cubeTextActor->SetPosition(0,15);
    cubeTextActor->GetTextProperty()->SetFontSize(20);
    cubeTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(cubeTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreatePointButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Point" text
    vtkSmartPointer<vtkTextActor> pointTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    // configure cube text
    pointTextActor->SetInput("Point");
    pointTextActor->SetPosition(0,15);
    pointTextActor->GetTextProperty()->SetFontSize(20);
    pointTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(pointTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateOtherButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Other" text
    vtkSmartPointer<vtkTextActor> otherTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    otherTextActor->SetInput("Other");
    otherTextActor->SetPosition(0,15);
    otherTextActor->GetTextProperty()->SetFontSize(20);
    otherTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(otherTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateDrawButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Draw" text
    vtkSmartPointer<vtkTextActor> drawTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    drawTextActor->SetInput("Draw");
    drawTextActor->SetPosition(0,15);
    drawTextActor->GetTextProperty()->SetFontSize(20);
    drawTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(drawTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateScaleButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Scale" text
    vtkSmartPointer<vtkTextActor> scaleTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    scaleTextActor->SetInput("Scale");
    scaleTextActor->SetPosition(0,15);
    scaleTextActor->GetTextProperty()->SetFontSize(20);
    scaleTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(scaleTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateZoomButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Zoom" text
    vtkSmartPointer<vtkTextActor> zoomTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    zoomTextActor->SetInput("Zoom");
    zoomTextActor->SetPosition(0,15);
    zoomTextActor->GetTextProperty()->SetFontSize(20);
    zoomTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(zoomTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateStretchButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Stretch" text
    vtkSmartPointer<vtkTextActor> stretchTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    stretchTextActor->SetInput("Stretch");
    stretchTextActor->SetPosition(0,15);
    stretchTextActor->GetTextProperty()->SetFontSize(20);
    stretchTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(stretchTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateRotateButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Rotate" text
    vtkSmartPointer<vtkTextActor> rotateTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    rotateTextActor->SetInput("Rotate");
    rotateTextActor->SetPosition(0,15);
    rotateTextActor->GetTextProperty()->SetFontSize(20);
    rotateTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(rotateTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateMoveButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Move" text
    vtkSmartPointer<vtkTextActor> moveTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    moveTextActor->SetInput("Move");
    moveTextActor->SetPosition(0,15);
    moveTextActor->GetTextProperty()->SetFontSize(20);
    moveTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(moveTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateRequestButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Request" text
    vtkSmartPointer<vtkTextActor> requestTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    requestTextActor->SetInput("Request");
    requestTextActor->SetPosition(0,15);
    requestTextActor->GetTextProperty()->SetFontSize(20);
    requestTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(requestTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> CreateOutputButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Output" text
    vtkSmartPointer<vtkTextActor> outputTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    outputTextActor->SetInput("Output");
    outputTextActor->SetPosition(0,15);
    outputTextActor->GetTextProperty()->SetFontSize(20);
    outputTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(outputTextActor);

    return renderer;
}
