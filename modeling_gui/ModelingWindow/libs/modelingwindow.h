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
struct PoseData;
class ModelingWindowStyle;

class ModelingWindow {
    public:
        // constructor for first window to open
        ModelingWindow(int numImages);

        // update window to dual-windowed
        void DualWindows(int newPose, PoseData *data);

        // store interactor as a public member
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;

        // store ptr to window attributes
        WindowStyleAttributes *attributes;

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
        vtkSmartPointer<vtkRenderer> CreateLeftArrowButtonRenderer();
        vtkSmartPointer<vtkRenderer> CreateRightArrowButtonRenderer();
};

#endif // MODELINGWINDOW_H
