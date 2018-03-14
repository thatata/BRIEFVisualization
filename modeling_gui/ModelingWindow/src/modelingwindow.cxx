#include <vtkSmartPointer.h>
#include <vtkPNGReader.h>
#include <vector>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderWindow.h>

#include "modelingwindow.h"
#include "modelingwindowstyle.h"

ModelingWindow::ModelingWindow(int numImages) {
    // array for size of the window
    int size[2] = {1350, 700};

    // create vector of PNG readers to store images
    std::vector<vtkSmartPointer<vtkPNGReader>> readers = GetReaders(numImages);

    // create render window, set size and number of layers
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetSize(size[0],size[1]);

    // create map object to map renderer addresses to integers
    std::map<int,vtkRenderer*> rendererMap;

    // create renderer for the image (display first pose)
    vtkSmartPointer<vtkRenderer> imageRenderer = CreateImageRenderer(readers[0]);
    renderWindow->AddRenderer(imageRenderer);
    rendererMap[0] = imageRenderer;

    // create renderer for the draw button
    vtkSmartPointer<vtkRenderer> drawRenderer = CreateDrawButtonRenderer();
    renderWindow->AddRenderer(drawRenderer);
    rendererMap[1] = drawRenderer;

    // create renderer for the scale button
    vtkSmartPointer<vtkRenderer> scaleRenderer = CreateScaleButtonRenderer();
    renderWindow->AddRenderer(scaleRenderer);
    rendererMap[2] = scaleRenderer;

    // create renderer for the stretch button
    vtkSmartPointer<vtkRenderer> stretchRenderer = CreateStretchButtonRenderer();
    renderWindow->AddRenderer(stretchRenderer);
    rendererMap[3] = stretchRenderer;

    // create renderer for the rotate button
    vtkSmartPointer<vtkRenderer> rotateRenderer = CreateRotateButtonRenderer();
    renderWindow->AddRenderer(rotateRenderer);
    rendererMap[4] = rotateRenderer;

    // create renderer for the move button
    vtkSmartPointer<vtkRenderer> moveRenderer = CreateMoveButtonRenderer();
    renderWindow->AddRenderer(moveRenderer);
    rendererMap[5] = moveRenderer;

    // create renderer for the request button
    vtkSmartPointer<vtkRenderer> requestRenderer = CreateRequestButtonRenderer();
    renderWindow->AddRenderer(requestRenderer);
    rendererMap[6] = requestRenderer;

    // create renderer for the output button
    vtkSmartPointer<vtkRenderer> outputRenderer = CreateOutputButtonRenderer();
    renderWindow->AddRenderer(outputRenderer);
    rendererMap[7] = outputRenderer;

    // TODO: create renderer for each arrow key
    vtkSmartPointer<vtkRenderer> leftArrowRenderer = CreateLeftArrowButtonRenderer();
    renderWindow->AddRenderer(leftArrowRenderer);
    rendererMap[8] = leftArrowRenderer;

    vtkSmartPointer<vtkRenderer> rightArrowRenderer = CreateRightArrowButtonRenderer();
    renderWindow->AddRenderer(rightArrowRenderer);
    rendererMap[9] = rightArrowRenderer;

    // create renderer for the right image (empty for now)
    vtkSmartPointer<vtkRenderer> rightPoseRenderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(rightPoseRenderer);
    rendererMap[10] = rightPoseRenderer;

    // create renderer for the cube button
    vtkSmartPointer<vtkRenderer> cubeRenderer = CreateCubeButtonRenderer();
    renderWindow->AddRenderer(cubeRenderer);
    rendererMap[11] = cubeRenderer;

    // create renderer for the point button
    vtkSmartPointer<vtkRenderer> pointRenderer = CreatePointButtonRenderer();
    renderWindow->AddRenderer(pointRenderer);
    rendererMap[12] = pointRenderer;

    // create renderer for the others button
    vtkSmartPointer<vtkRenderer> otherRenderer = CreateOtherButtonRenderer();
    renderWindow->AddRenderer(otherRenderer);
    rendererMap[13] = otherRenderer;

    // create renderer for the operations text
    vtkSmartPointer<vtkRenderer> operationsRenderer = CreateOperationsRenderer();
    renderWindow->AddRenderer(operationsRenderer);

    // define viewports (in pixels)
    double xmin[15] = {0,600,600,600,600,600,600,600,600,600,675,750,600,650,700};
    double ymin[15] = {0,630,560,420,350,280,210,140,70,0,0,0,490,490,490};
    double xmax[15] = {600,750,750,750,750,750,750,750,750,675,750,1350,650,700,750};
    double ymax[15] = {700,700,630,560,420,350,280,210,140,70,70,700,560,560,560};

    // convert to (0,1) range
    for (unsigned int i = 0; i < 15; i++) {
        xmin[i] = xmin[i] / size[0];
        xmax[i] = xmax[i] / size[0];
        ymin[i] = ymin[i] / size[1];
        ymax[i] = ymax[i] / size[1];
    }

    // set the appropriate viewport for each renderer
    imageRenderer->SetViewport(xmin[0], ymin[0], xmax[0], ymax[0]);
    operationsRenderer->SetViewport(xmin[1], ymin[1], xmax[1], ymax[1]);
    drawRenderer->SetViewport(xmin[2], ymin[2], xmax[2], ymax[2]);
    scaleRenderer->SetViewport(xmin[3], ymin[3], xmax[3], ymax[3]);
    stretchRenderer->SetViewport(xmin[4], ymin[4], xmax[4], ymax[4]);
    rotateRenderer->SetViewport(xmin[5], ymin[5], xmax[5], ymax[5]);
    moveRenderer->SetViewport(xmin[6], ymin[6], xmax[6], ymax[6]);
    requestRenderer->SetViewport(xmin[7], ymin[7], xmax[7], ymax[7]);
    outputRenderer->SetViewport(xmin[8], ymin[8], xmax[8], ymax[8]);
    leftArrowRenderer->SetViewport(xmin[9], ymin[9], xmax[9], ymax[9]);
    rightArrowRenderer->SetViewport(xmin[10], ymin[10], xmax[10], ymax[10]);
    rightPoseRenderer->SetViewport(xmin[11], ymin[11], xmax[11], ymax[11]);
    cubeRenderer->SetViewport(xmin[12], ymin[12], xmax[12], ymax[12]);
    pointRenderer->SetViewport(xmin[13], ymin[13], xmax[13], ymax[13]);
    otherRenderer->SetViewport(xmin[14], ymin[14], xmax[14], ymax[14]);

    // set colors of each renderer to gray (rgb = .86, .86, .86)
    imageRenderer->SetBackground(.86,.86,.86);
    operationsRenderer->SetBackground(.86,.86,.86);
    drawRenderer->SetBackground(.86,.86,.86);
    scaleRenderer->SetBackground(.86,.86,.86);
    stretchRenderer->SetBackground(.86,.86,.86);
    rotateRenderer->SetBackground(.86,.86,.86);
    moveRenderer->SetBackground(.86,.86,.86);
    requestRenderer->SetBackground(.86,.86,.86);
    outputRenderer->SetBackground(.86,.86,.86);
    leftArrowRenderer->SetBackground(.86,.86,.86);
    rightArrowRenderer->SetBackground(.86,.86,.86);
    rightPoseRenderer->SetBackground(.86,.86,.86);
    cubeRenderer->SetBackground(.86,.86,.86);
    pointRenderer->SetBackground(.86,.86,.86);
    otherRenderer->SetBackground(.86,.86,.86);

    // initialize render window interactor and set render window
    renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // create interactor style to handle events
    vtkSmartPointer<ModelingWindowStyle> style =
        vtkSmartPointer<ModelingWindowStyle>::New();

    renderWindowInteractor->SetInteractorStyle(style);

    // set renderer map, readers, and current window
    style->SetRendererMap(rendererMap);
    style->SetReaders(readers);
    style->SetWindow(this);

    // render the window to figure out where image camera is
    renderWindow->Render();

    // fill image in the image renderer
    //SetImageCamera(readers[0], imageRenderer);

    // re-render to reflect camera change
    renderWindow->Render();

    // start interactor
    renderWindowInteractor->Start();
}

std::vector<vtkSmartPointer<vtkPNGReader>> ModelingWindow::GetReaders(int numImages) {
    // create vector of PNG readers to store images
    std::vector<vtkSmartPointer<vtkPNGReader>> readers;

    // loop through each image to get the reader and add to vector
    for (unsigned int i = 0; i <= numImages; i++) {
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

void ModelingWindow::SetImageCamera(vtkSmartPointer<vtkPNGReader> reader, vtkSmartPointer<vtkRenderer> renderer) {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateImageRenderer(vtkSmartPointer<vtkPNGReader> reader) {
    // put in its own image actor
    vtkSmartPointer<vtkImageActor> imageActor =
        vtkSmartPointer<vtkImageActor>::New();

    imageActor->GetMapper()->SetInputConnection(reader->GetOutputPort());

    // set image actor's z coordinate (z = -100)
    imageActor->SetPosition(imageActor->GetPosition()[0],
                            imageActor->GetPosition()[1],
                            -100);

    // create renderer to hold image
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // add image actor to image renderer
    renderer->AddActor(imageActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateOperationsRenderer() {
    // create operations renderer to manipulate objects
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create operations in the top right corner
    vtkSmartPointer<vtkTextActor> operationsText =
        vtkSmartPointer<vtkTextActor>::New();

    // configure text
    operationsText->SetInput("Operations");
    operationsText->SetPosition(0,0);
    operationsText->GetTextProperty()->BoldOn();
    operationsText->GetTextProperty()->SetFontSize(20);
    operationsText->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer
    renderer->AddActor2D(operationsText);

    return renderer;
}

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateTitleRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateObjectsRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateCubeButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Cube" text
    vtkSmartPointer<vtkTextActor> cubeTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    // configure cube text
    cubeTextActor->SetInput("Cube");
    cubeTextActor->SetPosition(0,15);
    cubeTextActor->GetTextProperty()->SetFontSize(15);
    cubeTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(cubeTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> ModelingWindow::CreatePointButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Point" text
    vtkSmartPointer<vtkTextActor> pointTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    // configure cube text
    pointTextActor->SetInput("Point");
    pointTextActor->SetPosition(0,15);
    pointTextActor->GetTextProperty()->SetFontSize(15);
    pointTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(pointTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateOtherButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "Other" text
    vtkSmartPointer<vtkTextActor> otherTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    otherTextActor->SetInput("Other");
    otherTextActor->SetPosition(0,15);
    otherTextActor->GetTextProperty()->SetFontSize(15);
    otherTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(otherTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateDrawButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateScaleButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateZoomButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateStretchButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateRotateButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateMoveButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateRequestButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateOutputButtonRenderer() {
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

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateLeftArrowButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store "<" text
    vtkSmartPointer<vtkTextActor> leftTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    leftTextActor->SetInput("<");
    leftTextActor->SetPosition(0,15);
    leftTextActor->GetTextProperty()->SetFontSize(20);
    leftTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(leftTextActor);

    return renderer;
}

vtkSmartPointer<vtkRenderer> ModelingWindow::CreateRightArrowButtonRenderer() {
    // create renderer
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();

    // create text actor to store ">" text
    vtkSmartPointer<vtkTextActor> rightTextActor =
        vtkSmartPointer<vtkTextActor>::New();

    rightTextActor->SetInput(">");
    rightTextActor->SetPosition(0,15);
    rightTextActor->GetTextProperty()->SetFontSize(20);
    rightTextActor->GetTextProperty()->SetColor(0,0,0);

    // add actor to renderer and renderer to window
    renderer->AddActor2D(rightTextActor);

    return renderer;
}
