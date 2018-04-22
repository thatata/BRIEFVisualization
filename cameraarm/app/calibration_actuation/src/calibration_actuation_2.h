#ifndef CALIBRATION_ACTUATION_H
#define CALIBRATION_ACTUATION_H

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
 #include <math.h>
 #include <string.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <dirent.h>
 #include <errno.h>
 #include <unistd.h>

 using namespace std;
 using namespace cv;
 using std::vector;

 int move( Vec3d, Vec3d );

 /* (II) CONSTANTS */

 // Constants for Calibration
 const float calibrationSquareDemension = .0239f; //meters
 const float arucoSquareDimension = .160f;   //meters - 160mm = 16cm
 const Size chessboardDimensions = Size(6, 9); //Size of calibration board (given)

 // Runtime Constants
 Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
 Mat distanceCoefficients;
 vector<int> markerIds;
 vector< vector<Point2f> > markerCorners;

 // Known location of markers in Test Environment
 std::vector<Vec3d> translationVectorsToOrigin;
 Vec3d mk0 (0, -0.4318f, 0);
 Vec3d mk1 ( 0.4318f, 0, 0);
 Vec3d mk2 (0,  0.4318f, 0);
 Vec3d mk3 (-0.4318f, 0, 0);

 // All markers have NOT been rotated with respect to the origin
 Vec3d markerRotation (0, 0, 0);

 // Global variable to determine where the Kinect actually is
 Vec3d kinectActual (0, 0, 0);
 Vec3d kinectRVec   (0, 0, 0);  // In Euler notation NOT rotation Matrix
 Mat kinectRotationMatrix = Mat::eye(3, 3, CV_64F);

 /* (III) CREATION OF ARUCO MARKERS */

 // Only execute on initial installation of API
 void createArucoMarkers();

 void createKnowBoardPositions(Size , float , vector<Point3f>  );

 void getChessboardCorners( vector<Mat> , vector< vector<Point2f> >& , bool );

 void cameraCalibration( vector<Mat> , Size , float , Mat& , Mat&  );

 bool saveCameraCalibration( string , Mat , Mat  );

 // Fully working. Takes in precalculated calibration values of the camera for analysis
 bool loadCameraCalibration( string , Mat& , Mat&  );

 // Function that is called upon installation and recalibration
 void cameraCalibrationProcess( Mat&, Mat&);

 int analyzeFrame( vector< vector<Point2f> > markerCorners, vector<Vec3d> rotationVectors, vector<Vec3d> translationVectors, vector<int> markerIds );

 int obtainSavedImage( string, const Mat&, const Mat&, bool);

 int obtainLiveImage( const Mat&, const Mat&, bool);

 class tcp_client
 {
 private:
     int sock;
     std::string address;
     int port;
     struct sockaddr_in server;

 public:
     tcp_client();
     bool conn(string, int);
     bool send_data(string data);
     string receive(int);
 };

 tcp_client::tcp_client()
 {
     sock = -1;
     port = 0;
     address = "";
 }

 bool tcp_client::conn(string, int);

 bool tcp_client::send_data(string );

 string tcp_client::receive(int);

 // Communication with Karl's python code
 int socket_request(string data);


// Write camera intrinsics to file for Tarek to pull
bool write_intrinsics( string name );

 // Conversion functions
 Vec3d cartesian_to_polar( Vec3d cartesian );

 Vec3d polar_to_cartesian( Vec3d polar );

 // Ensure that the final location of the Kinect Camera is within the bounds of the robotic arm
 bool validLocation( Vec3d originalPolar );

 double* get_mtr_movements( Vec3d, Vec3d );

 // When running using KinectScan.cpp rather than obtaining live images they are already happing in the background
 // This function returns the name of the pcl and jpg images to analyze
 string obtainMostRecentImage();

 // Within marvin of error calculation using MSE
 bool approxEquals (Vec3d, Vec3d, double);

 // Error correct
 void error_correct( Vec3d  );

 // Move to location
 // Takes in cartesian coordinates
 int move( Vec3d , Vec3d  );

 // Public API
 // Takes in a desired endpoint, determines the current location and returns the string of the name of the image from that pose (jpg and pcl)
 // Note: user can manually pull kinectActual from global after running new_pose()
 string new_pose( Vec3d  );

 //Initial calls within any running instance
 void load();

#endif
