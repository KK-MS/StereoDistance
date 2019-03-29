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


static int threshCanny = 100, N = 5;
static int threshBinary = 80;



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

static void FindRectangles(const Mat& image, vector<vector<Point> >& squares)
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
	return;


	// find contours and store them all as a list
	findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	vector<Point> approx;

	// test each contour
	for (size_t i = 0; i < contours.size(); i++) {
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

static void FindLineSegments(const Mat& image, vector<vector<Point> >& squares)
{
 
  Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_NONE);//(LSD_REFINE_STD);
  //Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);

  double start = double(getTickCount());
  vector<Vec4f> lines_std;

	//Canny(image, image, 5, threshCanny, 5);
	//medianBlur(image, image, 7); // blur will enhance edge detection

  double start_lsd = double(getTickCount());
  // Detect the lines
  ls->detect(image, lines_std);

  double duration_ms = (double(getTickCount()) - start) * 1000 / getTickFrequency();
  double duration_lsd_ms = (double(getTickCount()) - start_lsd) * 1000 / getTickFrequency();
  std::cout << "It took lsd: " << duration_lsd_ms  << " Total: "<< duration_ms << " ms." << std::endl;

  // Show found lines
  Mat drawnLines(image);

  ls->drawSegments(drawnLines, lines_std);

  imshow("Standard refinement", drawnLines);
}

void select_roi(Mat &frame) {
	int rows = frame.rows;
	int cols = frame.cols;
	int type = frame.type();

	Point points[1][4];
	points[0][0] = Point(cols*0.00, rows * 0.70);  //LEFT Bottom (right side steering)
	points[0][1] = Point(cols*0.4, rows*0.4); // LEFT TOP
	points[0][2] = Point(cols*0.6, rows*0.4); // RIGHT TOP
	points[0][3] = Point(cols*0.95, rows * 0.90);;   // RIGHT TOP

	//Mat img(Size(frame.rows, frame.cols)); // = empty_image(frame); //User defined function that returns empty image of frame dimensions
	
	//Mat img = Mat::ones(frame.size(), frame.type); //(Size(frame.rows, frame.cols));
	Mat img = Mat::ones(frame.size(), frame.type());
	
	const Point* ppt[1] = { points[0] };

	int npt[] = { 4 };
	
	fillPoly(img, ppt, npt, 1, Scalar(255, 0, 0), 8);
	
	Mat res;
	//bitwise_and(frame, img, res);  //   imshow("AND", res);
	bitwise_and(frame, img, frame);  //   imshow("AND", res);

	//imshow("Poly", img);
}

int xValue = 3, yValue = 3;
int HT = 50;
static void FindLinesByThreshold(Mat& matInputImage)
{
	Mat OutMat, imgBlur, MatGray;
	Mat cdst, cdstP;



	createTrackbar("xV", "D1", &xValue, 31, NULL);
	createTrackbar("yV", "D1", &yValue, 31, NULL);
	createTrackbar("HT", "D1", &HT, 200, NULL);

	cv::cvtColor(matInputImage, MatGray, CV_BGR2GRAY);
	
	// Copy edges to the images that will display the results in BGR
	cvtColor(MatGray, cdst, COLOR_GRAY2BGR);
	cdstP = cdst.clone();

	cv::threshold(MatGray, OutMat, threshBinary, 255, THRESH_BINARY);
	imshow("D2", OutMat);
	
	int kernel_size = 5;
	
	GaussianBlur(OutMat, imgBlur, Size(xValue, yValue), 0, 0);
	//imshow("D3", imgBlur);

	Canny(imgBlur, MatGray, 5, threshCanny, 5);
	Canny(imgBlur, MatGray, 5, threshCanny, 5);

	imshow("D3", MatGray);

	select_roi(MatGray);
	
	imshow("D4", MatGray);

	// Probabilistic Line Transform
	vector<Vec4i> linesP; // will hold the results of the detection
	HoughLinesP(MatGray, linesP, 1, CV_PI / 180, HT, 50, 10); // runs the actual detection

	// Draw the lines
	for (size_t i = 0; i < linesP.size(); i++)
	{
		Vec4i l = linesP[i];
		line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
	}

	imshow("cdstP", cdstP);

	//FindLineSegments(OutMat, squares);
}


void Lines(std::string strImageSrc, Mat& image)
{
	//Mat image1;
	//ector<vector<Point> > squares;

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


	//cv::cvtColor(image, image1, CV_BGR2GRAY);

	createTrackbar("T1", "D1", &threshCanny, 255, NULL);
	createTrackbar("T2", "D1", &threshBinary, 255, NULL);

	double dStartTime = double(getTickCount());
	/////////////////////////////////////////////////////////

	//FindLineSegments(image1, squares);
	FindLinesByThreshold(image);
	//drawSquares(image, squares);


	/////////////////////////////////////////////////////////
	double dDuration_ms = (double(getTickCount()) - dStartTime) * 1000 / getTickFrequency();
	std::cout << "Time: " << dDuration_ms << " ms." << std::endl;


	imshow("D9", image);

}

// Reference:
// https://medium.com/@galen.ballew/opencv-lanedetection-419361364fc0
