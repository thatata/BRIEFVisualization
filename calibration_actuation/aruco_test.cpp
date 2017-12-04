  #include "opencv2/core.hpp"
  #include "opencv2/imgcodecs.hpp"
  #include "opencv2/imgproc.hpp"
  #include "opencv2/highgui.hpp"
  #include "opencv2/aruco.hpp"
  #include "opencv2/calib3d.hpp"

  #include <sstream>
  #include <iostream>
  #include <fstream>

  #include <vector>

  using namespace std;
  using namespace cv;
  using std::vector;

  // Constants for Calibration
  const float calibrationSquareDemension = .0239f; //meters
  const float arucoSquareDimension = .080f;   //meters - 80mm = 8cm
  const Size chessboardDimensions = Size(6, 9); //Size of calibration board (given)

  // Known location of markers in Test Environment
  std::vector<Vec3d> translationVectorsToOrigin;
  Vec3d mk0 (0.040f, 0.304f, 0);
  Vec3d mk1 (0.134f, 0.040f, 0);
  Vec3d mk2 (0.148f, 0.204f, 0);

  // Global variable to determine where the Kinect actually is
  Vec3d kinectActual (0, 0, 0);

  void createArucoMarkers() {
    Mat outputMarker;

    //Ptr<aruco::Dictionary> markerDictionary =
    //  aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4x4_50);

    Ptr<aruco::Dictionary> markerDictionary =
        aruco::getPredefinedDictionary( 0 );



    for ( int i = 0 ; i < 50 ; i++ ) {
      aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
      ostringstream convert;
      string imageName = "4x4Marker_";
      convert << imageName << i << ".jpg";
      imwrite(convert.str(), outputMarker);
    }
  }

  void createKnowBoardPositions(Size boardSize, float squareEdgeLength, vector<Point3f> corners ) {
    for ( int i = 0 ; i < boardSize.height; i++ ) {
      for ( int j = 0 ; j < boardSize.width; j++ ) {
        // Push in all calculated of expected Point3f
        corners.push_back( Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f) );
      }
    }
  }

  void getChessboardCorners( vector<Mat> images, vector< vector<Point2f> >& allFoundCorners, bool showResults = false ) {
    for ( vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++ ) {
      vector<Point2f> pointBuf;
      // Using opencv function
      bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE );

      if ( found ) {
        allFoundCorners.push_back(pointBuf);
      }

      if ( showResults ) {
        drawChessboardCorners(*iter, Size(9,6), pointBuf, found);
        imshow("Looking for Corners", *iter);
        waitKey(0); //Does not terminate until done
      }
    }
  }

  void cameraCalibration( vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients ) {
    printf("Enters function successfully\n");
    vector< vector<Point2f> > checkerboardImageSpacePoints;
    getChessboardCorners( calibrationImages, checkerboardImageSpacePoints, false );
    printf("successfully gets chessboardCorners\n");
    // Create known board positions
    vector< vector<Point3f> > worldSpaceCornerPoints(1);

    createKnowBoardPositions(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<Mat> rVectors, tVectors;
    distanceCoefficients = Mat::zeros(8, 1, CV_64F);

    printf("I successfully getChessboardCorners, createKnowBoardPositions, and set distanceCoefficients\n");

    // TODO ISSUE! passing in correct points but not recognizing in opencv function
    cv::calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
  }

  // TODO: Test if it is working. Cannot without camera calibration
  bool saveCameraCalibration( string name, Mat cameraMatrix, Mat distanceCoefficients ) {
    printf("ENTERS CAMERA CALIBRATION SUCCESSFULLY\n");

    ofstream outStream;
    outStream.open(name.c_str());

    if ( outStream ) {
      uint16_t rows = cameraMatrix.rows;
      uint16_t columns = cameraMatrix.cols;

      // Push out rows and cols to file << endl = \n
      outStream << rows << endl;
      outStream << columns << endl;

      for ( int r = 0 ; r < rows ; r++  ) {
        for ( int c = 0 ; c < columns ; c++ ) {
          double value = cameraMatrix.at<double>(r, c);
          outStream << value << endl;
        }
      }

      rows = distanceCoefficients.rows;
      columns = distanceCoefficients.cols;

      // Push out rows and cols to file << endl = \n for distanceCoefficients
      outStream << rows << endl;
      outStream << columns << endl;

      for ( int r = 0 ; r < rows ; r++  ) {
        for ( int c = 0 ; c < columns ; c++ ) {
          double value = distanceCoefficients.at<double>(r, c);
          outStream << value << endl;
        }
      }

      outStream.close();
      return true;
    }
    return false;
  }

  // Fully working. Takes in precalculated calibration values of the camera for analysis
  bool loadCameraCalibration( string name, Mat& cameraMatrix, Mat& distanceCoefficients ) {

    // Bring in the file that we are loading information from
    ifstream inStream;
    inStream.open(name.c_str());

    if ( inStream ) {
      uint16_t rows;
      uint16_t columns;

      inStream >> rows;
      inStream >> columns;

      cameraMatrix = Mat( Size(columns, rows), CV_64F );

      for ( int r = 0 ; r < rows ; r++ ) {
        for ( int c = 0 ; c < columns ; c++ ) {
          double temp = 0.0f;
          inStream >> temp;
          cameraMatrix.at<double>(r,c) = temp;
          cout << cameraMatrix.at<double>(r,c) << "\n";
        }
      }

      // Find distanceCoefficients
      inStream >> rows;
      inStream >> columns;

      distanceCoefficients = Mat::zeros(rows, columns, CV_64F);

      for ( int r = 0 ; r < rows ; r++ ) {
        for ( int c = 0 ; c < columns ; c++ ) {
          double temp = 0.0f;
          inStream >> temp;
          distanceCoefficients.at<double>(r,c) = temp;
          cout << distanceCoefficients.at<double>(r,c) << "\n";
        }
      }

      inStream.close();
      return true;
    }

    return false;
  }

  // Function is called by startWebcamMonitoring
  // TODO: Remove cameraMatrix and distanceCoefficients from input. Not needed
  int analyzeFrame( vector< vector<Point2f> > markerCorners, const Mat& cameraMatrix, const Mat& distanceCoefficients, vector<Vec3d> rotationVectors, vector<Vec3d> translationVectors, vector<int> markerIds ) {

    // Start by printing markerCorners and determing what the data looks like
    printf("MarkerId:\t");
    printf("(  A  )\t\t");
    printf("(  B  )\t\t");
    printf("(  C  )\t\t");
    printf("(  D  )\n");

    for ( int r = 0 ; r < markerCorners.size() ; r++ ) {
      printf("MarkerId: %d\t", markerIds[r]);
      for ( int c = 0 ; c < 4 ; c++ ) {
        printf("(%f, %f)\t", markerCorners[r][c].x, markerCorners[r][c].y );
      }
      printf("\n");
    }
    printf("\n");

    // Print out rotationVectors and translationVectors
    for ( int r = 0 ; r < rotationVectors.size() ; r++ ) {
      printf("MarkerId: %d\n", markerIds[r]);
      cout << "[r,p,y] " << rotationVectors.at(r)[0] <<", " << rotationVectors.at(r)[1] <<", " << rotationVectors.at(r)[2] << endl;
      cout << "[x,y,z] " << translationVectors.at(r)[0] <<", " << translationVectors.at(r)[1] <<", " << translationVectors.at(r)[2] << endl;
      printf("\n");
    }

    /*
     * Iteration 1: Apply translationVectors to known translation of first found
     * to get the actual location of the camera
     * TODO: Math is NOT correct. How to know if to add vs. subtract? or matrix multiplication?
     */
     int firstMarkerFound = markerIds[0];
     kinectActual[0] = translationVectors.at(0)[0] + translationVectorsToOrigin.at(firstMarkerFound)[0];
     kinectActual[1] = translationVectors.at(0)[1] + translationVectorsToOrigin.at(firstMarkerFound)[1];
     kinectActual[2] = translationVectors.at(0)[2] + translationVectorsToOrigin.at(firstMarkerFound)[2];

     // Print out results
     printf("Actual location of the camera wrt origin using MarkerId: %d\n", firstMarkerFound);
     cout << "[r,p,y] " << rotationVectors.at(0)[0] <<", " << rotationVectors.at(0)[1] <<", " << rotationVectors.at(0)[2] << endl;
     cout << "[x,y,z] " << kinectActual[0] <<", " << kinectActual[1] <<", " << kinectActual[2] << endl;
     printf("\n");


    return 1;
  }

  int startWebcamMonitoring( const Mat& cameraMatrix, const Mat& distanceCoefficients, bool showMarkers) {

    Mat frame;  //Hold frame of info from webcam

    vector<int> markerIds;
    vector< vector<Point2f> > markerCorners, rejectedCandidates;

    // Aruco part
    aruco::DetectorParameters parameters;

    Ptr<aruco::Dictionary> markerDictionary =
        aruco::getPredefinedDictionary( 0 );

    VideoCapture vid(0);  //TODO: When using on Machine with Kinect change to source 1 if not working!!!!

    if ( !vid.isOpened() ) {
      return -1;
    }

    namedWindow("Webcam", CV_WINDOW_AUTOSIZE);  //Makes the GUI

    vector<Vec3d> rotationVectors, translationVectors;

    while(true) {
      if (!vid.read(frame)) {
        break;
      }

      // Finds them
      aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);

      // Outline the markers and label with ids
      if (showMarkers) { aruco::drawDetectedMarkers(frame, markerCorners, markerIds); }

      // Estimate pose
      aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors );

      // Continually draw the axis
      if ( showMarkers ) {
        for ( int i = 0 ; i < markerIds.size() ; i++ ) {
          aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
          // printf("Marker %i found\n", markerIds[i] ); // Only print once but already shown visually
        }
        imshow("Webcam", frame);
      }

      char character;
      if ( character = waitKey(30) >= 0) break;

      // Once a good photo has been taken
      // press SPACE to exit for analysis
      if ( character == ' ' ) { break; }

    }
    // Perform analysis and determine actual location
    analyzeFrame( markerCorners, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors, markerIds );

    return 1;
  }

  void cameraCalibrationProcess( Mat& cameraMatrix, Mat& distanceCoefficients ) {
    Mat frame;
    Mat drawToFrame;

    vector<Mat> savedImages;

    vector< vector<Point2f> > markerCorners, rejectedCandidates;

    VideoCapture vid(0);

    if (!vid.isOpened()) { return; }

    int framesPerSecond = 20;

    namedWindow( "Webcam", CV_WINDOW_AUTOSIZE );

    while(true) {
      if (!vid.read(frame)) {
        break;
      }

      vector<Vec2f> foundPoints;
      bool found = false;

      found = findChessboardCorners( frame, chessboardDimensions, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE );
      frame.copyTo( drawToFrame );
      drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);
      if (found) {
        imshow("Webcam", drawToFrame);
      } else {
        imshow("Webcam", frame);
      }
      char character = waitKey(1000/framesPerSecond);

      switch (character) {
        case ' ':
          printf("SPACE: Looking for image\n");
          // Saving image
          if (found) {
            printf("Image found\n");
            Mat temp;
            frame.copyTo(temp);
            savedImages.push_back(temp);
          }
          break;
        case 'f':
          // Enter Key - start calibration
          // First check that we have enough
          printf("ENTER: Save File\n");
          printf("COUNT Image found: %i\n", savedImages.size());
          if ( savedImages.size() > 15 ) {
            // TODO: Issue with this call in my method to other method
            printf("Enters. Correct number of images found\n");
            cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDemension, cameraMatrix, distanceCoefficients);
            printf("Calibration Complete...Waiting to save file\n");
            saveCameraCalibration("ILoveCameraCalibration", cameraMatrix, distanceCoefficients);
            printf("SUCCESSFULLY saved calibration\n");
          }
          printf("Not enough images found\n");
          break;
        case 27:
          // ESC Key
          return;
          break;

      }
    }
  }

  // Given analysis figure out where the markers are
  // then take this and determine their location relation to OriginWorld
  vector<Point3f> getCornersInCameraWorld( Vec3d rvec, Vec3d tvec ) {

       double half_side = arucoSquareDimension/2;


       // compute rot_mat
       Mat rot_mat;
       Rodrigues(rvec, rot_mat);

       // transpose of rot_mat for easy columns extraction
       Mat rot_mat_t = rot_mat.t();

       // the two E-O and F-O vectors
       double * tmp = rot_mat_t.ptr<double>(0);
       Point3f camWorldE(tmp[0]*half_side,
                         tmp[1]*half_side,
                         tmp[2]*half_side);

       tmp = rot_mat_t.ptr<double>(1);
       Point3f camWorldF(tmp[0]*half_side,
                         tmp[1]*half_side,
                         tmp[2]*half_side);

       // convert tvec to point
       Point3f tvec_3f(tvec[0], tvec[1], tvec[2]);

       // return vector:
       vector<Point3f> ret(4,tvec_3f);

       ret[0] +=  camWorldE + camWorldF;
       ret[1] += -camWorldE + camWorldF;
       ret[2] += -camWorldE - camWorldF;
       ret[3] +=  camWorldE - camWorldF;

       return ret;
  }

  int main(int argv, char** argc) {

    //createArucoMarkers(); // Done

    // Add elements to translationVectorsToOrigin vector
    translationVectorsToOrigin.push_back(mk0);
    translationVectorsToOrigin.push_back(mk1);
    translationVectorsToOrigin.push_back(mk2);

    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
    Mat distanceCoefficients;
    vector<int> markerIds;
    vector< vector<Point2f> > markerCorners;

    // Not working, using temp file
    // cameraCalibrationProcess( cameraMatrix, distanceCoefficients );

    loadCameraCalibration("KinectCalibration", cameraMatrix, distanceCoefficients);
    startWebcamMonitoring( cameraMatrix, distanceCoefficients, true);

    /* TRANSLATION TO ACTUAL IMPLIMENTATION */
    // TODO:
      // Create getKinectLocation
      // Copy rVector to kinectActual
      // Determine how to translate to Karl's API
      // Fix calibration
    /*
     * One time:
     * + calibrate camera (still figuring out) > save to KinectCalibration
     * + Update translationVectorsToOrigin with actual location of markers
     *
     * Upon start of system:
     * + Push elements to the translationVectorsToOrigin vector
     * + Create cameraMatrix, distanceCoefficients, markerIds, markerCorners
     * + loadCameraCalibration("KinectCalibration", cameraMatrix, distanceCoefficients);
     *
     * Each movement:
     * + startWebcamMonitoring(cameraMatrix, distanceCoefficients, false);
     * + Update startWebcamMonitoring to automatically stop after one frame of > 1000 mill
     * + call getKinectLocation to get actual
     * + moveCamera(current, desired)
        // If within error, return, else call Karl's API and moveCamera
     */

  }
