#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string>

#include "Header.h"
#include "Input.h"

using namespace cv;
using namespace std;

// INPUT MODE SELECT
#define INPUT_NONE   ( 1u)
#define INPUT_FILE   ( 2u)
#define INPUT_VIDEO  ( 3u)
#define INPUT_CAMERA ( 4u)

#define INPUT_MODE  INPUT_FILE

// INPUT MODE SELECT
#define OUTPUT_NONE  ( 1u)
#define OUTPUT_FILE  ( 2u)
#define OUTPUT_VIDEO ( 3u)
#define OUTPUT_MODE  OUTPUT_NONE 

static int threshBinary = 150;//80;
static int iXVal = 1;//80;
static int iYVal = 1;//80;

//int mainRectangle(int argc, char** argv)
int main(int argc, char** argv)
{
  char cKey;
  char cFindKey = 'r';

  // INPUT
  int iCameraNum = 0;
  VideoCapture video_in;
  int iInputMode = INPUT_VIDEO; //INPUT_FILE;
  int IFileIdx = 1;
  int iLstCnt = 0x3F; // should be a mask
  //const char *sVIn = ".\\vids\\TSignSensor.avi"; //".\\vids\\movie_2_TireView.avi"; //"dummy.mp4";
  const char *sVIn = ".\\vids\\Left_right_TS_10Sec.avi"; //movie_2_TireView.avi"; //"dummy.mp4";
  //const char *sVIn = ".\\vids\\movie_2_TireView.avi"; //"dummy.mp4";

  // OUTPUT
  int iOutputMode = OUTPUT_MODE; //INPUT_FILE;
  Size FrameSize;
  const char *sVOut = ".\\vids\\Out_movie_2_TireView.avi"; //"dummy.mp4";

  // PARAMETERS
  double fps;
  double dMaxFrames;
  double dFrameNum;

  // PROCESS
  Mat image;
  std::string strImgSrc = "Video or Cam"; // Default to cam or video.

  // Select the algo to test
  cFindKey = '3';

  // Input setup
  if (iInputMode == INPUT_CAMERA) {
    video_in.open(iCameraNum);
    if (!video_in.isOpened()) { cerr << "Couldn't open Camera" << iCameraNum << endl; return -1; }

  } else if (iInputMode == INPUT_VIDEO) {
cout << "A" << endl;
    video_in.open(sVIn);
    if (!video_in.isOpened()) { cerr << "Couldn't open " << sVIn << endl; return -1; }

    fps = video_in.get(CV_CAP_PROP_FPS);
    dMaxFrames = video_in.get(CV_CAP_PROP_FRAME_COUNT);
    dFrameNum = 0;
cout << "B" << endl;
    printf("Init video file image. fps:%f max:%f, cur:%f\n", fps, dMaxFrames, dFrameNum);

  }


#if (OUTPUT_MODE == OUTPUT_VIDEO)
  // Output setup
  FrameSize = Size((int)video_in.get(CV_CAP_PROP_FRAME_WIDTH),
      (int)video_in.get(CV_CAP_PROP_FRAME_HEIGHT));

  VideoWriter videoOut(sVOut, CV_FOURCC('M', 'J', 'P', 'G'), 
      video_in.get(CV_CAP_PROP_FPS), FrameSize);
  
  if (!videoOut.isOpened()) {
    cout << "Could not open the output video: " << sVOut << endl;
    getchar();
    return -1;
  }
#endif

  // Start process
  namedWindow("Image", WINDOW_NORMAL); resizeWindow("Image", Size(640, 480));

  for (;;) {

    if (iInputMode != INPUT_FILE) {
      // to loop the video
      dFrameNum++;
      if (dFrameNum >= dMaxFrames) {
        video_in.set(CV_CAP_PROP_POS_FRAMES, 0);
        dFrameNum = 0;
      }
      // get the frame
      video_in >> image; 

    } else {
      // get the required image
      strImgSrc = aFileNames[IFileIdx];
      image = imread(strImgSrc.c_str());
      printf("F: %s, i:%d\n", strImgSrc.c_str(), IFileIdx); 
    }

    if (image.empty()) { goto errImage;}

    switch(cFindKey) {
      case '1': FindShapes(strImgSrc, image); break; 
      //case '2': FindDisparity(strImgSrc, image); break;
      case '2': Disparity(image); break;
      case '3': Lines(strImgSrc, image); break; 
      case '4': FindRectangles(strImgSrc, image); break;
      case '5': DistanceToLane_Main("DistanceToLine", image); break;
      default:  continue;
    }

#if (OUTPUT_MODE == OUTPUT_VIDEO)
    if (iOutputMode == OUTPUT_VIDEO) { videoOut << image; } //imwrite( "out", image );
#endif


    imshow("Image", image);

    cKey = waitKey(100);

    if (cKey == 27) break; // ESC
    else if (cKey == 32) { IFileIdx = ++IFileIdx & iLstCnt; }
    else if (cKey == '1' || cKey == '2' || cKey == '3' || cKey == '4' 
        || cKey == '5' || cKey == '6' || cKey == '7') {
      cFindKey = cKey;
    }
    else if (cKey == 'p') { 
      printf("\n PAUSED !!. Press r or R to resume. ESC keys to Exit\n");
      while (1) {
        char key = (char)waitKey(10);
        if (key == 27 || key == 'r' || key == 'R' || key == 32) // 'ESC'
          break;
      }
    }
  }

  return 0;

errImage:
  cout << "Main: Couldn't get image! Check INPUT!";
  getchar();
  return -1;

}

// References:
// 
// ** Contour provides the shapes, edge detected output.
// The below doc provides how to identify different shapes using Contour output.
// https://docs.opencv.org/3.1.0/dd/d49/tutorial_py_contour_features.html

// Helpful to know"

//  Moment
// https://en.wikipedia.org/wiki/Image_moment
// https://en.wikipedia.org/wiki/Moment_(mathematics)



// Filters:
// 1. Bilateral
//    Bilateral filtering forms a very good way to preserve edges. It is a 
//    non-linear filter and helps reduce noise. The concept of this filter is 
//    that, at each pixel, its value is substituted by the average of the 
//    neighbourhood pixels. That gives the smooth effect when this filter is
//    applied.

// Processing
// 1. approxPolyDP. 
//    What does this do? Well, this function helps to find what kind of 
//    polygon is the contour. How? It gives the output as number of vertices
//    for the polygon. For eg., a square contour will give the output as 4 
//    points, pentagon as 5 points and so on. Circular objects will have higher
//    number of points. Here, we see that depending on the type of objects in 
//    the image, polygons with greater than 8 vertices form curvier shapes, 
//    here circle and ellipse.
//    E.g. For circle? The higher the percentage, the lower the number of 
//         vertices. We are therefore using a very low percentage to find 
//         details of the contour

// Shape detection
// 1. Method-1
//    https://www.pyimagesearch.com/2016/02/08/opencv-shape-detection/
//    Step-1: Resize the image
//    Step-2: Grayscale
//    Step-3: GaussianBlur, (5,5), 0
//    Step-4: threshold, 60, 255, THRES_BINARY
//    Step-5: findCountour RETR_EXTERNAL, CHAIN_APPROX_SIMPLE
//    Step-6:*arcLenght(c, true)
//    Step-7: approxPolyDP(,4% of ArcLength, True)
//    Step-8: Len(approx): 3=> Triangle, 4=> Sq/Rec, boundingbox, aspect ration ~2 => Sq, 5=>pentago, else circle
//    Step-

// 2. Method-2 (ZIM-101)
//    Fast radial symmetry transform & Harris corner detector.
//    Circle: fast radial symmetry detection method (FRS) introduced in [5] 
//    has been employed for circles recognition. This algorithm outputs a 
//    grayscale map with a peak at that position, where any circle is present
//    in the input image. Thus we get ROI.
//    The other two basic shapes(triangle and rectangle) are distinguished by 
//    means of the Harris corner detector. The Harris corner detector gives a 
//    strong response at the four principal corners of the ROI in case of 
//    rectangular object in the input image and the three corners in case of 
//    triangular object.
//    Question: 1st, how to find ROI for rectangular and triangle. ? color based ? or line segment ? etc.

// Circles
// Method-1
//   http://layer0.authentise.com/detecting-circular-shapes-using-contours.html
//   Step-1: Bilateral Filtering
//   Step-2: Edge Detection
//   Step-3: Finding contours
//  *Step-4: approxPolyDP with low percentage of arcLength or perimeter of the contour
//   Step-5: Area of contour & eliminate very small objects.

// Examples
// // source: https://github.com/bsdnoobz/opencv-code/blob/master/shape-detect.cpp
