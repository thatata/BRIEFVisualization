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

class MyInteractorStyle : public vtkInteractorStyleTrackballActor {
    public:
        static MyInteractorStyle* New();
        vtkTypeMacro(MyInteractorStyle, vtkInteractorStyleTrackballActor);

        MyInteractorStyle() {
            // set default values
            Cube = false;
            Draw = false;
            Rotate = false;
            Selected = false;
            Scale = false;
            Move = false;
            Stretch = false;

            // initialize selected actor
            selectedActor = vtkSmartPointer<vtkActor>::New();
        }

        virtual void OnLeftButtonDown() {
            // get position of the click
            int x = this->Interactor->GetEventPosition()[0];
            int y = this->Interactor->GetEventPosition()[1];

            // set the appropriate renderer based on the click position
            this->FindPokedRenderer(x, y);

            // perform operation based on the renderer selected
            PerformAction();

            // forward events
            //vtkInteractorStyleTrackballActor::OnLeftButtonDown();
        }

        void PerformAction() {
            // perform appropriate action based on which renderer is selected
            if (this->CurrentRenderer == rendererMap[0]) {
                // cube selected
                CubeSelected();
            } else if (this->CurrentRenderer == rendererMap[1]) {
                // other selected (future iteration)
            } else if (this->CurrentRenderer == rendererMap[2]) {
                // scene selected
                SceneSelected();
            } else if (this->CurrentRenderer == rendererMap[3]) {
                // draw selected
                DrawSelected();
            } else if (this->CurrentRenderer == rendererMap[4]) {
                // zoom selected
                ZoomSelected();
            } else if (this->CurrentRenderer == rendererMap[5]) {
                // scale selected
                ScaleSelected();
            } else if (this->CurrentRenderer == rendererMap[6]) {
                // stretch selected
                StretchSelected();
            } else if (this->CurrentRenderer == rendererMap[7]) {
                // rotate selected
                RotateSelected();
            } else if (this->CurrentRenderer == rendererMap[8]) {
                // move selected
                MoveSelected();
            } else if (this->CurrentRenderer == rendererMap[9]) {
                // request selected
                RequestSelected();
            } else if (this->CurrentRenderer == rendererMap[10]) {
                // output selected
                OutputSelected();
            }

            // otherwise, perform no action
        }

        void SceneSelected() {
            // check if draw is selected
            if (Draw) {
                // check if cube is selected, if so draw cube
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
                // if so, check if a manipulation tool was selected
                if (Rotate) {

                } else if (Scale) {

                } else if (Stretch) {

                } else if (Move) {

                } else {
                    // otherwise, deselect actor
                    selectedActor->GetProperty()->SetColor(1,1,1); // white

                    // change flag
                    Selected = false;

                    // re-initialize selectedActor
                    selectedActor = vtkSmartPointer<vtkActor>::New();

                    // re-render
                    this->Interactor->Render();
                }

            } else { // if not, check if user tried to select an object

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
                        // if so, actor has been selected
                        Selected = true;

                        // save address for future use
                        selectedActor = actor;

                        // change color to red to denote selected
                        actor->GetProperty()->SetColor(1,0,0); // red

                        // re-render
                        this->Interactor->Render();

                        // done, so return
                        return;
                    }

                }

                // if this is reached, none of the actors were under the mouse click
            }
            // otherwise, do nothing
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

        void ZoomSelected() {}
        void ScaleSelected() {}
        void StretchSelected() {}
        void MoveSelected() {}
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

            // set center based on click (z = 0)
            cubeSource->SetCenter(position[0], position[1], position[2]);

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
        }

        // setters
        void SetRendererMap(std::map<int,vtkRenderer*> map) { rendererMap = map; }

    private:
        // private vars
        bool Cube;
        bool Draw;
        bool Rotate;
        bool Selected;
        bool Scale;
        bool Move;
        bool Stretch;

        // selected actor
        vtkSmartPointer<vtkActor> selectedActor;

        // map vtkRenderer addresses to integers for processing
        std::map<int,vtkRenderer*> rendererMap;
};

vtkStandardNewMacro(MyInteractorStyle);

vtkSmartPointer<vtkRenderer> CreateImageRenderer(vtkSmartPointer<vtkPNGReader> reader);
vtkSmartPointer<vtkRenderer> CreateTitleRenderer();
vtkSmartPointer<vtkRenderer> CreateObjectsRenderer();
vtkSmartPointer<vtkRenderer> CreateCubeButtonRenderer();
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

    // create renderer for the others button
    vtkSmartPointer<vtkRenderer> otherRenderer = CreateOtherButtonRenderer();
    renderWindow->AddRenderer(otherRenderer);
    rendererMap[1] = otherRenderer;

    // create renderer for the image (display first image for now)
    vtkSmartPointer<vtkRenderer> imageRenderer = CreateImageRenderer(readers[0]);
    renderWindow->AddRenderer(imageRenderer);
    rendererMap[2] = imageRenderer;

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
    rendererMap[3] = drawRenderer;

    // create renderer for the zoom button
    vtkSmartPointer<vtkRenderer> zoomRenderer = CreateZoomButtonRenderer();
    renderWindow->AddRenderer(zoomRenderer);
    rendererMap[4] = zoomRenderer;

    // create renderer for the scale button
    vtkSmartPointer<vtkRenderer> scaleRenderer = CreateScaleButtonRenderer();
    renderWindow->AddRenderer(scaleRenderer);
    rendererMap[5] = scaleRenderer;

    // create renderer for the stretch button
    vtkSmartPointer<vtkRenderer> stretchRenderer = CreateStretchButtonRenderer();
    renderWindow->AddRenderer(stretchRenderer);
    rendererMap[6] = stretchRenderer;

    // create renderer for the rotate button
    vtkSmartPointer<vtkRenderer> rotateRenderer = CreateRotateButtonRenderer();
    renderWindow->AddRenderer(rotateRenderer);
    rendererMap[7] = rotateRenderer;

    // create renderer for the move button
    vtkSmartPointer<vtkRenderer> moveRenderer = CreateMoveButtonRenderer();
    renderWindow->AddRenderer(moveRenderer);
    rendererMap[8] = moveRenderer;

    // create renderer for the request button
    vtkSmartPointer<vtkRenderer> requestRenderer = CreateRequestButtonRenderer();
    renderWindow->AddRenderer(requestRenderer);
    rendererMap[9] = requestRenderer;

    // create renderer for the output button
    vtkSmartPointer<vtkRenderer> outputRenderer = CreateOutputButtonRenderer();
    renderWindow->AddRenderer(outputRenderer);
    rendererMap[10] = outputRenderer;

    // define viewports (in pixels)
    double xmin[15] = {0,0,200,400,0,0,800,800,800,800,800,800,800,800,800};
    double ymin[15] = {700,0,0,0,100,100,700,612.5,525,437.5,350,262.5,175,87.5,0};
    double xmax[15] = {800,200,400,800,800,800,1200,1200,1200,1200,1200,1200,1200,1200,1200};
    double ymax[15] = {800,100,100,100,700,700,800,700,612.5,525,437.5,350,262.5,175,87.5};

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
    otherRenderer->SetViewport(xmin[3], ymin[3], xmax[3], ymax[3]);
    imageRenderer->SetViewport(xmin[4], ymin[4], xmax[4], ymax[4]);
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
