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
#include <vtkSphereSource.h>
#include <sstream>

#include "modelingwindow.h"

// structure of an object holding actor, appropriate source objects, and transformation values
struct ObjectData {
    // pointer to actor object
    vtkSmartPointer<vtkActor> actor;

    // pointer to cube source object
    vtkSmartPointer<vtkCubeSource> cubeSource;

    // pointer to sphere source object
    vtkSmartPointer<vtkSphereSource> sphereSource;

    // transformation values for scale, rotation, translation
    double scaleX, scaleY, scaleZ, rotationX, rotationY, rotationZ, translateX, translateY;

    // pointer to the transform filter
    vtkSmartPointer<vtkTransformFilter> filter;

    // transformation matrix from snapping
    vtkSmartPointer<vtkMatrix4x4> snapMatrix;

    // constructor to initialize transformation values
    ObjectData() {
        // scale values start at 1
        scaleX = scaleY = scaleZ = 1;

        // all other values start at 0
        rotationX = rotationY = rotationZ = translateX = translateY = 0;
    }
};

// structure of a point
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

// structure of a pose, containing objects, pose #, and appropriate
// transformation matrices
struct PoseData {
    // store vector of ObjectData objects
    std::vector<ObjectData*> objects;

    // store vector of PointData objects
    std::vector<PointData*> points;

    // transformation matrix of the current pose
    vtkSmartPointer<vtkMatrix4x4> matrix;

    // pose number
    int poseNum;
};

struct WindowStyleAttributes {
    // private vars
    bool Cube;
    bool Point;
    bool Sphere;
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

    // vars for drawing a bounding box around the object/snapping
    bool DrawingBox;
    bool DrawBox;
    bool Snap;
    int PointsDrawn;
    int StartPosition[2];
    int EndPosition[2];
    vtkUnsignedCharArray *PixelArray;
    int StartBox[2];
    int EndBox[2];

    // value to determine which pose to zoom in/out
    int poseToZoom;

    // selected actor
    vtkSmartPointer<vtkActor> selectedActor;

    // map vtkRenderer addresses to integers for processing
    std::map<int,vtkRenderer*> rendererMap;

    // PNG readers
    std::vector<vtkSmartPointer<vtkPNGReader> > readers;
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
        void MoveObject();
        void RotateObject();
        void StretchObject();
        void ScaleObject();
        void DrawCubeOntoImage();
        void DrawCubeOntoImage(int *center, int sideLength);
        void DrawPointOntoImage();
        void DrawSphereOntoImage();
        void PerformTransformations(ObjectData *data);
        vtkSmartPointer<vtkTransform> GetTransform(ObjectData *data);
        vtkAlgorithmOutput *GetOutputPort(ObjectData *data);
        void CameraZoom(double factor);
        void ChangePose(int direction);
        void RequestNewPose();
        void Output();
        void Snap();

        // Utils
        vtkSmartPointer<vtkActor> GetActorUnderClick();
        void ChangeRenderer(double r, double g, double b);
        double *GetClickPosition();
        double *GetClickPosition(int x, int y);
        ObjectData *GetObject(vtkSmartPointer<vtkActor> actor);
        vtkSmartPointer<vtkMatrix4x4> GetMatrix(std::string fileName);
        void CreateNewPose(int newPose);
        void TransformEntities(PoseData *pose);
        void UpdateRightPoseImage(int newPose);
        void UpdateRightPoseEntities(PoseData *pose);
        void DeselectActor();
        void SelectActor(vtkSmartPointer<vtkActor> actor);
        void PromptSnapping();
        void PromptBoundingBox();
        void RedrawBoundingBox();

        // Button handling
        void PerformAction();
        void LeftSceneSelected();
        void RightSceneSelected();
        void CubeSelected();
        void DrawSelected();
        void RotateSelected();
        void ZoomSelected();
        void ScaleSelected();
        void StretchSelected();
        void MoveSelected();
        void PointSelected();
        void SphereSelected();
        void RequestSelected();
        void OutputSelected();
        void LeftArrowSelected();
        void RightArrowSelected();
        void ZoomInSelected();
        void ZoomOutSelected();

        // Setter for renderer map
        void SetRendererMap(std::map<int,vtkRenderer*> map);
        void SetReaders(std::vector<vtkSmartPointer<vtkPNGReader> > pngReaders);

    private:
        // window attributes
        WindowStyleAttributes *attributes;

        // vector of PoseData for each pose
        std::vector<PoseData*> poses;

        // current PoseData object
        PoseData *currentPose;

        // index of pose in the vector of poses
        int poseIdx;

        // index of pose displayed on the right image
        int rightPoseIdx;

        // pointer to right pose image actor
        vtkSmartPointer<vtkImageActor> rightPoseImage;
};

#endif // MODELINGWINDOWSTYLE_H
