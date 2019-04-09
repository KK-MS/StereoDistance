/**
* Simple lane distane calculator
* It loads an image and tries to find the center.
*  Step 1: Consider the ROI
*  Step 2: Consider the vertical center point (x=0, Ycenter)
*  Step 3: Scan the row from the (x from 0 to end/white, Ycenter)
*  Step 4: Starting point: After calibration, this can also be fixed OR Note the first white it is starting point
*  Step 5: End point: Note the second white it is ending point. OR Not that 
*          after calibrated first point, there are not white point, else it means vehicle is on the lane.
*/
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>

using namespace cv;
using namespace std;

static int threshBinary = 150;//80;
//
// Strip is 20 pixel
//
// Tire pixes may be from pixel 17
int FindPoints(cv::Mat & source)
{
  int iWhiteThreshold = 200;
  int iBlackThreshold = 20;

  int iMaxTh = 40; // White strip max pixels
  int iMinTh = 15; // White strip min pixels

  int   iCurrVal;
  int   iPrevVal = -1;
  int   iNextVal;
  int   iSameCnt = 0;

  int i = source.rows / 2;
  int j = -1; // start from 17th pixels

  for (j=17; j<source.cols; ++j) {

    iCurrVal = source.at<unsigned char>(i,j);

    printf("val at %d, %d: %d\n", i, j, iCurrVal);

    if (iCurrVal < iBlackThreshold) {
      iCurrVal = iBlackThreshold;
    } else {
      iCurrVal = iWhiteThreshold;
    }

    if (iCurrVal == iPrevVal) {
      iSameCnt++; 
      continue;
    }

    printf("Color: %d (~0 => Black), SameCnt:%d\n", iPrevVal, iSameCnt);
    
    if (iPrevVal == iWhiteThreshold) {
      if ((iSameCnt < iMaxTh)
          && (iSameCnt > iMinTh)
         ) {
          return (j - iSameCnt); 

      }
    }

    iSameCnt = 0; 
    iPrevVal = iCurrVal;

    //if (iCurrVal == iBlackThreshold) {
    //  printf("Black at %d, %d\n", i, j);
    //  getchar();
    //} else {
    //  printf("White at %d, %d\n", i, j);
    //  getchar();
    //}

  }

//  for (int i=0; i<source.cols; ++i) {
//    for (int j=0; j<source.rows; ++j) {
//      if (source.at<unsigned char>(i,j) > threshold) {
//        temp_var = j;
//        for (; j<source.rows; ++j)
//          if (source.at<unsigned char>(i,j) > threshold) {
//
//          }
//            //output.push_back( (j-temp_var)/DPI ); // Results are stored in Inch
//      }
//    }
//  }
//
  return j; 
}

void DistanceToLane_Main(std::string strImgSrc, cv::Mat & image)
{
  int dDist;
  int iRoiX = 355;
  int iRoiY = 859;
  int iRoiW = 331;
  int iRoiH = 70;
  // Select the ROI
#if 1
  Rect2d roi = selectROI("Image", image, true, false);
  printf("ROI: %f %f %f %f\n", roi.x, roi.y, roi.width, roi.height);
  iRoiX = roi.x;
  iRoiY = roi.y;
  iRoiW = roi.width;
  iRoiH = roi.height;

#else
  Rect2d roi(iRoiX, iRoiY, iRoiW, iRoiH);// x, y, w, h
  rectangle(image, roi, Scalar(255, 0, 0), 2, 1); // draw the tracked object
#endif

	// Crop image
	cv::Mat gray;
	cv::Mat bw, blurMat, OutMat, sobelMat;

	Mat imCrop = image(roi);

  //imshow("Cr", imCrop);

	cv::cvtColor(imCrop, gray, CV_BGR2GRAY);
	cv::threshold(gray, OutMat, threshBinary, 255, THRESH_BINARY);

	imshow("Thresold", OutMat);

  dDist = FindPoints(OutMat);
  
  int y = iRoiY + (iRoiH / 2);//(OutMat.rows / 2;
  int xStart = iRoiX + 17;
  int xEnd = iRoiX + dDist;

  Point pt1(xStart, y);
  Point pt2(xEnd, y);

  printf("IMG: w=%d, h=%d, xS:%d, xE:%d, y:%d\n", image.cols, image.rows, xStart, xEnd, y);
  line(image, pt1, pt2, /*const Scalar&*/ Scalar(0xFF, 0x0, 0x0), /*thickness */ 2);
  
  imshow("Image", image);
  //getchar();
}

// Step 1: Consider the ROI
// Step 2: Consider the vertical center point (x=0, Ycenter)
// Step 3: Scan the row from the (x from 0 to end/white, Ycenter)
// Step 4: Starting point: After calibration, this can also be fixed OR Note the first white it is starting point
// Step 5: End point: Note the second white it is ending point. OR Not that after calibrated first point, there are not white point, else it means vehicle is on the lane.
// see: https://stackoverflow.com/questions/19954975/find-distance-between-two-lines-opencv
// http://answers.opencv.org/question/74400/calculate-the-distance-pixel-between-the-two-edges-lines/


