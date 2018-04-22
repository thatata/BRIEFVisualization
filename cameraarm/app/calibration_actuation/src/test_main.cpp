#include "calibration_actuation.h"

int main(int argc, char** argv) {
  /**/
  load(argc, argv);

  bool showWindow = false;
  if (argc > 2 ) {
    if ( strcmp(argv[2], "true") == 0) { printf("print\n"); showWindow = true; }
  }

  obtainSavedImage(argv[1], cameraMatrix, distanceCoefficients, showWindow);
  write_intrinsics("test.txt");
  /**/

  // obtainMostRecentImage();
  // Vec3d testInput (0, 0, 0);
  //
  // string file = new_pose( testInput );
  // cout << file << endl;

  /*
  socket_request("4,100");
  printf("Socket request completed.\n");
  sleep(50);
  socket_request("Disconnect");
  printf("Server shutdown successfully...\n");


  exit(0);
` */
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
