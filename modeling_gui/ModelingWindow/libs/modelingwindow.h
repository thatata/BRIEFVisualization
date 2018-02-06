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

class ModelingWindow {
    public:
        ModelingWindow(int numImages);

        // store interactor as a public member
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;

    private:
        std::vector<vtkSmartPointer<vtkPNGReader>> GetReaders(int numImages);
        void SetImageCamera(vtkSmartPointer<vtkPNGReader> reader, vtkSmartPointer<vtkRenderer> renderer);
        vtkSmartPointer<vtkRenderer> CreateImageRenderer(vtkSmartPointer<vtkPNGReader> reader);
        vtkSmartPointer<vtkRenderer> CreateOperationsRenderer();
        vtkSmartPointer<vtkRenderer> CreateTitleRenderer();
        vtkSmartPointer<vtkRenderer> CreateObjectsRenderer();
        vtkSmartPointer<vtkRenderer> CreateCubeButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreatePointButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateOtherButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateDrawButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateScaleButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateZoomButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateStretchButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateRotateButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateMoveButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateRequestButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateOutputButtonRenderer();
};

#endif // MODELINGWINDOW_H
