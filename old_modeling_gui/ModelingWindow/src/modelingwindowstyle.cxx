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
#include <vtkPLYWriter.h>
#include <vtkSimplePointsWriter.h>
#include <vtkSTLWriter.h>
#include <sstream>
#include <iostream>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkInteractorStyleRubberBandPick.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/thread/thread.hpp>
#include <pcl/io/pcd_io.h>

#include "modelingwindow.h"
#include "modelingwindowstyle.h"

ModelingWindowStyle::ModelingWindowStyle() {
    // initialize struct for attributes
    attributes = new WindowStyleAttributes();

    // set default values
    attributes->Cube = false;
    attributes->Point = false;
    attributes->Sphere = false;
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

    // set values for bounding box vars
    attributes->DrawBox = false;
    attributes->DrawingBox = false;
    attributes->Snap = false;
    attributes->PointsDrawn = 0;
    attributes->StartPosition[0] = attributes->StartPosition[1] = 0;
    attributes->EndPosition[0] = attributes->EndPosition[1] = 0;
    attributes->PixelArray = vtkUnsignedCharArray::New();

    // set default value for pose to zoom (-1)
    attributes->poseToZoom = -1;

    // initialize PoseData object for initial pose
    PoseData *data = new PoseData();

    // set pose number
    data->poseNum = 0;

    // set current pose
    currentPose = data;

    // set current transformation matrix
    data->matrix = GetMatrix("0.txt");

    // set pose idx to 0 (first element in the vector)
    poseIdx = 0;

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
    // check if user was drawing bounding box
    if (attributes->DrawingBox) {
        // if so, end drawing
        attributes->DrawingBox = false;

        // if start and end positions aren't the same, snap!
        if ((attributes->StartPosition[0] != attributes->EndPosition[0]) ||
            (attributes->StartPosition[1] != attributes->EndPosition[1])) {
            // set flag to true
            attributes->Snap = true;

            // save start and end positions (in screen coords) for snapping
            attributes->StartBox[0] = attributes->StartPosition[0];
            attributes->StartBox[1] = attributes->StartPosition[1];
            attributes->EndBox[0] = attributes->EndPosition[0];
            attributes->EndBox[1] = attributes->EndPosition[1];

            // prompt user to identify a plane on the object, and click on
            // (1) the bottom left corner
            std::cout << "\n----LET'S SNAP!---- \n\nIdentify a plane on the object to model, "
                      << "and click on the bottom left corner to start." << std::endl;
        }

        // remove bounding box on the window
        attributes->StartPosition[0] = attributes->StartPosition[1] = 0;
        attributes->EndPosition[0] = attributes->EndPosition[1] = 0;
        RedrawBoundingBox();

        // change draw flag since the bounding box only happens once
        attributes->DrawBox = false;

        return;
    }

    // check if moving, if so finish
    if (attributes->Moving) {
        // end pan
        this->EndPan();

        // change moving to false
        attributes->Moving = false;

        // get object data
        ObjectData *data = GetObject(attributes->selectedActor);

        // update translation values (in screen coordinates)
        double *screenCoords = GetClickPosition();
        data->translateX = screenCoords[0];
        data->translateY = screenCoords[1];
    }
}

void ModelingWindowStyle::OnMouseMove() {
    // check if user is drawing bounding box
    if (!attributes->DrawingBox) {
        // if not, let parent class handle it
        this->Superclass::OnMouseMove();

        return;
    }

    // otherwise, update end position coordinates
    attributes->EndPosition[0] = this->Interactor->GetEventPosition()[0];
    attributes->EndPosition[1] = this->Interactor->GetEventPosition()[1];
    int *size = this->Interactor->GetRenderWindow()->GetSize();

    // update position values depending on size of the render window
    if (attributes->EndPosition[0] > (size[0]-1)) {
        attributes->EndPosition[0] = size[0]-1;
    }
    if (attributes->EndPosition[0] < 0) {
        attributes->EndPosition[0] = 0;
    }
    if (attributes->EndPosition[1] > (size[1]-1)) {
        attributes->EndPosition[1] = size[1]-1;
    }
    if (attributes->EndPosition[1] < 0) {
        attributes->EndPosition[1] = 0;
    }

    // redraw bounding box
    RedrawBoundingBox();
}

void ModelingWindowStyle::PromptSnapping() {
    // prompt user if they'd like to perform snapping on the cube
    std::cout << "Cube Selected! \n\nWould you like to perform snapping (0 = no, 1 = yes)? ";

    // get the response from the console
    int response;
    std::cin >> response;

    // perform based on response
    if (response == 1) {
        // set snap attribute flag to true
        attributes->Snap = true;

        // snap! --> prompt bounding box
        PromptBoundingBox();
    } else {
        // otherwise, tell user to draw manually
        std::cout << "\nNo snapping! :(\n\nUse the manipulation tools to draw and manipulate the objects manually."
                  << std::endl;
    }
}

void ModelingWindowStyle::PromptBoundingBox() {
    // prompt user to draw a bounding box around the cube to model
    std::cout << "\nSnapping Selected! \n\nSelect the cube to model by click and dragging a box around the cube."
              << std::endl;

    // set bounding box flag to true
    attributes->DrawBox = true;
}

void ModelingWindowStyle::RedrawBoundingBox() {
    // update the rubber band on the screen
    int *size = this->Interactor->GetRenderWindow()->GetSize();

    // copy the original pixel array
    vtkUnsignedCharArray *tmpPixelArray = vtkUnsignedCharArray::New();
    tmpPixelArray->DeepCopy(attributes->PixelArray);
    unsigned char *pixels = tmpPixelArray->GetPointer(0);

    // get the max coordinate for the x coordinate of the pixel array
    int min[2], max[2];
    min[0] = attributes->StartPosition[0] <= attributes->EndPosition[0] ?
             attributes->StartPosition[0] : attributes->EndPosition[0];

    // check if min is out of bounds
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 1; }

    // repeat process for y coordinate
    min[1] = attributes->StartPosition[1] <= attributes->EndPosition[1] ?
             attributes->StartPosition[1] : attributes->EndPosition[1];

    // check OOB
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 1; }

    // repeat process for maximum coordinate
    max[0] = attributes->EndPosition[0] > attributes->StartPosition[0] ?
             attributes->EndPosition[0] : attributes->StartPosition[0];

    // check OOB
    if (max[0] < 0) { max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 1; }

    // repeat for y
    max[1] = attributes->EndPosition[1] > attributes->StartPosition[1] ?
             attributes->EndPosition[1] : attributes->StartPosition[1];

    if (max[1] < 0) { max[1] = 0; }
    if (max[1] >= size[1]) { max[1] = size[1] - 1; }

    // loop through minimum and maximum coordinates for x/y to update/redraw pixel array
    int i;
    for (i = min[0]; i <= max[0]; i++) {
        pixels[4*(min[1]*size[0]+i)] = 255 ^ pixels[4*(min[1]*size[0]+i)];
        pixels[4*(min[1]*size[0]+i)+1] = 255 ^ pixels[4*(min[1]*size[0]+i)+1];
        pixels[4*(min[1]*size[0]+i)+2] = 255 ^ pixels[4*(min[1]*size[0]+i)+2];
        pixels[4*(max[1]*size[0]+i)] = 255 ^ pixels[4*(max[1]*size[0]+i)];
        pixels[4*(max[1]*size[0]+i)+1] = 255 ^ pixels[4*(max[1]*size[0]+i)+1];
        pixels[4*(max[1]*size[0]+i)+2] = 255 ^ pixels[4*(max[1]*size[0]+i)+2];
    }
    for (i = min[1]+1; i < max[1]; i++)
    {
        pixels[4*(i*size[0]+min[0])] = 255 ^ pixels[4*(i*size[0]+min[0])];
        pixels[4*(i*size[0]+min[0])+1] = 255 ^ pixels[4*(i*size[0]+min[0])+1];
        pixels[4*(i*size[0]+min[0])+2] = 255 ^ pixels[4*(i*size[0]+min[0])+2];
        pixels[4*(i*size[0]+max[0])] = 255 ^ pixels[4*(i*size[0]+max[0])];
        pixels[4*(i*size[0]+max[0])+1] = 255 ^ pixels[4*(i*size[0]+max[0])+1];
        pixels[4*(i*size[0]+max[0])+2] = 255 ^ pixels[4*(i*size[0]+max[0])+2];
    }

    // set proper pixel data and update
    this->Interactor->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0]-1, size[1]-1, pixels, 0);
    this->Interactor->GetRenderWindow()->Frame();

    tmpPixelArray->Delete();
}

void ModelingWindowStyle::Snap() {
    // IMPLEMENTING KABSCH ALGORITHM TO FIND OPTIMAL ROTATION BETWEEN SETS OF POINTS

    // STEP 1: get center x,y position (in sc) of the new cube from the bounding box positions (z = 0)

    // find lower left, upper right corners and center (make sure bounding box doesn't
    // exceed the render window dimensions)
    int center[2];
    int *size = this->Interactor->GetRenderWindow()->GetSize();
    int min[2], max[2];

    // get min x,y values
    min[0] = attributes->StartBox[0] <= attributes->EndBox[0] ?
      attributes->StartBox[0] : attributes->EndBox[0];
    if (min[0] < 0) { min[0] = 0; }
    if (min[0] >= size[0]) { min[0] = size[0] - 2; }

    min[1] = attributes->StartBox[1] <= attributes->EndBox[1] ?
      attributes->StartBox[1] : attributes->EndBox[1];
    if (min[1] < 0) { min[1] = 0; }
    if (min[1] >= size[1]) { min[1] = size[1] - 2; }

    // get max x,y values
    max[0] = attributes->EndBox[0] > attributes->StartBox[0] ?
      attributes->EndBox[0] : attributes->StartBox[0];
    if (max[0] < 0) { max[0] = 0; }
    if (max[0] >= size[0]) { max[0] = size[0] - 2; }

    max[1] = attributes->EndBox[1] > attributes->StartBox[1] ?
      attributes->EndBox[1] : attributes->StartBox[1];
    if (max[1] < 0) { max[1] = 0; }
    if (max[1] >= size[1]) { max[1] = size[1] - 2; }

    // set center position by averaging min/max values
    center[0] = (min[0] + max[0])/2;
    center[1] = (min[1] + max[1])/2;

    // STEP 2: calculate the side length of the cube (use shortest x or y length based on center)
    int sideLength = (max[0] - min[0]) > (max[1] - min[1]) ?
                    (max[1] - min[1]) : (max[0] - min[0]);

    // STEP 3: draw a cube at the center with side lengths
    DrawCubeOntoImage(center, sideLength);

    // STEP 4: get the bounds of the actor of the drawn cube to calculate the
    // corresponding corners of the front face of the cube
    double *bounds = currentPose->objects[0]->actor->GetBounds();

    // Note: bounds are (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax)

    // get the bottom left corner coordinates (xmin,ymin,zmax)
    double blCube[3];
    blCube[0] = bounds[0];
    blCube[1] = bounds[2];
    blCube[2] = bounds[5];

    // get the top left corner coordinates (xmin,ymax,zmax)
    double tlCube[3];
    tlCube[0] = bounds[0];
    tlCube[1] = bounds[3];
    tlCube[2] = bounds[5];

    // get the top right corner coordinates (xmax,ymax,zmax)
    double trCube[3];
    trCube[0] = bounds[1];
    trCube[1] = bounds[3];
    trCube[2] = bounds[5];

    // get the bottom right corner coordinates (xmax,ymin,zmax)
    double brCube[3];
    brCube[0] = bounds[1];
    brCube[1] = bounds[2];
    brCube[2] = bounds[5];

    // STEP 5: get x,y data from the points drawn

    // first point drawn is bottom left corner (idx = 0)
    double blDrawn[3];
    currentPose->points[0]->points->GetPoint(0, blDrawn);

    // second point drawn is top left corner (idx = 1)
    double tlDrawn[3];
    currentPose->points[1]->points->GetPoint(0, tlDrawn);

    // third point drawn is top right corner (idx = 2)
    double trDrawn[3];
    currentPose->points[2]->points->GetPoint(0, trDrawn);

    // fourth point drawn is the bottom right corner (idx = 3)
    double brDrawn[3];
    currentPose->points[3]->points->GetPoint(0, brDrawn);

    // STEP 6: get the depth value from the point cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr point_cloud_ptr (new pcl::PointCloud<pcl::PointXYZ>());
    pcl::PointCloud<pcl::PointXYZ>& point_cloud = *point_cloud_ptr;

    // build the filename of the PCD
    std::stringstream filename;

    if (currentPose->poseNum < 10) {
        filename << "000" << currentPose->poseNum << ".pcd";
    } else {
        filename << "00" << currentPose->poseNum << ".pcd";
    }

    // attempt to load the PCD
    if (pcl::io::loadPCDFile(filename.str().c_str(), point_cloud) == -1) {
        // error
        return;
    }

    // use the x,y values of the drawn points to get the corresponding point in the point cloud
//    pcl::PointXYZ bl = point_cloud.at(blDrawn[0], blDrawn[1]);
//    pcl::PointXYZ tl = point_cloud.at(tlDrawn[0], tlDrawn[1]);
//    pcl::PointXYZ tr = point_cloud.at(trDrawn[0], trDrawn[1]);
//    pcl::PointXYZ br = point_cloud.at(brDrawn[0], brDrawn[1]);

    pcl::PointXYZ bl = point_cloud.at(106166);
    pcl::PointXYZ tl = point_cloud.at(76971);
    pcl::PointXYZ tr = point_cloud.at(98950);
    pcl::PointXYZ br = point_cloud.at(127512);

    // set z values for each drawn point
    blDrawn[0] = bl.x;
    tlDrawn[0] = tl.x;
    trDrawn[0] = tr.x;
    brDrawn[0] = br.x;

    blDrawn[1] = bl.y;
    tlDrawn[1] = tl.y;
    trDrawn[1] = tr.y;
    brDrawn[1] = br.y;

    blDrawn[2] = bl.z;
    tlDrawn[2] = tl.z;
    trDrawn[2] = tr.z;
    brDrawn[2] = br.z;

    // STEP 6: find centroids for the two sets of points (cube = A, drawn = B)
    double centA[3], centB[3];
    for (int i = 0; i < 3; i++) {
        double aVal = (blCube[i] + tlCube[i] + trCube[i] + brCube[i]) / 4;
        double bVal = (blDrawn[i] + tlDrawn[i] + trDrawn[i] + brDrawn[i]) / 4;

        centA[i] = aVal;
        centB[i] = bVal;
    }

    // STEP 7: recenter each point in both sets so centroids are at the origin
    blCube[0] -= centA[0];
    tlCube[0] -= centA[0];
    trCube[0] -= centA[0];
    brCube[0] -= centA[0];

    blCube[1] -= centA[1];
    tlCube[1] -= centA[1];
    trCube[1] -= centA[1];
    brCube[1] -= centA[1];

    blCube[2] -= centA[2];
    tlCube[2] -= centA[2];
    trCube[2] -= centA[2];
    brCube[2] -= centA[2];

    blDrawn[0] -= centB[0];
    tlDrawn[0] -= centB[0];
    trDrawn[0] -= centB[0];
    brDrawn[0] -= centB[0];

    blDrawn[1] -= centB[1];
    tlDrawn[1] -= centB[1];
    trDrawn[1] -= centB[1];
    brDrawn[1] -= centB[1];

    blDrawn[2] -= centB[2];
    tlDrawn[2] -= centB[2];
    trDrawn[2] -= centB[2];
    brDrawn[2] -= centB[2];

    // STEP 8: Create Mat objects using these values
    cv::Mat a = (cv::Mat_<double>(3,4) << blCube[0], tlCube[0], trCube[0], brCube[0], blCube[1], tlCube[1], trCube[1], brCube[1], blCube[2], tlCube[2], trCube[2], brCube[2]);
    cv::Mat b = (cv::Mat_<double>(3,4) << blDrawn[0], tlDrawn[0], trDrawn[0], brDrawn[0], blDrawn[1], tlDrawn[1], trDrawn[1], brDrawn[1], blDrawn[2], tlDrawn[2], trDrawn[2], brDrawn[2]);

    // STEP 9: transpose B to calculate matrix H (H = A*Bt) for SVD
    b = b.t();
    cv::Mat h = a * b;

    // STEP 10: plug H into SVD to find [U,S,Vt], then calculate R (R = VUt)
    cv::Mat s,u,vt;
    cv::SVD::compute(h,s,u,vt);
    cv::Mat r = vt.t() * u.t();

    // STEP 11: now that we have the optimal rotation matrix, apply it to the cube
    // convert matrix of uchar data to double data
    cv::Mat rDouble;
    r.convertTo(rDouble, CV_64F);

    double rData[9];
    double *vals[rDouble.rows];
    int idx = 0;

    // loop through rows/columns of the double matrix
    for (int i = 0; i < rDouble.rows; ++i) {
        vals[i] = rDouble.ptr<double>(i);

        for (int j = 0; j < rDouble.cols; ++j) {
            // save to the rData array and increment idx
            rData[idx] = vals[i][j];
            idx++;
        }
    }

    // create 4x4 matrix to represent Transformation Matrix
    vtkSmartPointer<vtkMatrix4x4> matrix =
        vtkSmartPointer<vtkMatrix4x4>::New();

    // reset idx for iterating through array
    idx = 0;

    // use r matrix to fill out 4x4 matrix (NOTE: use negative values for the z column to
    // avoid reflection issue)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // check if the last row is reached
            if (i == 3) {
                // set last line for homogeneous coords
                matrix->SetElement(i, 0, 0);
                matrix->SetElement(i, 1, 0);
                matrix->SetElement(i, 2, 0);
                matrix->SetElement(i, 3, 1);
                break;
            } else if (j == 2) {
                // use negative value to reflect against the XY plane (see note)
                matrix->SetElement(i, j, -rData[idx]);
                idx++;
            } else if (j == 3) { // check for last column to set translation vector
                // set element depending on which row we're on
                if (i == 0) {
                    matrix->SetElement(i, j, 0);
                } else if (i == 1) {
                    matrix->SetElement(i, j, 0);
                } else {
                    matrix->SetElement(i, j, 0);
                }
            } else {
                // otherwise, set the next element in the matrix and increment idx
                matrix->SetElement(i, j, rData[idx]);
                idx++;
            }
        }
    }

    // create transform object and apply it to the cube
    vtkSmartPointer<vtkTransform> transform =
        vtkSmartPointer<vtkTransform>::New();

    // get the center of the object for rotation
    double *cent = currentPose->objects[0]->cubeSource->GetCenter();

    // translate to/from the center, and concatenate matrix
    transform->Translate(cent[0], cent[1], cent[2]);
    transform->Concatenate(matrix);
    transform->Translate(-cent[0], -cent[1], -cent[2]);

    // set up transform filter
    vtkSmartPointer<vtkTransformFilter> filter =
        vtkSmartPointer<vtkTransformFilter>::New();

    // add cube and transform, and update
    filter->SetInputConnection(currentPose->objects[0]->cubeSource->GetOutputPort());
    filter->SetTransform(transform);
    filter->Update();

    // set up new mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(filter->GetOutputPort());

    // set mapper to existing actor
    currentPose->objects[0]->actor->SetMapper(mapper);

    // save transform filter in ObjectData object
    currentPose->objects[0]->filter = filter;

    // save snapping transformation matrix and change snap flag
    currentPose->objects[0]->snapMatrix = matrix;
    currentPose->objects[0]->snap = true;

    // remove drawn points from the window
    for (int i = 0; i < 4; i++) {
        // retrieve point from the vector of point data objects
        PointData *point = currentPose->points[i];

        // remove actor from the current renderer
        this->CurrentRenderer->RemoveActor(point->actor);

        // if last point is reached, remove all points from the vector
        if (i == 3) {
            // erase
            currentPose->points.erase(currentPose->points.begin(), currentPose->points.end());
        }
    }

    // update window
    this->CurrentRenderer->ResetCamera();
    this->Interactor->Render();
}

void ModelingWindowStyle::MoveObject() {
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
        std::cout << "Z Rotation "
                  << (attributes->RotateZ ? "Enabled!" : "Disabled!")
                  << std::endl;

        return;
    }

    // get object data to update rotation values
    ObjectData *data = GetObject(attributes->selectedActor);

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
        std::cout << "Z Stretch "
                  << (attributes->StretchZ ? "Enabled!" : "Disabled!")
                  << std::endl;

        return;
    }

    // get object data to update scale values
    ObjectData *data = GetObject(attributes->selectedActor);

    // check if z rotation has been enabled
    if (attributes->StretchZ) {
        // check for left/right arrow keys only
        if (key == "Left") {
            // make sure scale value doesn't fall to 0.3 or below
            if (data->scaleZ - .3 <= 0) {
                // prompt user and return
                std::cout << "Error Stretching!" << std::endl
                          << "Scale value would fall to zero or below."
                          << std::endl;
                return;
            } else {
                // stretch z axis (-)
                data->scaleZ -= .1;
            }
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
            // make sure scale value doesn't fall to 0.3 or below
            if (data->scaleX - .3 <= 0) {
                // prompt user and return
                std::cout << "Error Stretching!" << std::endl
                          << "Scale value would fall to zero or below."
                          << std::endl;
                return;
            } else {
                // stretch x axis (-)
                data->scaleX -= .1;
            }
        } else if (key == "Right") {
            // stretch x axis (+)
            data->scaleX += .1;
        } else if (key == "Up") {
            // stretch y axis (+)
            data->scaleY += .1;
        } else if (key == "Down") {
            // make sure scale value doesn't fall to 0.3 or below
            if (data->scaleY - .3 <= 0) {
                // prompt user and return
                std::cout << "Error Stretching!" << std::endl
                          << "Scale value would fall to zero or below."
                          << std::endl;
                return;
            } else {
                // stretch y axis (-)
                data->scaleY -= .1;
            }
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

    // get object data to update scale values
    ObjectData *data = GetObject(attributes->selectedActor);

    // scale object equally for each axis for up/down arrows
    if (key == "Up") {
        // scale +5%
        data->scaleX += .05;
        data->scaleY += .05;
        data->scaleZ += .05;
    } else if (key == "Down") {
        // make sure scale values don't fall to 0.3 or below
        if (data->scaleX - .1 <= .3 || data->scaleY - .1 <= .3 ||
                data->scaleZ - .3 <= 0) {
            // prompt user and return
            std::cout << "Error Scaling!" << std::endl
                      << "Scale value(s) would fall to zero or below."
                      << std::endl;
            return;
        } else {
            // scale -5%
            data->scaleX -= .05;
            data->scaleY -= .05;
            data->scaleZ -= .05;
        }
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

    // create ObjectData object to store cube source and actor
    ObjectData *object = new ObjectData();
    object->actor = actor;
    object->cubeSource = cubeSource;

    // add to vector of objects
    currentPose->objects.push_back(object);
}

void ModelingWindowStyle::DrawCubeOntoImage(int *center, int sideLength) {
    // get center position in world coordinates
    double *centerWc = GetClickPosition(center[0], center[1]);

    // set z coordinate to 0
    centerWc[2] = 0.0;

    // Create a cube
    vtkSmartPointer<vtkCubeSource> cubeSource =
      vtkSmartPointer<vtkCubeSource>::New();

    // set center in world coordinates
    cubeSource->SetCenter(centerWc);

    // set length/width/height from parameters
    cubeSource->SetXLength(sideLength);
    cubeSource->SetYLength(sideLength);
    cubeSource->SetZLength(sideLength);

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

    // create ObjectData object to store cube source and actor
    ObjectData *object = new ObjectData();
    object->actor = actor;
    object->cubeSource = cubeSource;

    // add to vector of objects
    currentPose->objects.push_back(object);
}

void ModelingWindowStyle::DrawPointOntoImage() {
    // create PointData object
    PointData *data = new PointData();

    // get click position
    double *position = GetClickPosition();

    // change z coordinate to 0
    position[2] = 0;

    // initialize points and insert point
    data->points = vtkSmartPointer<vtkPoints>::New();

    vtkIdType pid[1];
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

    // add point data to the pose
    currentPose->points.push_back(data);
}

void ModelingWindowStyle::DrawSphereOntoImage() {
    // get click position (x,y)
    double *position = GetClickPosition();

    // Create a sphere
    vtkSmartPointer<vtkSphereSource> sphereSource =
      vtkSmartPointer<vtkSphereSource>::New();

    // set center based on click (z = 0)
    sphereSource->SetCenter(position[0], position[1], 0);

    // set radius (match size of cube)
    sphereSource->SetRadius(12.5);

    // set phi and theta resolutions higher to smooth sphere
    sphereSource->SetPhiResolution(20);
    sphereSource->SetThetaResolution(20);

    // map cube source to data set mapper
    vtkSmartPointer<vtkDataSetMapper> mapper =
      vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    // create actor and set mapper
    vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // add actor to the current renderer
    this->CurrentRenderer->AddActor(actor);

    // create ObjectData object to store sphere source and actor
    ObjectData *object = new ObjectData();
    object->actor = actor;
    object->sphereSource = sphereSource;

    // add to vector of objects
    currentPose->objects.push_back(object);
}

void ModelingWindowStyle::PerformTransformations(ObjectData *data) {
    // get output port of the object from GetOutputPort
    vtkAlgorithmOutput *outputPort = GetOutputPort(data);

    // generate transform object from GetTransform
    vtkSmartPointer<vtkTransform> transform = GetTransform(data);

    // set up transform filter
    vtkSmartPointer<vtkTransformFilter> filter =
      vtkSmartPointer<vtkTransformFilter>::New();

    // add cube and transform, and update
    filter->SetInputConnection(outputPort);
    filter->SetTransform(transform);
    filter->Update();

    // set up new mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(filter->GetOutputPort());

    // set mapper to existing actor
    this->attributes->selectedActor->SetMapper(mapper);

    // save transform filter in ObjectData object
    data->filter = filter;

    // reset camera and update window
    this->Interactor->Render();
}

vtkSmartPointer<vtkTransform> ModelingWindowStyle::GetTransform(ObjectData *data) {
    // create transform object
    vtkSmartPointer<vtkTransform> transform =
        vtkSmartPointer<vtkTransform>::New();

    // grab the center of the object by checking the pointers
    // to the cube/sphere source objects
    double *center;

    if (data->cubeSource) {
        // grab values from cube source
        center = data->cubeSource->GetCenter();
    } else if (data->sphereSource) {
        // grab values from sphere source
        center = data->sphereSource->GetCenter();
    } else {
        // prompt user of error and return
        std::cout << "Unknown Error!\nPlease try again." << std::endl;
        return NULL;
    }

    // apply initial translation to center the cube at the origin
    transform->Translate(center[0], center[1], center[2]);

    // apply rotations
    transform->RotateX(data->rotationX);
    transform->RotateY(data->rotationY);
    transform->RotateZ(data->rotationZ);

    // scale with appropriate values
    transform->Scale(data->scaleX, data->scaleY, data->scaleZ);

    // check flag to see if snapping has been applied
    if (data->snap) {
        // if so, concat matrix before translating back to center
        transform->Concatenate(data->snapMatrix);
    }

    // apply final translation
    transform->Translate(-center[0], -center[1], -center[2]);

    // set to post multiply to append matrix for potential pose change
    transform->PostMultiply();

    // check pose number for initial pose
    if (currentPose->poseNum != 0) {
        // if not initial pose, concat transformation matrix
        transform->Concatenate(currentPose->matrix);
    } // otherwise, no need

    // done, so return transform
    return transform;
}

vtkAlgorithmOutput *ModelingWindowStyle::GetOutputPort(ObjectData *data) {
    // check pointers in the ObjectData object to get the appropriate output port
    vtkAlgorithmOutput *outputPort;

    if (data->cubeSource) {
        // return value from cube source
        return data->cubeSource->GetOutputPort();
    } else if (data->sphereSource) {
        // return value from sphere source
        return data->sphereSource->GetOutputPort();
    } else {
        // prompt user of error and return
        std::cout << "Unknown Error!\nPlease try again." << std::endl;
        return NULL;
    }
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

double *ModelingWindowStyle::GetClickPosition(int x, int y) {
    // use picker to select correct pixel location
    this->Interactor->GetPicker()->Pick(x,
                                        y,
                                        0, // always zero
                                        this->CurrentRenderer);

    // get the pick position
    double *position = this->Interactor->GetPicker()->GetPickPosition();

    return position;
}

ObjectData *ModelingWindowStyle::GetObject(vtkSmartPointer<vtkActor> actor) {
    // traverse through cubes and compare actors
    for (unsigned int i = 0; i < currentPose->objects.size(); i++) {
        // if actors match, return the CubeData reference
        if (currentPose->objects[i]->actor == actor) {
            return currentPose->objects[i];
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

    // check if pose is already displayed in either pose renderer
    if (rightPoseIdx == poseNum || poseNum == 0) {
        // prompt user and return
        std::cout << "Pose Already Displayed!" << std::endl;
        return;
    }

    // check if pose has already been fetched
    for (unsigned int i = 0; i < poses.size(); i++) {
        // check each pose number in poses
        if (poses[i]->poseNum == poseNum) {
            // prompt user that pose already fetched
            std::cout << "Pose Already Requested!" << std::endl;

            // update right pose image
            UpdateRightPoseImage(poseNum);

            // reset camera and re-render before adding objects
            attributes->rendererMap[10]->ResetCamera();
            this->Interactor->Render();

            // update entities in that renderer
            UpdateRightPoseEntities(poses[i]);

            // set pose idx to i
            poseIdx = i;
        }
    }

    // otherwise, check if pose idx is invalid
    if (rightPoseIdx != -1) {
        // if so, update image in the right pose renderer
        UpdateRightPoseImage(poseNum);

        // create new pose using pose #
        CreateNewPose(poseNum);
    } else {
        // create image actor to display new pose in right image renderer
        vtkSmartPointer<vtkImageActor> imageActor =
            vtkSmartPointer<vtkImageActor>::New();
        imageActor->GetMapper()->SetInputConnection(attributes->readers[poseNum]->GetOutputPort());

        // set image actor's z coordinate (z = -100)
        imageActor->SetPosition(imageActor->GetPosition()[0],
                                imageActor->GetPosition()[1],
                                -100);

        // add image actor to the right image renderer (idx = 10)
        attributes->rendererMap[10]->AddActor(imageActor);

        // save reference in style
        rightPoseImage = imageActor;

        // reset camera and re-render
        attributes->rendererMap[10]->ResetCamera();
        this->Interactor->Render();

        // create new pose
        CreateNewPose(poseNum);
    }

    // update right pose idx
    rightPoseIdx = poseNum;

    // reset camera of that renderer
    attributes->rendererMap[10]->ResetCamera();

    // re-render window
    this->Interactor->Render();
}

void ModelingWindowStyle::Output() {
    // prompt user to select their desired file format
    std::cout << "Output Selected! \nEnter 0 for .ply, 1 for .xyz, or 2 for .stl: ";

    // get selection from input
    int outputFormat;
    std::cin >> outputFormat;

    // check if selection is valid
    if (outputFormat < 0 || outputFormat > 2) {
        // prompt user if invalid and return
        std::cout << "Invalid Selection! Please try again." << std::endl;

        return;
    }

    // prompt user to output one combined model, or separate models
    std::cout << "Please enter 0 for a combined model, or 1 for separate models: ";

    // get selection from input
    int combined;
    std::cin >> combined;

    // check validity of the selection
    if (combined < 0 || combined > 1) {
        // prompt user if invalid and return
        std::cout << "Invalid Selection! Please try again." << std::endl;

        return;
    }

    // initialize writer based on the output format chosen
    vtkSmartPointer<vtkPLYWriter> plyWriter;
    vtkSmartPointer<vtkSimplePointsWriter> xyzWriter;
    vtkSmartPointer<vtkSTLWriter> stlWriter;

    if (outputFormat == 0) {
        // print format selected to console
        std::cout << ".PLY Selected!" << std::endl;

        // initialize PLY writer
        plyWriter = vtkSmartPointer<vtkPLYWriter>::New();
    } else if (outputFormat == 1) {
        // print format selected to console
        std::cout << ".XYZ Selected!" << std::endl;

        // initialize simple points (XYZ) writer
        xyzWriter = vtkSmartPointer<vtkSimplePointsWriter>::New();
    } else {
        // print format selected to console
        std::cout << ".STL Selected!" << std::endl;

        // initialize STL writer
        stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
    }

    // combine objects into one cohesive model
    if (combined == 0) {
        // initialize append poly data object to attach the objects
        vtkSmartPointer<vtkAppendPolyData> appendFilter =
            vtkSmartPointer<vtkAppendPolyData>::New();

        // loop through each object in the final pose
        PoseData *lastPose = poses[poses.size() - 1];

        for (int i = 0; i < lastPose->objects.size(); i++) {
            // grab the poly data from the filter
            vtkPolyData *data = lastPose->objects[i]->filter->GetPolyDataOutput();

            // append to the filter
            appendFilter->AddInputData(data);
        }

        // initialize clean poly data object to remove duplicate points
        vtkSmartPointer<vtkCleanPolyData> cleanFilter =
            vtkSmartPointer<vtkCleanPolyData>::New();
        cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
        cleanFilter->Update();

        // set the filename, output, and write to file depending on the specified format
        if (plyWriter) {
            plyWriter->SetFileName("combined.ply");
            plyWriter->SetInputConnection(cleanFilter->GetOutputPort());
            plyWriter->Write();
        } else if (xyzWriter) {
            xyzWriter->SetFileName("combined.xyz");
            xyzWriter->SetInputConnection(cleanFilter->GetOutputPort());
            xyzWriter->Write();
        } else if (stlWriter) {
            stlWriter->SetFileName("combined.stl");
            stlWriter->SetInputConnection(cleanFilter->GetOutputPort());
            stlWriter->Write();
        }
    } else {
        // otherwise, output each object into its own file

        // get the final requested pose
        PoseData *lastPose = poses[poses.size() - 1];

        // loop through each object in the pose to output
        for (int i = 0; i < lastPose->objects.size(); i++) {
            // set the filename, output, and write to file depending on the specified format
            if (plyWriter) {
                // create filename string
                std::stringstream filename;
                filename << i << ".ply";

                // set filename, input, and write file
                plyWriter->SetFileName(filename.str().c_str());
                plyWriter->SetInputConnection(lastPose->objects[i]->filter->GetOutputPort());
                plyWriter->Write();
            } else if (xyzWriter) {
                // create filename string
                std::stringstream filename;
                filename << i << ".xyz";

                // set filename, input, and write file
                xyzWriter->SetFileName(filename.str().c_str());
                xyzWriter->SetInputConnection(lastPose->objects[i]->filter->GetOutputPort());
                xyzWriter->Write();
            } else if (stlWriter) {
                // create filename string
                std::stringstream filename;
                filename << i << ".stl";

                // set filename, input, and write file
                stlWriter->SetFileName(filename.str().c_str());
                stlWriter->SetInputConnection(lastPose->objects[i]->filter->GetOutputPort());
                stlWriter->Write();
            }
        }
    }
}

void ModelingWindowStyle::UpdateRightPoseImage(int newPose) {
    // check if pose idx is valid
    if (newPose < 0 || newPose >= attributes->readers.size()) {
        std::cout << "Invalid Pose Number!" << std::endl;
    }

    // otherwise, update right pose image
    rightPoseImage->GetMapper()->SetInputConnection(attributes->readers[newPose]->GetOutputPort());
}

void ModelingWindowStyle::UpdateRightPoseEntities(PoseData *pose) {
    // find PoseData object of right pose
    PoseData *oldPose;

    // loop through poses vector and compare poses #'s
    for (unsigned int i = 0; i < poses.size(); i++) {
        // if rightPoseIdx matches pose #, break
        if (poses[i]->poseNum == rightPoseIdx) {
            oldPose = poses[i];
            break;
        }
    }

    // check if pose was found
    if (oldPose) {
        // if so, loop through each object in this pose
        for (unsigned int i = 0; i < oldPose->objects.size(); i++) {
            // remove actor from the right pose renderer (idx = 10)
            attributes->rendererMap[10]->RemoveActor(oldPose->objects[i]->actor);
        }

        // now, loop through each cube in the new pose
        for (unsigned int i = 0; i < pose->objects.size(); i++) {
            // add actor to the same renderer
            attributes->rendererMap[10]->AddActor(pose->objects[i]->actor);
        }

    } // otherwise, do nothing
}

void ModelingWindowStyle::CreateNewPose(int newPose) {
    // create new PoseData object
    PoseData *pose = new PoseData();

    // set pose number to newPose
    pose->poseNum = newPose;

    // get filename of new pose
    std::stringstream newPoseFile;
    newPoseFile << newPose << ".txt";

    // set current matrix of new pose
    pose->matrix = GetMatrix(newPoseFile.str().c_str());

    // check if multiple poses have been requested
    if (poses.size() > 1) {
        // find PoseData object of right pose
        PoseData *oldPose;

        for (unsigned int i = 0; i < poses.size(); i++) {
            // if rightPoseIdx matches pose #, break to remove old actors
            if (poses[i]->poseNum == rightPoseIdx) {
                oldPose = poses[i];
                break;
            }
        }

        // check if pose was found
        if (oldPose) {
            // if so, loop through each object in this pose
            for (unsigned int i = 0; i < oldPose->objects.size(); i++) {
                // remove actor from the right pose renderer (idx = 10)
                attributes->rendererMap[10]->RemoveActor(oldPose->objects[i]->actor);
            }
        } else return; // otherwise, do nothing
    }


    // transform entities to reflect new pose
    TransformEntities(pose);

    // add pose to vector of PoseData objects
    poses.push_back(pose);

    // update pose index to last element of the vector
    poseIdx = poses.size() - 1;

    // deselect selectedActor if it exists
    if (attributes->selectedActor != NULL) {
        DeselectActor();
    }
}

void ModelingWindowStyle::TransformEntities(PoseData *pose) {
    PoseData *lastPose = poses[poses.size() - 1];

    // loop through each cube drawn in the previous RF
    for (unsigned int i = 0; i < lastPose->objects.size(); i++) {
        // create new ObjectData object
        ObjectData *data = new ObjectData();

        // copy cube/sphere source and transformation values (except
        // translation values) to new ObjectData object
        data->cubeSource = lastPose->objects[i]->cubeSource;
        data->sphereSource = lastPose->objects[i]->sphereSource;
        data->rotationX = lastPose->objects[i]->rotationX;
        data->rotationY = lastPose->objects[i]->rotationY;
        data->rotationZ = lastPose->objects[i]->rotationZ;
        data->scaleX = lastPose->objects[i]->scaleX;
        data->scaleY = lastPose->objects[i]->scaleY;
        data->scaleZ = lastPose->objects[i]->scaleZ;

        // if object has been snapped, copy the snap matrix and change flag
        if (lastPose->objects[i]->snap) {
            data->snap = true;
            data->snapMatrix = lastPose->objects[i]->snapMatrix;
        } else {
            // otherwise, set flag to false
            data->snap = false;
        }

        // initialize actor to be placed in right pose renderer
        data->actor = vtkSmartPointer<vtkActor>::New();

        // create transform object
        vtkSmartPointer<vtkTransform> transform =
            vtkSmartPointer<vtkTransform>::New();

        // grab the center/output port of the object by checking the
        // pointers to the cube/sphere source objects
        double *center;
        vtkAlgorithmOutput *outputPort;

        if (lastPose->objects[i]->cubeSource) {
            // grab values from cube source
            center = lastPose->objects[i]->cubeSource->GetCenter();
            outputPort = lastPose->objects[i]->cubeSource->GetOutputPort();
        } else if (lastPose->objects[i]->sphereSource) {
            // grab values from sphere source
            center = lastPose->objects[i]->sphereSource->GetCenter();
            outputPort = lastPose->objects[i]->sphereSource->GetOutputPort();
        } else {
            // prompt user of error and return
            std::cout << "Unknown Error!\nPlease try again." << std::endl;
            return;
        }

        // apply initial translation to center the cube at the origin
        transform->Translate(center[0], center[1], center[2]);

        // apply rotations
        transform->RotateX(lastPose->objects[i]->rotationX);
        transform->RotateY(lastPose->objects[i]->rotationY);
        transform->RotateZ(lastPose->objects[i]->rotationZ);

        // scale with appropriate values
        transform->Scale(lastPose->objects[i]->scaleX, lastPose->objects[i]->scaleY, lastPose->objects[i]->scaleZ);

        // check if object has been snapped
        if (data->snap) {
            // if so, concat the snap matrix
            transform->Concatenate(data->snapMatrix);
        }

        // apply final translation
        transform->Translate(-center[0], -center[1], -center[2]);

        // apply transformation matrix
        transform->PostMultiply();
        transform->Concatenate(pose->matrix);

        // set up transform filter
        vtkSmartPointer<vtkTransformFilter> filter =
          vtkSmartPointer<vtkTransformFilter>::New();

        // add cube and transform, and update
        filter->SetInputConnection(outputPort);
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

        // set filter of the object
        data->filter = filter;

        // add new ObjectData to new PoseData
        pose->objects.push_back(data);
    }
}

void ModelingWindowStyle::ChangePose(int direction) {
    // check if only two poses (or less) exist
    if (poses.size() <= 2) {
        // prompt user and return
        std::cout << "No New Poses to Display! Request a New Pose to Display a New One!"
                  << std::endl;
        return;
    }
    // update pose idx based on bounds of the vector of poses
    if ((poseIdx + direction) > (poses.size() - 1)) {
        // idx becomes the second element of vector of poses (i.e. don't
        // want to show initial pose twice)
        poseIdx = 1;
    } else if ((poseIdx + direction) <= 0) {
        // idx becomes the last element of the vector
        poseIdx = poses.size() - 1;
    } else {
        // otherwise, just add direction
        poseIdx += direction;
    }

    // update image
    UpdateRightPoseImage(poses[poseIdx]->poseNum);

    // update entities
    UpdateRightPoseEntities(poses[poseIdx]);

    // update right pose idx
    rightPoseIdx = poses[poseIdx]->poseNum;

    // deselect selectedActor if it exists
    if (attributes->selectedActor != NULL) {
        DeselectActor();
    }

    // reset camera of the right pose renderer
    attributes->rendererMap[10]->ResetCamera();

    // re-render window
    this->Interactor->Render();
}

void ModelingWindowStyle::SelectActor(vtkSmartPointer<vtkActor> actor) {
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

void ModelingWindowStyle::DeselectActor() {
    // change selectedActor color to white
    attributes->selectedActor->GetProperty()->SetColor(1,1,1);

    // change Selected flag
    attributes->Selected = false;

    // re-render the window
    this->Interactor->Render();
}

vtkSmartPointer<vtkMatrix4x4> ModelingWindowStyle::GetMatrix(std::string fileName) {
    // vars to read text files
    std::string line;
    ifstream file(fileName.c_str());

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
            // check if the last row is reached
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

void ModelingWindowStyle::CameraZoom(double factor) {
    // get the (pose) renderer to zoom based on attributes value
    vtkSmartPointer<vtkRenderer> renderer;

    if (attributes->poseToZoom == 0) renderer = attributes->rendererMap[0];
    else renderer = attributes->rendererMap[10];

    // get active camera of the renderer
    vtkCamera *camera = renderer->GetActiveCamera();

    // zoom based on the factor passed in (add 1 to start at 100%)
    camera->Zoom(factor + 1);

    // re-render window to show change
    this->Interactor->Render();
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
    } else if (this->CurrentRenderer == attributes->rendererMap[6]) {
        // request selected
        RequestSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[7]) {
        // output selected
        OutputSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[8]) {
        // left arrow selected
        LeftArrowSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[9]) {
        // right arrow selected
        RightArrowSelected();
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
        SphereSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[14]) {
        // zoom selected
        ZoomSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[15]) {
        // zoom in selected
        ZoomInSelected();
    } else if (this->CurrentRenderer == attributes->rendererMap[16]) {
        // zoom out selected
        ZoomOutSelected();
    }

    // otherwise, perform no action
}

void ModelingWindowStyle::LeftSceneSelected() {
    // check if user is drawing a bounding box
    if (attributes->DrawBox) {
        // if so, set drawing flag to true
        attributes->DrawingBox = true;

        // set start and end positions
        attributes->StartPosition[0] = this->Interactor->GetEventPosition()[0];
        attributes->StartPosition[1] = this->Interactor->GetEventPosition()[1];
        attributes->EndPosition[0] = attributes->StartPosition[0];
        attributes->EndPosition[1] = attributes->StartPosition[1];

        // initialize pixel array
        attributes->PixelArray->Initialize();
        attributes->PixelArray->SetNumberOfComponents(4);
        int *size = this->Interactor->GetRenderWindow()->GetSize();
        attributes->PixelArray->SetNumberOfTuples(size[0] * size[1]);

        // set data from the render window
        this->Interactor->GetRenderWindow()->GetRGBACharPixelData(0,0,size[0]-1, size[1]-1,1,attributes->PixelArray);

        return;
    }

    // check if snapping is occurring
    if (attributes->Snap) {
        // draw point for the user
        DrawPointOntoImage();

        // update window
        this->CurrentRenderer->ResetCamera();
        this->Interactor->Render();

        // check how many points have been drawn
        if (attributes->PointsDrawn == 0) {
            // none --> top left corner next
            std::cout << "\nBottom Left Corner point drawn."
                      << "\n\nClick on the top left corner." << std::endl;
        } else if (attributes->PointsDrawn == 1) {
            // 1 --> top right corner next
            std::cout << "\nTop Left Corner point drawn."
                      << "\n\nClick on the top right corner." << std::endl;
        } else if (attributes->PointsDrawn == 2) {
            // 2 --> bottom right corner next
            std::cout << "\nTop Right Corner point drawn."
                      << "\n\nClick on the bottom right corner." << std::endl;
        } else {
            // we have all the points required, so draw cube and snap!
            std::cout << "\nBottom Right Corner point drawn."
                      << "\n\n------Snapping------" << std::endl;

            // call Snap function
            Snap();

            // change flag back to false and reset points drawn
            attributes->Snap = false;
            attributes->PointsDrawn = 0;

            // prompt user that snapping has been performed
            std::cout << "\nSnapping Occurred! If you don't like the results, use the manipulation tools"
                      << " to tweak the model to your preference" << std::endl;
        }

        // increment points drawn
        attributes->PointsDrawn++;

        return;
    }

    // check if draw is selected
    if (attributes->Draw) {
        // check if cube is selected, if so draw a cube
        if (attributes->Cube) {
            // if so, draw cube
            DrawCubeOntoImage();

            // reset camera
            this->CurrentRenderer->ResetCamera();

            // re-render
            this->Interactor->Render();

            // done, so return
            return;

        } // check if point is selected
        else if (attributes->Point) {
            // draw point
            DrawPointOntoImage();

            // reset camera
            this->CurrentRenderer->ResetCamera();

            // re-render
            this->Interactor->Render();

            // done, so return
            return;
        } // check if sphere is selected
        else if (attributes->Sphere) {
            // draw sphere
            DrawSphereOntoImage();

            // reset camera
            this->CurrentRenderer->ResetCamera();

            // re-render
            this->Interactor->Render();

            // done, so return
            return;
        }
    }

    // check if object was selected
    if (attributes->Selected) {
        // if so, check if move tool was selected
        if (attributes->Move) {
            // if so, move object
            MoveObject();
        } else {
            // otherwise, deselect the actor
            DeselectActor();
        }
    }

    // check if user tried to select an object
    vtkSmartPointer<vtkActor> actor = GetActorUnderClick();

    // check if actor was selected
    if (actor != NULL) {
        // if actor is not null, user selected that actor
        SelectActor(actor);
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

        // change bounding box flag to false
        attributes->DrawBox = false;
    } else {
        // change to true
        attributes->Cube = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);

        // prompt user to snap!
        PromptSnapping();
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

        // change attributes value to default
        attributes->poseToZoom = -1;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change renderer to be green
        ChangeRenderer(0,1,0);

        // prompt user to select a pose to zoom
        std::cout << "Zoom Selected!" << std::endl
                  << "Enter 0 for left pose, or 1 for right pose: ";

        // grab value
        int selectedPose;
        std::cin >> selectedPose;

        // make sure pose entered was valid
        if (selectedPose != 0 && selectedPose != 1) {
            // if invalid, prompt user
            std::cout << "Invalid pose entered! Please try again." << std::endl;

            // change renderer back to gray
            ChangeRenderer(.86,.86,.86);
        } else {
            // save value in window attributes
            attributes->poseToZoom = selectedPose;

            // change flag to true
            attributes->Zoom = true;
        }
    }
}

void ModelingWindowStyle::ZoomInSelected() {
    // if zoom is not preselected, do nothing
    if (!attributes->Zoom) return;

    // otherwise, zoom in by 10%
    CameraZoom(.1);
}

void ModelingWindowStyle::ZoomOutSelected() {
    // if zoom is not preselected, do nothing
    if (!attributes->Zoom) return;

    // otherwise, zoom out by 10%
    CameraZoom(-.1);
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

void ModelingWindowStyle::SphereSelected() {
    // change flag depending on selected or not
    if (attributes->Sphere) {
        // change back to false
        attributes->Sphere = false;

        // change renderer to be gray
        ChangeRenderer(.86,.86,.86);
    } else {
        // change to true
        attributes->Sphere = true;

        // change renderer to be green
        ChangeRenderer(0,1,0);
    }
}

void ModelingWindowStyle::RequestSelected() {
    // change renderer to be green
    ChangeRenderer(0,1,0);

    // request a new pose
    RequestNewPose();

    // change renderer back to gray
    ChangeRenderer(.86,.86,.86);
}

void ModelingWindowStyle::OutputSelected() {
    // change renderer to be green
    ChangeRenderer(0,1,0);

    // call output
    Output();

    // close the window
    Interactor->TerminateApp();
}

void ModelingWindowStyle::LeftArrowSelected() {
    // change pose in the negative direction
    ChangePose(-1);
}

void ModelingWindowStyle::RightArrowSelected() {
    // change pose in the positive direction
    ChangePose(1);
}

void ModelingWindowStyle::SetRendererMap(std::map<int, vtkRenderer *> map) { attributes->rendererMap = map; }

void ModelingWindowStyle::SetReaders(std::vector<vtkSmartPointer<vtkPNGReader> > pngReaders) { attributes->readers = pngReaders; }
