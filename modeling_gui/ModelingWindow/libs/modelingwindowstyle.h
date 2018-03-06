#ifndef MODELINGWINDOWSTYLE_H
#define MODELINGWINDOWSTYLE_H

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDataSetMapper.h>
#include <map>
#include <vtkMatrix4x4.h>
#include <vtkPNGReader.h>
#include <vector>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkActorCollection.h>
#include <vtkProperty.h>
#include <vtkAbstractPicker.h>
#include <vtkImageMapper3D.h>
#include <sstream>

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

    // variables to store scale (start at 1), rotation, and translation values
    double scaleX = 1, scaleY = 1, scaleZ = 1, rotationX, rotationY, rotationZ, translateX, translateY;
};

struct PointData {
    // actor object
    vtkSmartPointer<vtkActor> actor;

    // polydata object
    vtkSmartPointer<vtkPolyData> polyData;

    // points object
    vtkSmartPointer<vtkPoints> points;

    // cell array object
    vtkSmartPointer<vtkCellArray> vertices;
};

struct WindowStyleAttributes {
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
    bool StretchZ;
    bool Zoom;

    // selected actor
    vtkSmartPointer<vtkActor> selectedActor;

    // map vtkRenderer addresses to integers for processing
    std::map<int,vtkRenderer*> rendererMap;

    // store vector of CubeData objects
    std::vector<CubeData*> cubes;

    // store vector of PointData objects
    std::vector<PointData*> points;

    // readers
    std::vector<vtkSmartPointer<vtkPNGReader>> readers;

    // next modeling window for a new pose
    ModelingWindow *window;
};

class ModelingWindowStyle : public vtkInteractorStyleTrackballActor {
    public:
        static ModelingWindowStyle* New();
        vtkTypeMacro(ModelingWindowStyle, vtkInteractorStyleTrackballActor);

        // Constructor to initialize private vars
        ModelingWindowStyle();

        // Mouse events
        void OnLeftButtonDown();
        void OnKeyPress();
        void OnLeftButtonUp();
        void OnMouseMove();

        // Operations
        void MoveCube();
        void RotateObject();
        void StretchObject();
        void ScaleObject();
        void DrawCubeOntoImage();
        void DrawPointOntoImage();
        void PerformTransformations(CubeData *data);

        // Utils
        vtkSmartPointer<vtkActor> GetActorUnderClick();
        void ChangeRenderer(double r, double g, double b);
        double *GetClickPosition();
        CubeData *GetCube(vtkSmartPointer<vtkActor> actor);
        void RequestNewPose();
        vtkSmartPointer<vtkMatrix4x4> GetMatrix(std::string fileName);
        void TransformEntities(int poseNum);

        // Button handling
        void PerformAction();
        void SceneSelected();
        void CubeSelected();
        void DrawSelected();
        void RotateSelected();
        void ZoomSelected();
        void ScaleSelected();
        void StretchSelected();
        void MoveSelected();
        void PointSelected();
        void OtherSelected();
        void RequestSelected();
        void OutputSelected();

        // Setter for renderer map
        void SetRendererMap(std::map<int,vtkRenderer*> map);
        void SetReaders(std::vector<vtkSmartPointer<vtkPNGReader>> pngReaders);
        void SetCubes(std::vector<CubeData*> cubes);
        void SetPoints(std::vector<PointData*> points);

    private:
        // private attributes stored in struct
        WindowStyleAttributes *attributes;
};

#endif // MODELINGWINDOWSTYLE_H
