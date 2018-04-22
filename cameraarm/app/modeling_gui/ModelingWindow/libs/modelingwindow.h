#ifndef MODELINGWINDOW_H
#define MODELINGWINDOW_H

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

struct WindowStyleAttributes;

class ModelingWindow {
    public:
        // constructor for first window to open
        ModelingWindow(int numImages);

        // store interactor as a public member
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;

        // store ptr to window attributes
        WindowStyleAttributes *attributes;

    private:
        std::vector< vtkSmartPointer<vtkPNGReader> > GetReaders(int numImages);
        vtkSmartPointer<vtkRenderer> CreateImageRenderer(vtkSmartPointer<vtkPNGReader> reader);
        vtkSmartPointer<vtkRenderer> CreateOperationsRenderer();
        vtkSmartPointer<vtkRenderer> CreateTitleRenderer();
        vtkSmartPointer<vtkRenderer> CreateObjectsRenderer();
        vtkSmartPointer<vtkRenderer> CreateCubeButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreatePointButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateSphereButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateDrawButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateScaleButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateStretchButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateRotateButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateMoveButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateRequestButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateOutputButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateLeftArrowButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateRightArrowButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateZoomButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateZoomInButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateZoomOutButtonRenderer();
};

#endif // MODELINGWINDOW_H
