/*
 * Senior Design 2018 - Hatata & Magnan
 * The George Washington University
 *
 * This API focuses on the algorithms to support our modeling
 * functionality.
 * It relies on a previous Senior Design Project (Karl Presner)
 * for motor execution and communicated with via Python-C++ Socket server
 *
 * This document is broken down into a few sections:
 * (I)    Includes
 * (II)   Constants for calibration and current Environment
 * (III)  Creation of Aruco Markers for printing
 * (IV)   Camera Calibration
 * (V)    Saving Images for testing
 * (VI)   Calibration Analysis
 * (VII)  Importing saved images for analysis
 * (VIII) Obtaining live image for ANALYSIS
 * (IX)   Client for Python-C Server
 * (X)    Actuation
 *
 */

 /* (I) INCLUDES + NAMESPAGES */

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
 /*
 #include<stdio.h>      //printf
 #include<string.h>     //strlen    // Might need these for the socket communication if not working.
 #include<string>       //string
 */

 using namespace std;
 using namespace cv;
 using std::vector;

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
 /* TODO: Update these values with those recorded. */
 std::vector<Vec3d> translationVectorsToOrigin;
 Vec3d mk0 (0.040f, 0.304f, 0);
 Vec3d mk1 (0.134f, 0.040f, 0);
 Vec3d mk2 (0.148f, 0.204f, 0);
 Vec3d mk3 (0.148f, 0.204f, 0);

 // Global variable to determine where the Kinect actually is
 Vec3d kinectActual (0, 0, 0);

 /* (III) CREATION OF ARUCO MARKERS */

 // Only execute on initial installation of API
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

 /* (IV) CAMERA CALIBRATION */

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
         //cout << cameraMatrix.at<double>(r,c) << "\n";
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
         //cout << distanceCoefficients.at<double>(r,c) << "\n";
       }
     }

     inStream.close();
     return true;
   }

   return false;
 }

 // Function that is called upon installation and recalibration
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


 /* (V) SAVING IMAGES */
 // Currently in it's own function save_photos.cpp with build of ./save [image_name.png]

 /* (VI) CALIBRATION ANALYSIS */
 int analyzeFrame( vector< vector<Point2f> > markerCorners, vector<Vec3d> rotationVectors, vector<Vec3d> translationVectors, vector<int> markerIds ) {

   printf("Successfully enters analyzeFrame\n");
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
    printf("%i Markers found, determining location of Kinect using Marker %i\n\n", markerIds.size(), markerIds[0] );
    int firstMarkerFound = markerIds[0];
    kinectActual[0] = translationVectors.at(0)[0] + translationVectorsToOrigin.at(firstMarkerFound)[0];
    kinectActual[1] = translationVectors.at(0)[1] + translationVectorsToOrigin.at(firstMarkerFound)[1];
    kinectActual[2] = translationVectors.at(0)[2] + translationVectorsToOrigin.at(firstMarkerFound)[2];

    // Print out results
    printf("Actual location of the camera wrt origin using MarkerId: %d\n", firstMarkerFound);
    cout << "[r,p,y] " << rotationVectors.at(0)[0] <<", " << rotationVectors.at(0)[1] <<", " << rotationVectors.at(0)[2] << endl;
    cout << "[x,y,z] " << kinectActual[0] <<", " << kinectActual[1] <<", " << kinectActual[2] << endl;
    printf("\n");

    //Print out kinectActual to ensure correct

   return 1;
 }

 /* (VII) IMPORTING SAVED IMAGE FOR ANALYSIS */
 int obtainSavedImage( string name, const Mat& cameraMatrix, const Mat& distanceCoefficients, bool showWindow) {

   // Bring in the file that we are loading image from
   ifstream inStream;
   inStream.open(name.c_str());

   Mat frame;  //Hold frame of info from image

   vector<int> markerIds;
   vector< vector<Point2f> > markerCorners, rejectedCandidates;

   // Aruco part
   aruco::DetectorParameters parameters;

   Ptr<aruco::Dictionary> markerDictionary =
       aruco::getPredefinedDictionary( 0 );

   // Opening Image
   printf("Attempting to open %s\n", name.c_str());
   string projectPath = "/home/tjmagnan/sd-18-hatata_magnan/calibration_actuation";
   string fullPath = projectPath + name.c_str();
   try {
     frame = imread(fullPath);
   }
   catch (runtime_error& ex) {
       fprintf(stderr, "Exception importing image: %s\n", ex.what());
       return 1;
   }
   printf("Image successfully opens!\n");

   if (showWindow) {
     printf("Opening window ...\n");
     namedWindow(name.c_str(), CV_WINDOW_AUTOSIZE);  //Makes the GUI
     printf("Successfully built Window\n");
   }

   vector<Vec3d> rotationVectors, translationVectors;
   printf("Successfully created rotationVectors and translationVectors...\n");

   // Finds them
   aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);

   // Outline the markers and label with ids
   if ( showWindow ) { aruco::drawDetectedMarkers(frame, markerCorners, markerIds); }

   // Estimate pose
   aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors );

   // Continually draw the axis
   if ( showWindow ) {
     for ( int i = 0 ; i < markerIds.size() ; i++ ) {
       aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
       // printf("Marker %i found\n", markerIds[i] ); // Only print once but already shown visually
     }
     imshow(name.c_str(), frame);
   }

   if ( showWindow ) {
     while(true) {
      imshow(name.c_str(), frame);

       char character;
       if ( character = waitKey(30) >= 0) break;

       // Once a good photo has been taken
       // press SPACE to exit for analysis
       if ( character == ' ' ) { break; }
     }
   }

   // Perform analysis and determine actual location
   printf("Desired image aquired and markers found. Continuing to analyse image...\n");
   analyzeFrame( markerCorners, rotationVectors, translationVectors, markerIds );

   return 1;
 }

 /* (VIII) OBTAINING LIVE IMAGE FOR ANALYSIS */
 int obtainLiveImage( const Mat& cameraMatrix, const Mat& distanceCoefficients, bool showMarkers) {

   Mat frame;  //Hold frame of info from webcam

   vector<int> markerIds;
   vector< vector<Point2f> > markerCorners, rejectedCandidates;

   // Aruco part
   aruco::DetectorParameters parameters;

   Ptr<aruco::Dictionary> markerDictionary =
       aruco::getPredefinedDictionary( 0 );

   // Use for Kinect
   printf("Initiating VideoCapture\n");
   VideoCapture vid( CAP_OPENNI );
   //VideoCapture vid(CAP_OPENNI_GRAY_IMAGE);

   if ( !vid.isOpened() ) {
     printf("Video fails to open\n");
     return -1;
   }

   printf("Video successfully open!\n");
   printf("Opening window ...\n");

   namedWindow("Webcam", CV_WINDOW_AUTOSIZE);  //Makes the GUI

   printf("Successfully built Window\n");


   vector<Vec3d> rotationVectors, translationVectors;
   printf("Successfully created rotationVectors and translationVectors...\n");

   //while (true) {}

   while(true) {
     if (!vid.read(frame)) {
     //if (!vid.retrieve(frame, CAP_OPENNI_BGR_IMAGE)) {
       printf("Unable to read current frame. Exiting\n");
       return -1;
     }

     // Alters image type
     vid.retrieve(frame, CAP_OPENNI_BGR_IMAGE);

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
   printf("Desired frame aquired. Continuing to analyse frame...\n");
   analyzeFrame( markerCorners, rotationVectors, translationVectors, markerIds );

   return 1;
 }

 /* (IX) CLIENT FOR Python-C */

 // Source: http://www.binarytides.com/code-a-simple-socket-client-class-in-c/

 /**
     TCP Client class
 */
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

 /**
     Connect to a host on a certain port number
 */
 bool tcp_client::conn(string address , int port)
 {
     //create socket if it is not already created
     if(sock == -1)
     {
         //Create socket
         sock = socket(AF_INET , SOCK_STREAM , 0);
         if (sock == -1)
         {
             perror("Could not create socket");
         }

         cout<<"Socket created\n";
     }
     else    {   /* OK , nothing */  }

     //setup address structure
     if(inet_addr(address.c_str()) == -1)
     {
         struct hostent *he;
         struct in_addr **addr_list;

         //resolve the hostname, its not an ip address
         if ( (he = gethostbyname( address.c_str() ) ) == NULL)
         {
             //gethostbyname failed
             herror("gethostbyname");
             cout<<"Failed to resolve hostname\n";

             return false;
         }

         //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
         addr_list = (struct in_addr **) he->h_addr_list;

         for(int i = 0; addr_list[i] != NULL; i++)
         {
             //strcpy(ip , inet_ntoa(*addr_list[i]) );
             server.sin_addr = *addr_list[i];

             cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;

             break;
         }
     }

     //plain ip address
     else
     {
         server.sin_addr.s_addr = inet_addr( address.c_str() );
     }

     server.sin_family = AF_INET;
     server.sin_port = htons( port );

     //Connect to remote server
     if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
     {
         perror("connect failed. Error");
         return 1;
     }

     cout<<"Connected\n";
     return true;
 }

 /**
     Send data to the connected host
 */
 bool tcp_client::send_data(string data)
 {
     //Send some data
     if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
     {
         perror("Send failed : ");
         return false;
     }
     cout<<"Data send\n";

     return true;
 }

 /**
     Receive data from the connected host
 */
 string tcp_client::receive(int size=512)
 {
     char buffer[size];
     string reply;

     //Receive a reply from the server
     if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
     {
         puts("recv failed");
     }

     reply = buffer;
     return reply;
 }

 // Communication with Karl's python code
 int socket_request(string data) {
   tcp_client c;

   // Connect to host
   c.conn("localhost", 5000);

   // Sending data
   c.send_data(data);

   // Recieve and echo reply data
   cout<<"----------------------------\n\n";
   cout<<c.receive(1024);
   cout<<"\n\n----------------------------\n\n";
   cout<<"--COMMUNICATION SUCCESSFUL--\n\n";
   cout<<"----------------------------\n\n";

   // Motors have been moved
   return 0;
 }

 /* (X) ACTUATION */

 // Conversion functions
 Vec3d cartesian_to_polar( Vec3d cartesian ) {
   double r, theta = 0;
   Vec3d polar ( r, theta, cartesian[2] );
   r = sqrt( cartesian[0]*cartesian[0] + cartesian[1]*cartesian[1] );
   theta = atan( cartesian[1]/cartesian[0] );

   return polar;
 }

 Vec3d polar_to_cartesian( Vec3d polar ) {
   double x, y = 0;
   x = polar[0] * cos( polar[1] );
   y = polar[0] * sin( polar[1] );
   Vec3d cartesian ( x, y, cartesian[2] );
   return cartesian;
 }

 // Basic functions when location isn't specified

 int down() {
   // open one x2 close the other
 }

 int up() {
   // open one x2 close the other - inverse
 }

 int left() {
   // Stepper motor
   socket_request("move(3, 10)");
 }

 int right() {
   // stepper motor
 }

 int rotateCamera() {
   // open one x2 close the other
 }

 // Error correct
 int error_correct( Vec3d desired_endpoint ) {
     //TODO: Get actual location of camera (actual_pt)
         //TODO: Call upon Arucu & opencv libraries
     //TODO: Margin of error mesuring
     //TODO: If within, simply return | else continue
     //TODO: Not within error, call move function between points
     //TODO: move( actual_pt, desired_endpoint )
 }

 // Move to location
 int move( Vec3d old_pt, Vec3d new_pt ) {
     //TODO: Get differences in theta and move stepper motor
     double theta_diff = new_pt[1] - old_pt[1];

     //TODO: Get differences in r(radius)
     double r_diff = new_pt[0] - old_pt[0];

     //TODO: Get differences in z
     double z_diff = new_pt[2] - old_pt[2];

     //TODO: Determine what the z_diff translates to in stepper movements

     //TODO: Determine the combination of movements of other motors to get to correct r & z
         // Longer term

     //TODO: Ensure that moves are valid, if not change to valid, or throw error (return -1;)
      // Within min and maximum of Motor movements

     //TODO: Iniate Socket communication and Call Karl's API to move points
        // Stepper motor for z
        // Continued

     //TODO: Call calibration function
     error_correct( new_pt );
     return 1;
 }

 //Initial calls within any running instance
 void load(int argc, char** argv) {
   // Add elements to translationVectorsToOrigin vector
   translationVectorsToOrigin.push_back(mk0);
   translationVectorsToOrigin.push_back(mk1);
   translationVectorsToOrigin.push_back(mk2);
   translationVectorsToOrigin.push_back(mk3);

   loadCameraCalibration("KinectCalibration", cameraMatrix, distanceCoefficients);
 }
