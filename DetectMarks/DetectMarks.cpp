

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string>

#include"Header.h"
#include"Input.h"

using namespace cv;
using namespace std;

// INPUT MODE SELECT
#define INPUT_FILE   ( 1u)
#define INPUT_VIDEO  ( 2u)
#define INPUT_CAMERA ( 3u)

#define INPUT_MODE  INPUT_FILE

static int threshBinary = 150;//80;
static int iXVal = 1;//80;
static int iYVal = 1;//80;

//int mainRectangle(int argc, char** argv)
int main(int argc, char** argv)
{
  char cKey;
  char cFindKey = 'r';

  int iInputMode = INPUT_VIDEO; //INPUT_FILE;
  int iFileNameIndex = 1;
  int iListCount = 0x3F; // should be a mask

  VideoCapture video_in;
  
  int iCameraNum = 0;
  const char *strVideoName = ".\\vids\\cmLaneFishView.avi"; //"dummy.mp4";
  double fps = video_in.get(CV_CAP_PROP_FPS);
  double dMaxFrames = video_in.get(CV_CAP_PROP_FRAME_COUNT);
  double dFrameNum;

  std::string strImgSrc = "Video or Cam"; // Default to cam or video.

  if (iInputMode == INPUT_CAMERA) {
    video_in.open(iCameraNum);
    if (!video_in.isOpened()) { cerr << "Couldn't open Camera" << iCameraNum << endl; return -1; }
  } 


  if (iInputMode == INPUT_VIDEO) {
    video_in.open(strVideoName);
    if (!video_in.isOpened()) { cerr << "Couldn't open " << strVideoName << endl; return -1; }

    fps = video_in.get(CV_CAP_PROP_FPS);
    dMaxFrames = video_in.get(CV_CAP_PROP_FRAME_COUNT);
    dFrameNum = 0;
    printf("Init video file image. max:%f, cur:%f\n", dMaxFrames, dFrameNum);

	
	getchar();
  }

  namedWindow("Image", WINDOW_NORMAL); resizeWindow("Image", Size(640, 480));
  //namedWindow("Crop", WINDOW_NORMAL); resizeWindow("Crop", Size(640, 480));
  //namedWindow("Canny", WINDOW_NORMAL); resizeWindow("Canny", Size(640, 480));
  //namedWindow("Blur", WINDOW_NORMAL); resizeWindow("Blur", Size(640, 480));
  namedWindow("Thresold", WINDOW_NORMAL); resizeWindow("Thresold", Size(640, 480));


  cFindKey = 't';
  //createTrackbar("T2", "Canny", &threshBinary, 255, NULL);
  //createTrackbar("X", "Canny", &iXVal, 5, NULL);
  //createTrackbar("Y", "Canny", &iYVal, 5, NULL);

  for (;;) {

    Mat image;

    if (iInputMode != INPUT_FILE) {
      // to loop the video
      dFrameNum++;
      if (dFrameNum >= dMaxFrames) {
        video_in.set(CV_CAP_PROP_POS_FRAMES, 0);
        dFrameNum = 0;
      }
      video_in >> image; 
	    imshow("Image", image);
    }
    else {

      strImgSrc = aFileNames[iFileNameIndex];
      image = imread(strImgSrc.c_str());

      cout << "File# " << iFileNameIndex << " Name: " << strImgSrc << endl;

    }

    if (image.empty()) { goto errImage;}

//#if 0
//    Rect2d r = selectROI("Image", image, true, false);
//    printf("ROI: %f %f %f %f\n", r.x, r.y, r.width, r.height);
//
//#else
//    Rect2d roi(355, 859, 331, 70);
//	// draw the tracked object
//	rectangle(image, roi, Scalar(255, 0, 0), 2, 1);
//
//	
//#endif
	
	// Crop image
//	Mat imCrop = image(roi);
//	// Display Cropped Image
//	//imshow("Crop", imCrop);
//	
//	cv::Mat gray;
//
//	cv::cvtColor(imCrop, gray, CV_BGR2GRAY);
//	
//	cv::Mat bw, blurMat, OutMat, sobelMat;
//	
//	//GaussianBlur(gray, blurMat, Size(9, 9), 0, 0);
//
//	//cv::Canny(blurMat, bw, 0, 50, 5);
//	// TODO: Convolution Kernel matrix with our own values in matrix that best suits the senario!
//
//	// dilate canny output to remove potential holes between edge segments
//	//dilate(bw, gray, Mat(), Point(-1, -1));
//	cv::threshold(gray, OutMat, threshBinary, 255, THRESH_BINARY);
//	
//	//cv::Canny(OutMat, bw, 0, 50, 5);
//	
//	//Sobel(OutMat, sobelMat, CV_8U, 1, 1);
//	//Sobel(OutMat, sobelMat, CV_8U, iXVal, iYVal);
//
//	imshow("Thresold", image);
//	//imshow("Blur", bw);
//	//imshow("Crop", sobelMat);
//	//FindShapes("LANE", OutMat);
//
//	//imshow("Image", image);
//
//	cKey = waitKey(20);
	//printf("Show image. max:%f, cur:%f\n", dMaxFrames, dFrameNum);
	
	//getchar();
	//continue;

	////////////////////////////
    switch(cFindKey) {
      case 'c': case 'e': //FindEllipse(strImgSrc, image); break;
        printf("c %s, i:%d\n", strImgSrc.c_str(), iFileNameIndex); 
        FindShapes(strImgSrc, image); 
        break; 
	  case 'd': //Find disparity
		  printf("c %s, i:%d\n", strImgSrc.c_str(), iFileNameIndex);
		  FindDisparity(strImgSrc, image);
		  break;
      case 'l': 
        //Lines(strImgSrc, image); 
		
        break; //FindRectangles(strImgSrc, image); break;

      case 'r': case 's': FindRectangles(strImgSrc, image); break;
      case 't': 

      printf("Go into DistanceToLane_Main\n");
		  DistanceToLane_Main("DistanceToLine", image);
      getchar();
		  break; //FindRectangles(strImgSrc, image); break;
      default:
                continue;

    }


    //imwrite( "out", image );

    cKey = waitKey(2000);
    if (cKey == 27) break; // ESC
    else if (cKey == 32) iFileNameIndex = ++iFileNameIndex & iListCount; // ENTER
    else if (cKey == 'c' || cKey == 'd' || cKey == 'e' || cKey == 'l' || cKey == 'r' || cKey == 's' || cKey == 't')
      cFindKey = cKey;

  }

  return 0;

errImage:
  cout << "Couldn't get image!";  
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
