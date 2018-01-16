#include "calibration_actuation.h"

int main(int argc, char** argv) {

  load(argc, argv);

  obtainSavedImage(argv[1], cameraMatrix, distanceCoefficients, showWindow);

  socket_request("x+1");

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
