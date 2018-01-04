#include "calibration_actuation.h"

int main(int argv, char** argc) {

  //createArucoMarkers(); // Done

  // Add elements to translationVectorsToOrigin vector
  /* TODO: Must be copied into final main file with modeling. */
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
  obtainSavedImage(argc[1], cameraMatrix, distanceCoefficients, true);

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
