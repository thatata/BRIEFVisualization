#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void createAlphaMat(Mat &mat)
{
    CV_Assert(mat.channels() == 4);
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            Vec4b& bgra = mat.at<Vec4b>(i, j);
            bgra[0] = UCHAR_MAX; // Blue
            bgra[1] = saturate_cast<uchar>((float (mat.cols - j)) / ((float)mat.cols) * UCHAR_MAX); // Green
            bgra[2] = saturate_cast<uchar>((float (mat.rows - i)) / ((float)mat.rows) * UCHAR_MAX); // Red
            bgra[3] = saturate_cast<uchar>(0.5 * (bgra[1] + bgra[2])); // Alpha
        }
    }
}

int main(int argv, char **argc)
{
    // Create mat with alpha channel
    Mat frame(480, 640, CV_8UC4);
    createAlphaMat(frame);

    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

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


    while(true) {
      if (!vid.read(frame)) {
      //if (!vid.retrieve(frame, CAP_OPENNI_BGR_IMAGE)) {
        printf("Unable to read current frame. Exiting\n");
        return -1;
      }

      // Alters image type
      vid.retrieve(frame, CAP_OPENNI_BGR_IMAGE);

      char character;
      if ( character = waitKey(30) >= 0) break;

      // Once a good photo has been taken
      // press SPACE to exit for analysis
      if ( character == ' ' ) { break; }

    }

    try {
        imwrite("alpha.png", frame, compression_params);
    }
    catch (runtime_error& ex) {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
        return 1;
    }

    fprintf(stdout, "Saved PNG file with alpha data.\n");
    return 0;
}
