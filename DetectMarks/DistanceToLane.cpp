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

  int iCamCalXStart = 15;
  int   iCurrVal;
  int   iPrevVal = -1;
  int   iNextVal;
  int   iSameCnt = 0;

  int i = source.rows / 2;
  int j = -1; // start from nth pixels

  for (j= iCamCalXStart; j<source.cols; ++j) {

    iCurrVal = source.at<unsigned char>(i,j);

    //printf("val at %d, %d: %d\n", i, j, iCurrVal);

    if (iCurrVal < iBlackThreshold) {
      iCurrVal = iBlackThreshold;
    } else {
      iCurrVal = iWhiteThreshold;
    }

    if (iCurrVal == iPrevVal) { iSameCnt++; continue; }

    printf("Color: %d ( <150: Black), SameCnt:%d, j:%d\n", iPrevVal, iSameCnt, j);

    if (iPrevVal == iWhiteThreshold) {
      if ((iSameCnt < iMaxTh) && (iSameCnt > iMinTh)) {
        return (j - iSameCnt);
      }
    }

    iSameCnt = 0; 
    iPrevVal = iCurrVal;
  }

  return -1; 
}

void DistanceToLane_Main(std::string strImgSrc, cv::Mat & image)
{
  int dDistX;
  double dDist;
  double iDistPixel;

  int iRoiX = 1115;
  int iRoiY = 293;
  int iRoiW = 397;
  int iRoiH = 56;
  // Select the ROI
#if 0
  Rect2d roi = selectROI("Image", image, true, false);
  printf("ROI: %f %f %f %f\n", roi.x, roi.y, roi.width, roi.height);
  iRoiX = roi.x;
  iRoiY = roi.y;
  iRoiW = roi.width;
  iRoiH = roi.height;
  // ROI: 1115.000000 293.000000 397.000000 56.000000
#else
  Rect2d roi(iRoiX, iRoiY, iRoiW, iRoiH);// x, y, w, h
  //rectangle(image, roi, Scalar(255, 0, 0), 2, 1); // draw the tracked object
#endif

  // Crop image
  cv::Mat gray;
  cv::Mat bw, blurMat, OutMat, sobelMat;

  Mat imCrop = image(roi);

  cv::cvtColor(imCrop, gray, CV_BGR2GRAY);
  cv::threshold(gray, OutMat, threshBinary, 255, THRESH_BINARY);
  namedWindow("Thresold", WINDOW_NORMAL); resizeWindow("Thresold", Size(640, 480));
  imshow("Thresold", OutMat);

  dDistX = FindPoints(OutMat);
  if (dDistX > 0) {
    int y = iRoiY + (iRoiH / 2);//(OutMat.rows / 2;
    int xStart = iRoiX + 10;
    int xEnd = iRoiX + dDistX;

    Point pt1(xStart, y);
    Point pt2(xEnd, y);

    // NOTE: 
    iDistPixel = 1.0; // 1 pixel = 1 cm.
    dDistX = iDistPixel * dDistX;

    dDist = cv::sqrt(dDistX * dDistX); // Y = 0// diff.x + diff.y*diff.y);

    Scalar color = Scalar(0x00, 0xF0, 0x0);

    printf("IMG: dist:%f %f dDistX:%d w=%d, h=%d, xS:%d, xE:%d, y:%d\n", 
        dDist, cv::norm(pt1 - pt2), dDistX, image.cols, image.rows, xStart, xEnd, y);

    if (dDist < 80) {
      int intensity = (80 - dDist);
      color = Scalar(0x00, 0x00, 0xFF - 80 + intensity);
    }
    else if (dDist > 180) {
      int intensity = (dDist - 180);
      if (intensity > 80) intensity = 80;
      color = Scalar(0x00, 0x00, 0xFF - 80 + intensity);
    }

    line(image, pt1, pt2, color, 3);
  }

  imshow("Image", image);
  getchar();
}

// see: 
// https://stackoverflow.com/questions/19954975/find-distance-between-two-lines-opencv
// http://answers.opencv.org/question/74400/calculate-the-distance-pixel-between-the-two-edges-lines/

