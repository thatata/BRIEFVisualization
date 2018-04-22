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

class calibration_actuation {
  private:
    void createArucoMarkers();
    void createKnowBoardPositions(Size, float, vector<Point3f>);
    void getChessboardCorners( vector<Mat>, vector< vector<Point2f> >&, bool);
    void cameraCalibration( vector<Mat>, Size, float, Mat&, Mat&);
    bool saveCameraCalibration( string, Mat, Mat);
    void cameraCalibrationProcess( Mat&, Mat& );
    int analyzeFrame( vector< vector<Point2f> >, vector<Vec3d>, vector<Vec3d>, vector<int> );
    int obtainSavedImage(string, const Mat&, const Mat&, bool);
    int obtainLiveImage(const Mat&, const Mat&, bool);
    int socket_request(string);
    bool write_intrinsics(string);
    Vec3d cartesian_to_polar(Vec3d);
    Vec3d polar_to_cartesian(Vec3d);
    bool validLocation(Vec3d);
    double* get_mtr_movements(Vec3d, Vec3d);
    string obtainMostRecentImage();
    bool approxEquals(Vec3d, Vec3d, double);
    void error_correct(Vec3d);
    int move(Vec3d, Vec3d);
    bool loadCameraCalibration(string, Mat&, Mat&);

  public:
    calibration_actuation();
    string new_pose(Vec3d);
    void load();
};

#endif
