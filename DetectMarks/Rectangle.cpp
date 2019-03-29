// rectangledetect.cpp : Defines the entry point for the console application.
//


// The "Square Detector" program.
// It loads several images sequentially and tries to find squares in
// each image

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string>

using namespace cv;
using namespace std;


int threshCanny = 100, N = 5;
const char* wndname = "Square Detection Demo";


static void help()
{
  cout <<
    "\nA program using pyramid scaling, Canny, contours, contour simpification and\n"
    "memory storage to find squares in a list of images\n"
    "Returns sequence of squares detected on the image.\n"
    "the sequence is stored in the specified memory storage\n"
    "Call:\n"
    "./executable parameters\n"
    "Using OpenCV version %s\n" << CV_VERSION << "\n" << endl;
}


// the function draws all the squares in the image
static void drawSquares(Mat& image, const vector<vector<Point> >& squares)
{

	for (size_t i = 0; i < squares.size(); i++) {
		const Point* p = &squares[i][0];

		int n = (int)squares[i].size();
		//dont detect the border
		if (p->x > 3 && p->y > 3)
			polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, LINE_AA);
	}

	//imshow("D0", image);
}

void FindRectangles_Init()
{

}

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(Point pt1, Point pt2, Point pt0)
{
  double dx1 = pt1.x - pt0.x;
  double dy1 = pt1.y - pt0.y;
  double dx2 = pt2.x - pt0.x;
  double dy2 = pt2.y - pt0.y;
  return (dx1*dx2 + dy1 * dy2) / sqrt((dx1*dx1 + dy1 * dy1)*(dx2*dx2 + dy2 * dy2) + 1e-10);
}

// Returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
//
// Design: 
//  A program using pyramid scaling, Canny, contours, contour simpification and\n"
//  memory storage to find squares in a list of images\n"
//  Returns sequence of squares detected on the image.\n"
//  the sequence is stored in the specified memory storage\n"
//
// Code flow:
//  1. Clear the output buffer, squares.clear(); 
//
  
static void FindRectangles_CannyApprox(const Mat& image, vector<vector<Point> >& squares)
{
  Mat gray0;
  Mat gray;
  vector<vector<Point> > contours;

  imshow("D0", image);

  squares.clear();

  //medianBlur(image, image, 7); // blur will enhance edge detection
  //imshow("D1", image);

  // apply Canny. Take the upper threshold from slider
  // and set the lower to 0 (which forces edges merging)

  //Canny(image, gray, 5, threshCanny, 5);
  //adaptiveThreshold(image, gray, 200, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, 0);
  //adaptiveThreshold(gray0, gray, 200, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, 0);

  Canny(image, gray0, 5, threshCanny, 5);
  imshow("D2", gray0);


  // dilate canny output to remove potential holes between edge segments
  dilate(gray0, gray, Mat(), Point(-1, -1));

  imshow("D3", gray);
  return ;


  // find contours and store them all as a list
  findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

  vector<Point> approx;

  // test each contour
  for (size_t i = 0; i < contours.size(); i++)   {
    // arcLength(): Calculates a contour perimeter or a curve length.
    // Parameters:
    // curve – Input vector of 2D points, stored in std::vector or Mat.
    // closed – Flag indicating whether the curve is closed or not.
    //
    // approxPolyDP():
    // Parameters:
    // 1. curve – Input vector of a 2D point stored in :
    // 2. approxCurve – Result of the approximation. The type should match the type of the input curve. 
    // 3. epsilon – Parameter specifying the approximation accuracy. This is 
    //    the maximum distance between the original curve and its approximation.
    // 4. closed – If true, the approximated curve is closed(its first and last vertices are connected).Otherwise, it is not closed.

    // Design: approximate contour with accuracy proportional
    //         to the contour perimeter
    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true) * 0.02, true);

    // Design: i. square contours should have 4 vertices after approximation.
    //        ii. relatively large area (to filter out noisy contours) and 
    //       iii. be convex.
    // Note: absolute value of an area is used because
    // area may be positive or negative - in accordance with the
    // contour orientation
    if (approx.size() == 4 &&
        fabs(contourArea(Mat(approx))) > 1000 &&
        isContourConvex(Mat(approx))) {

      double maxCosine = 0;

      for (int j = 2; j < 5; j++) {
        // find the maximum cosine of the angle between joint edges
        //  j=2: approx 2, 0, 1
        //  j=3: approx 3, 1, 2
        //  j=4: approx 0, 2, 3
        double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
        maxCosine = MAX(maxCosine, cosine);
      }

      // if cosines of all angles are small
      // (all angles are ~90 degree) then write quandrange
      // vertices to resultant sequence
      if (maxCosine < 0.3)
        squares.push_back(approx);
    }
  }
}



void FindRectangles(std::string strImageSrc, Mat& image)
{
  Mat image1;
  vector<vector<Point> > squares;

  // namedWindow("D0", WINDOW_NORMAL);// resizeWindow("D0", Size(640, 480));
  // namedWindow("D1", WINDOW_NORMAL);// resizeWindow("D1", Size(640, 480));
  // namedWindow("D2", WINDOW_NORMAL);// resizeWindow("D2", Size(640, 480));
  // namedWindow("D3", WINDOW_NORMAL);// resizeWindow("D3", Size(640, 480));
  // namedWindow("D4", WINDOW_NORMAL);// resizeWindow("D4", Size(640, 480));
  // namedWindow("D9", WINDOW_NORMAL);// resizeWindow("D9", Size(640, 480));

  namedWindow("D0", WINDOW_AUTOSIZE);// resizeWindow("D0", Size(640, 480));
  namedWindow("D1", WINDOW_AUTOSIZE);// resizeWindow("D1", Size(640, 480));
  namedWindow("D2", WINDOW_AUTOSIZE);// resizeWindow("D2", Size(640, 480));
  namedWindow("D3", WINDOW_AUTOSIZE);// resizeWindow("D3", Size(640, 480));
  namedWindow("D4", WINDOW_AUTOSIZE);// resizeWindow("D4", Size(640, 480));
  namedWindow("D9", WINDOW_AUTOSIZE);// resizeWindow("D9", Size(640, 480));


  cv::cvtColor(image, image1, CV_BGR2GRAY);

  createTrackbar("T1", "D1", &threshCanny, 255, NULL);
  
  FindRectangles_CannyApprox(image1, squares);
  drawSquares(image, squares);
  imshow("D9", image);

  if (squares.size() > 0) {
    printf("R %d in %s\n", squares.size(), strImageSrc.c_str());
  }
}
