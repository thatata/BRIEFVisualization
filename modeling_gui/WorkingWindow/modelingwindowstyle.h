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

// temporary image actor
vtkSmartPointer<vtkImageActor> daImage;

class ModelingWindowStyle : public vtkInteractorStyleTrackballActor {
    public:
        static ModelingWindowStyle* New();
        vtkTypeMacro(ModelingWindowStyle, vtkInteractorStyleTrackballActor);

        // Constructor to initialize private vars
        ModelingWindowStyle() {
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
            StretchZ = false;
            Zoom = false;
        }

        // Mouse events
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
                // if so, check for rotation, stretch, or scale
                if (Rotate) {
                    // handle rotate operation
                    RotateObject();
                } else if (Stretch) {
                    // handle stretch operation
                    StretchObject();
                } else if (Scale) {
                    // handle scale operation
                    ScaleObject();
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

        // Operations
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
        void RotateObject() {
            // grab key pressed
            std::string key = this->Interactor->GetKeySym();

            // check if z was pressed for z axis rotation
            if (key == "z") {
                // if so, switch flag
                RotateZ = !RotateZ;

                // alert user in the console
                std::cout << "Z Rotation " << (RotateZ ? "Enabled!" : "Disabled!")
                          << std::endl;

                return;
            }

            // get cube data to update rotation values
            CubeData *data = GetCube(selectedActor);

            // check if z rotation has been enabled
            if (RotateZ) {
                // check for left/right arrow keys only
                if (key == "Left") {
                    // rotate about z axis (+)
                    data->rotationZ += 5;
                } else if (key == "Right") {
                    // rotate about z axis (-)
                    data->rotationZ -= 5;
                } else {
                    // otherwise don't rotate and return
                    return;
                }
            } else {
                // based on arrow key, rotate in native direction
                if (key == "Left") {
                    // rotate about y axis (-)
                    data->rotationY -= 5;
                } else if (key == "Right") {
                    // rotate about y axis (+)
                    data->rotationY += 5;
                } else if (key == "Up") {
                    // rotate about x axis (-)
                    data->rotationX -= 5;
                } else if (key == "Down") {
                    // rotate about x axis (+)
                    data->rotationX += 5;
                } else {
                    // otherwise don't rotate and return
                    return;
                }
            }

            // perform transformations using cube data
            PerformTransformations(data);
        }
        void StretchObject() {
            // grab key pressed
            std::string key = this->Interactor->GetKeySym();

            // check if z was pressed to stretch about z axis
            if (key == "z") {
                // if so, switch flag
                StretchZ = !StretchZ;

                // alert user in the console
                std::cout << "Z Stretch " << (StretchZ ? "Enabled!" : "Disabled!")
                          << std::endl;

                return;
            }

            // get cube data to update scale values
            CubeData *data = GetCube(selectedActor);

            // check if z rotation has been enabled
            if (StretchZ) {
                // check for left/right arrow keys only
                if (key == "Left") {
                    // stretch z axis (-)
                    data->scaleZ -= .1;
                } else if (key == "Right") {
                    // stretch z axis (+)
                    data->scaleZ += .1;
                } else {
                    // otherwise don't stretch and return
                    return;
                }
            } else {
                // based on arrow key, scale the object in the native direction
                if (key == "Left") {
                    // stretch x axis (-)
                    data->scaleX -= .1;
                } else if (key == "Right") {
                    // stretch x axis (+)
                    data->scaleX += .1;
                } else if (key == "Up") {
                    // stretch y axis (+)
                    data->scaleY += .1;
                } else if (key == "Down") {
                    // stretch y axis (-)
                    data->scaleY -= .1;
                } else {
                    // otherwise don't stretch and return
                    return;
                }
            }

            // rotate cube using axis and angle
            PerformTransformations(data);
        }
        void ScaleObject() {
            // grab key pressed
            std::string key = this->Interactor->GetKeySym();

            // get cube data to update scale values
            CubeData *data = GetCube(selectedActor);

            // scale object equally for each axis for up/down arrows
            if (key == "Up") {
                // scale +10%
                data->scaleX += .1;
                data->scaleY += .1;
                data->scaleZ += .1;
            } else if (key == "Down") {
                // scale -10%
                data->scaleX -= .1;
                data->scaleY -= .1;
                data->scaleZ -= .1;
            } else {
                // otherwise don't scale and return
                return;
            }

            // rotate cube using axis and angle
            PerformTransformations(data);
        }
        void DrawCubeOntoImage() {
            // get click position (x,y)
            double *position = GetClickPosition();

            // Create a cube
            vtkSmartPointer<vtkCubeSource> cubeSource =
              vtkSmartPointer<vtkCubeSource>::New();

            // set center based on click (z = 100)
            cubeSource->SetCenter(position[0], position[1], 100);

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
        void DrawPointOntoImage() {
            // create PointData object
            PointData *data = new PointData();

            // get click position
            double *position = GetClickPosition();

            // initialize points and insert point
            data->points = vtkSmartPointer<vtkPoints>::New();

            vtkIdType pid[1];
            //pid[0] = data->points->InsertNextPoint(x,y,10);
            pid[0] = data->points->InsertNextPoint(position);

            // initialize vertices and insert next cell
            data->vertices = vtkSmartPointer<vtkCellArray>::New();
            data->vertices->InsertNextCell(1,pid);

            // initialize polyData object and assign vertices/points
            data->polyData = vtkSmartPointer<vtkPolyData>::New();
            data->polyData->SetPoints(data->points);
            data->polyData->SetVerts(data->vertices);

            // map poly data to its own mapper
            vtkSmartPointer<vtkPolyDataMapper> mapper =
                vtkSmartPointer<vtkPolyDataMapper>::New();
            mapper->SetInputData(data->polyData);

            // initialize actor and set mapper
            data->actor = vtkSmartPointer<vtkActor>::New();
            data->actor->SetMapper(mapper);
            data->actor->GetProperty()->SetPointSize(10);

            // add actor to the current renderer
            this->CurrentRenderer->AddActor(data->actor);
        }
        void PerformTransformations(CubeData *data) {
            // create transform object
            vtkSmartPointer<vtkTransform> transform =
                vtkSmartPointer<vtkTransform>::New();

            // get the center position of the cube from CubeData
            double *center = data->cubeSource->GetCenter();

            // apply initial translation to center the cube at the origin
            transform->Translate(center[0], center[1], center[2]);

            // apply rotations
            transform->RotateX(data->rotationX);
            transform->RotateY(data->rotationY);
            transform->RotateZ(data->rotationZ);

            // scale with appropriate values
            transform->Scale(data->scaleX, data->scaleY, data->scaleZ);

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

        // Utils
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

        void RequestNewPose() {
            // print the request string to the console
            std::cout << "New Pose Request!" << std::endl
                      << "Please enter pose number: ";

            // get pose number from input
            int poseNum;
            std::cin >> poseNum;

            // STEPS:
            // 1. update the image to reflect new pose
            // get all actors in the scene
            daImage->GetMapper()->SetInputConnection(readers[poseNum]->GetOutputPort());

            // 2. transform cube based on T matrices
            // get cube data
            CubeData *data = GetCube(selectedActor);

            // create transform object
            vtkSmartPointer<vtkTransform> transform =
                vtkSmartPointer<vtkTransform>::New();

            // get the center position of the cube from CubeData
            double *center = data->cubeSource->GetCenter();

            // apply initial translation to center the cube at the origin
            transform->Translate(center[0], center[1], center[2]);

            // apply rotations
            transform->RotateX(data->rotationX);
            transform->RotateY(data->rotationY);
            transform->RotateZ(data->rotationZ);

            // scale with appropriate values
            transform->Scale(data->scaleX, data->scaleY, data->scaleZ);

            // apply final translation
            transform->Translate(-center[0], -center[1], -center[2]);

            // get filename of pose
            std::stringstream filename;
            filename << poseNum << ".txt";

            // get final matrix
            vtkSmartPointer<vtkMatrix4x4> initialMatrix = GetMatrix(filename.str().c_str());

            // get initial matrix
            vtkSmartPointer<vtkMatrix4x4> finalMatrix = GetMatrix("0.txt");

            transform->Concatenate(finalMatrix);
            transform->PostMultiply();
            transform->Concatenate(initialMatrix);

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

            this->Interactor->Render();
        }

        vtkSmartPointer<vtkMatrix4x4> GetMatrix(std::string fileName) {
            // vars to read text files
            std::string line;
            ifstream file (fileName);

            // vars to represent t vector and r matrix
            double tVector[3];
            double rMatrix[3][3];

            // local vars for parsing
            int lineCount = 0;
            int tIdx = 0;
            int rIdxRow = 0;
            int rIdxCol = 0;

            // parse through text file
            if (file.is_open()) {
                while (std::getline(file, line)) {
                    // check for t vector
                    if (lineCount > 0 && lineCount < 4) {
                        // get float from string
                        tVector[tIdx] = atof(line.c_str());
                        tIdx++;
                    }

                    // check for r matrix
                    else if (lineCount > 5 && lineCount < 9) {
                        // loop through characters of the string
                        // to get each element of the row

                        int startIdx = 0;
                        int length = 0;
                        for (unsigned int i = 0; i < line.length(); i++) {
                            if (line.at(i) == ' ' || i == line.length() - 1) {
                                // compute the length of the string from the start index
                                length = i - startIdx + 1;

                                // get resulting substring to check validity
                                std::string result = line.substr(startIdx, length);

                                // check if string is just " " or "  "
                                if (result == " " || result == "  ") {
                                    // if so, change start idx and continue
                                    startIdx = i;
                                    continue;
                                }

                                // get float value and assign to r matrix
                                rMatrix[rIdxRow][rIdxCol] = atof(line.substr(startIdx, length).c_str());

                                // change start idx to current index
                                startIdx = i;

                                // check if end of row is reached
                                if (rIdxCol == 2) {
                                    // if so, reset column idx and increment row idx
                                    rIdxCol = 0;
                                    rIdxRow++;
                                } else {
                                    // otherwise, just increment column idx
                                    rIdxCol++;
                                }
                            }
                        }
                    }

                    // increment line count
                    lineCount++;
                }
            } else cout << "Unable to open file";

            // create 4x4 matrix to represent Transformation Matrix
            vtkSmartPointer<vtkMatrix4x4> matrix =
                vtkSmartPointer<vtkMatrix4x4>::New();

            // use r matrix and t vector to fill out 4x4 matrix
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    // check if the last column is reached
                    if (i == 3) {
                        // set last line for homogeneous coords
                        matrix->SetElement(i, 0, 0);
                        matrix->SetElement(i, 1, 0);
                        matrix->SetElement(i, 2, 0);
                        matrix->SetElement(i, 3, 1);
                        break;
                    }

                    // check if columns should be r matrix or t vector
                    if (j < 3) {
                        // r matrix
                        matrix->SetElement(i, j, rMatrix[i][j]);
                    } else {
                        // t vector
                        matrix->SetElement(i, j, tVector[i]);
                    }
                }
            }

            return matrix;
        }

        // Button handling
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
                } else if (Point) {
                    // draw point
                    DrawPointOntoImage();

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

                    // set opacity to 75%
                    actor->GetProperty()->SetOpacity(0.75);

                    // re-render to update color
                    this->Interactor->Render();
                }
            }
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
        void RequestSelected() {
            RequestNewPose();
        }
        void OutputSelected() {}

        // Setter for renderer map
        void SetRendererMap(std::map<int,vtkRenderer*> map) { rendererMap = map; }
        void SetReaders(std::vector<vtkSmartPointer<vtkPNGReader>> pngReaders) { readers = pngReaders; }

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
};

vtkStandardNewMacro(ModelingWindowStyle);

#endif // MODELINGWINDOWSTYLE_H
