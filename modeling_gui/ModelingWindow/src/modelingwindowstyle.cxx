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
#include "modelingwindowstyle.h"

/**
  * TH: Operations Overview
  *
  * Draw: after selecting an object (e.g. , point),
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
  * Request: select Request, and in the console enter the
  * pose number to pop up a new rendering window.
  *
  * Output: ??
  *
  **/

ModelingWindowStyle::ModelingWindowStyle() {
    // initialize struct for attributes
    attributes = new WindowStyleAttributes();

    // set default values
    attributes->Cube = false;
    attributes->Point = false;
    attributes->Other = false;
    attributes->Draw = false;
    attributes->Rotate = false;
    attributes->RotateZ = false;
    attributes->Selected = false;
    attributes->Scale = false;
    attributes->Move = false;
    attributes->Moving = false;
    attributes->Stretch = false;
    attributes->StretchZ = false;
    attributes->Zoom = false;

    // initialize PoseData object for initial pose
    PoseData *data = new PoseData();

    // set pose number
    data->poseNum = 0;

    // set current pose
    currentPose = data;

    // set current transformation matrix
    data->currMatrix = GetMatrix("0.txt");

    // set right pose index to -1 (right pose DNE)
    rightPoseIdx = -1;

    // add to vector of poses
    poses.push_back(data);
}

void ModelingWindowStyle::OnLeftButtonDown() {
    // get position of the click
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    // set the appropriate renderer based on the click position
    this->FindPokedRenderer(x, y);

    // perform operation based on the renderer selected
    PerformAction();
}

void ModelingWindowStyle::OnKeyPress() {
    // check if an object was selected
    if (attributes->Selected) {
        // if so, check for rotation, stretch, or scale
        if (attributes->Rotate) {
            // handle rotate operation
            RotateObject();
        } else if (attributes->Stretch) {
            // handle stretch operation
            StretchObject();
        } else if (attributes->Scale) {
            // handle scale operation
            ScaleObject();
        }
    }
}

void ModelingWindowStyle::OnLeftButtonUp() {
    // check if moving, if so finish
    if (attributes->Moving) {
        // end pan
        this->EndPan();

        // change moving to false
        attributes->Moving = false;

        // get cube data
        CubeData *data = GetCube(attributes->selectedActor);

        // update translation values (in world coordinates)
        data->translateX = attributes->selectedActor->GetPosition()[0];
        data->translateY = attributes->selectedActor->GetPosition()[1];
    }
}

void ModelingWindowStyle::OnMouseMove() {
    // check if an actor is not moving, if so just return
    if (!attributes->Moving) {
        return;
    }

    // otherwise, forward events
    vtkInteractorStyleTrackballActor::OnMouseMove();
}

void ModelingWindowStyle::MoveCube() {
    // check if selected actor was under the mouse click
    vtkSmartPointer<vtkActor> actor = GetActorUnderClick();

    // check if actor isn't null and is the selected actor
    if (actor != NULL && actor == attributes->selectedActor) {
        // if so, moving
        attributes->Moving = true;

        // start pan
        this->StartPan();

        // change interaction prop to the actor selected
        this->InteractionProp = attributes->selectedActor;
    }
}

void ModelingWindowStyle::RotateObject() {
    // grab key pressed
    std::string key = this->Interactor->GetKeySym();

    // check if z was pressed for z axis rotation
    if (key == "z") {
        // if so, switch flag
        attributes->RotateZ = !attributes->RotateZ;

        // alert user in the console
        std::cout << "Z Rotation " << (attributes->RotateZ ? "Enabled!" : "Disabled!")
                  << std::endl;

        return;
    }

    // get cube data to update rotation values
    CubeData *data = GetCube(attributes->selectedActor);

    // check if z rotation has been enabled
    if (attributes->RotateZ) {
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

void ModelingWindowStyle::StretchObject() {
    // grab key pressed
    std::string key = this->Interactor->GetKeySym();

    // check if z was pressed to stretch about z axis
    if (key == "z") {
        // if so, switch flag
        attributes->StretchZ = !attributes->StretchZ;

        // alert user in the console
        std::cout << "Z Stretch " << (attributes->StretchZ ? "Enabled!" : "Disabled!")
                  << std::endl;

        return;
    }

    // get cube data to update scale values
    CubeData *data = GetCube(attributes->selectedActor);

    // check if z rotation has been enabled
    if (attributes->StretchZ) {
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

void ModelingWindowStyle::ScaleObject() {
    // grab key pressed
    std::string key = this->Interactor->GetKeySym();

    // get cube data to update scale values
    CubeData *data = GetCube(attributes->selectedActor);

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

void ModelingWindowStyle::DrawCubeOntoImage() {
    // get click position (x,y)
    double *position = GetClickPosition();

    // Create a cube
    vtkSmartPointer<vtkCubeSource> cubeSource =
      vtkSmartPointer<vtkCubeSource>::New();

    // set center based on click (z = 0)
    cubeSource->SetCenter(position[0], position[1], 0);

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
    currentPose->cubes.push_back(cube);
}

void ModelingWindowStyle::DrawPointOntoImage() {
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

void ModelingWindowStyle::PerformTransformations(CubeData *data) {
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

    // check pose number for initial pose (0)
    if (currentPose->poseNum != 0) {
        // if pose number is not 0, need to concat transformation matrices
        transform->Concatenate(currentPose->prevMatrix);
        transform->PostMultiply();
        transform->Concatenate(currentPose->currMatrix);
    } // otherwise, no need

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
    this->attributes->selectedActor->SetMapper(mapper);

    // reset camera and update window
    this->Interactor->Render();
}

vtkSmartPointer<vtkActor> ModelingWindowStyle::GetActorUnderClick() {
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

void ModelingWindowStyle::ChangeRenderer(double r, double g, double b) {
    // based on rgb values, set background of current renderer
    this->CurrentRenderer->SetBackground(r, g, b);

    // reset camera
    this->CurrentRenderer->ResetCamera();

    // render the window to reflect this change
    this->Interactor->Render();
}

double *ModelingWindowStyle::GetClickPosition() {
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

CubeData *ModelingWindowStyle::GetCube(vtkSmartPointer<vtkActor> actor) {
    // traverse through cubes and compare actors
    for (unsigned int i = 0; i < currentPose->cubes.size(); i++) {
        if (currentPose->cubes[i]->actor == actor) {
            return currentPose->cubes[i];
        }
    }

    // otherwise, return null
    return NULL;
}

void ModelingWindowStyle::RequestNewPose() {
    // print the request string to the console
    std::cout << "New Pose Request!" << std::endl
              << "Please enter pose number: ";

    // get pose number from input
    int poseNum;
    std::cin >> poseNum;

    // check if pose has already been fetched
    for (unsigned int i = 0; i < poses.size(); i++) {
        // check each pose number in poses
        if (poses[i]->poseNum == poseNum) {
            // prompt user that pose already fetched
            std::cout << "Pose Already Requested!\nUse the arrow keys to toggle to the pose."
                      << std::endl;
        }
    }

    // otherwise, check if dual window exists (check if pose idx is invalid)
    if (rightPoseIdx != -1) {
        // if so, update image in the right pose renderer
        UpdateRightPoseWindow(poseNum);

        // transform entities for the new renderer
        TransformEntities(poseNum);
    } else {
        // create image actor to display new pose in right image renderer
        vtkSmartPointer<vtkImageActor> imageActor =
            vtkSmartPointer<vtkImageActor>::New();
        imageActor->GetMapper()->SetInputConnection(attributes->readers[poseNum]->GetOutputPort());

        // set image actor's z coordinate (z = -100)
        imageActor->SetPosition(imageActor->GetPosition()[0],
                                imageActor->GetPosition()[1],
                                -100);

        // add image actor to the right image renderer (i = 10)
        attributes->rendererMap[10]->AddActor(imageActor);

        // reset camera of that renderer
        attributes->rendererMap[10]->ResetCamera();

        // re-render to check
        this->Interactor->Render();

        // transform entities
        TransformEntities(poseNum);
    }

    // update right pose idx
    rightPoseIdx = poseNum;

    // reset camera of that renderer
    attributes->rendererMap[10]->ResetCamera();

    // re-render window
    this->Interactor->Render();
}

void ModelingWindowStyle::UpdateRightPoseWindow(int newPose) {
    // get actors in the right pose renderer
    vtkSmartPointer<vtkActorCollection> actors = attributes->rendererMap[10]->GetActors();

    // initialize traversal
    actors->InitTraversal();

    // loop through each actor to find the imageActor
    for (unsigned int i = 0; i < actors->GetNumberOfItems(); i++) {
        // get next actor
        vtkSmartPointer<vtkActor> actor = actors->GetNextActor();

        // check class name
        if (actor->GetClassName() == "vtkImageActor") {
            std::cout << "found image actor!" << std::endl;

            // set the mapper to the new pose
            actor->GetMapper()->SetInputConnection(attributes->readers[newPose]->GetOutputPort());
            return;
        }
    }
}

void ModelingWindowStyle::TransformEntities(int newPose) {
    // create new PoseData object
    PoseData *pose = new PoseData();

    // set pose number to newPose
    pose->poseNum = newPose;

    // get filename of new pose
    std::stringstream newPoseFile;
    newPoseFile << newPose << ".txt";

    // set current matrix of new pose
    pose->currMatrix = GetMatrix(newPoseFile.str().c_str());

    // set previous matrix of new pose to current matrix of current pose
    pose->prevMatrix = currentPose->currMatrix;

    // loop through each cube drawn in the first RF
    for (unsigned int i = 0; i < currentPose->cubes.size(); i++) {
        // create new CubeData object
        CubeData *data = new CubeData();

        // copy cube source and transformation values (except translation
        // values) to new CubeData object
        data->cubeSource = currentPose->cubes[i]->cubeSource;
        data->rotationX = currentPose->cubes[i]->rotationX;
        data->rotationY = currentPose->cubes[i]->rotationY;
        data->rotationZ = currentPose->cubes[i]->rotationZ;
        data->scaleX = currentPose->cubes[i]->scaleX;
        data->scaleY = currentPose->cubes[i]->scaleY;
        data->scaleZ = currentPose->cubes[i]->scaleZ;

        // initialize actor to be placed in right pose renderer
        data->actor = vtkSmartPointer<vtkActor>::New();

        // create transform object
        vtkSmartPointer<vtkTransform> transform =
            vtkSmartPointer<vtkTransform>::New();

        // get the center position of the cube from CubeData
        double *center = currentPose->cubes[i]->cubeSource->GetCenter();

        // apply initial translation to center the cube at the origin
        transform->Translate(center[0], center[1], center[2]);

        // apply rotations
        transform->RotateX(currentPose->cubes[i]->rotationX);
        transform->RotateY(currentPose->cubes[i]->rotationY);
        transform->RotateZ(currentPose->cubes[i]->rotationZ);

        // scale with appropriate values
        transform->Scale(currentPose->cubes[i]->scaleX, currentPose->cubes[i]->scaleY, currentPose->cubes[i]->scaleZ);

        // apply final translation
        transform->Translate(-center[0], -center[1], -center[2]);

        // apply matrices
        transform->Concatenate(pose->prevMatrix);
        transform->PostMultiply();
        transform->Concatenate(pose->currMatrix);

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

        // set mapper to actor of CubeData
        data->actor->SetMapper(mapper);

        // add actor to the right renderer (i = 10)
        attributes->rendererMap[10]->AddActor(data->actor);

        // add new CubeData to new PoseData
        pose->cubes.push_back(data);
    }

    // add new PoseData to vector of PoseData objects
    poses.push_back(pose);
}

vtkSmartPointer<vtkMatrix4x4> ModelingWindowStyle::GetMatrix(std::string fileName) {
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

void ModelingWindowStyle::PerformAction() {
    // perform appropriate action based on which renderer is selected
    if (this->CurrentRenderer == attributes->rendererMap[0]) {
        // left scene selected
        LeftSceneSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[1]) {
        // draw selected
        DrawSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[2]) {
        // scale selected
        ScaleSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[3]) {
        // stretch selected
        StretchSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[4]) {
        // rotate selected
        RotateSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[5]) {
        // move selected
        MoveSelected();
        // zoom selected
        ZoomSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[6]) {
        // request selected
        RequestSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[7]) {
        // output selected
        OutputSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[8]) {
        // left arrow selected
    } else if (this->CurrentRenderer == attributes->rendererMap[9]) {
        // right arrow selected
    } else if (this->CurrentRenderer == attributes->rendererMap[10]) {
        // right pose selected
        RightSceneSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[11]) {
        // cube selected
        CubeSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[12]) {
        // point selected
        PointSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[13]) {
        // other selected
        OtherSelected();
    }

    // otherwise, perform no action
}

void ModelingWindowStyle::LeftSceneSelected() {
    // check if draw is selected
    if (attributes->Draw) {
        // check if cube is selected, if so draw a cube
        if (attributes->Cube) {
            // if so, draw cube
            DrawCubeOntoImage();

            // re-render
            this->Interactor->Render();

            // done, so return
            return;
        } else if (attributes->Point) {
            // draw point
            DrawPointOntoImage();

            // re-render
            this->Interactor->Render();

            // done, so return
            return;
        }
    }

    // otherwise, check if object was selected
    if (attributes->Selected) {
        // if so, check if move tool was selected
        if (attributes->Move) {
            // if so, move cube
            MoveCube();
        } else {
            // otherwise, deselect the actor

            // change actor color to white
            attributes->selectedActor->GetProperty()->SetColor(1,1,1);

            // change flag
            attributes->Selected = false;

            // re-render
            this->Interactor->Render();

        }

    } else {
        // if no object was selected, check if user tried to select an object
        vtkSmartPointer<vtkActor> actor = GetActorUnderClick();

        // if actor is not null, user selected that actor
        if (actor != NULL) {
            // change flag
            attributes->Selected = true;

            // save address for future use
            attributes->selectedActor = actor;

            // change color to red to denote selected
            actor->GetProperty()->SetColor(1,0,0); // red

            // set opacity to 75%
            actor->GetProperty()->SetOpacity(0.75);

            // re-render to update color
            this->Interactor->Render();
        }
    }
}

void ModelingWindowStyle::RightSceneSelected() {
    // check if pose is displayed (do nothing if not)
    if (rightPoseIdx == -1) return;

    // find PoseData object for pose displayed
    for (unsigned int i = 0; i < poses.size(); i++) {
        // check pose number
        if (poses[i]->poseNum == rightPoseIdx) {
            // set as current pose
            currentPose = poses[i];

            // perform scene selected actions
            LeftSceneSelected();
        }
    }
}

void ModelingWindowStyle::CubeSelected() {
    // change flag depending on selected or not
    if (attributes->Cube) {
        // change back to false
        attributes->Cube = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Cube = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::DrawSelected() {
    // change flag depending on selected or not
    if (attributes->Draw) {
        // change back to false
        attributes->Draw = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Draw = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::RotateSelected() {
    // change flag depending on selected or not
    if (attributes->Rotate) {
        // change back to false
        attributes->Rotate = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Rotate = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::ZoomSelected() {
    // change flag depending on selected or not
    if (attributes->Zoom) {
        // change back to false
        attributes->Zoom = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Zoom = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::ScaleSelected() {
    // change flag depending on selected or not
    if (attributes->Scale) {
        // change back to false
        attributes->Scale = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Scale = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::StretchSelected() {
    // change flag depending on selected or not
    if (attributes->Stretch) {
        // change back to false
        attributes->Stretch = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Stretch = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::MoveSelected() {
    // change flag depending on selected or not
    if (attributes->Move) {
        // change back to false
        attributes->Move = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Move = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::PointSelected() {
    // change flag depending on selected or not
    if (attributes->Point) {
        // change back to false
        attributes->Point = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Point = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::OtherSelected() {
    // change flag depending on selected or not
    if (attributes->Other) {
        // change back to false
        attributes->Other = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Other = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::RequestSelected() {
    RequestNewPose();
}

void ModelingWindowStyle::OutputSelected() {}

//void ModelingWindowStyle::SetCurrentPose(int poseNum) { this->currentPose = poseNum; }

void ModelingWindowStyle::SetCubes(std::vector<CubeData *> cubes) { currentPose->cubes = cubes; }

//void ModelingWindowStyle::SetPoints(std::vector<PointData *> points) { attributes->points = points; }

void ModelingWindowStyle::SetWindow(ModelingWindow *window) { this->window = window; }

void ModelingWindowStyle::SetRendererMap(std::map<int, vtkRenderer *> map) { attributes->rendererMap = map; }

void ModelingWindowStyle::SetReaders(std::vector<vtkSmartPointer<vtkPNGReader> > pngReaders) { attributes->readers = pngReaders; }

//WindowStyleAttributes *ModelingWindowStyle::GetAttributes() { return attributes; }

//vtkStandardNewMacro(ModelingWindowStyle);
