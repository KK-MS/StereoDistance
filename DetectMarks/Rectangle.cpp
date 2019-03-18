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
#include <string.h>

using namespace cv;
using namespace std;

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


int threshCanny = 50, N = 5;
const char* wndname = "Square Detection Demo";

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

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findSquares(const Mat& image, vector<vector<Point> >& squares)
{
	squares.clear();

	//s    Mat pyr, timg, gray0(image.size(), CV_8U), gray;

	// down-scale and upscale the image to filter out the noise
	//pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
	//pyrUp(pyr, timg, image.size());


	// blur will enhance edge detection
	//Mat timg(image);
	//medianBlur(image, timg, 9);
	//Mat gray0(timg.size(), CV_8U);
	Mat	gray0;
	Mat	gray;

	vector<vector<Point> > contours;

	cv::cvtColor(image, gray0, CV_BGR2GRAY); // aa added
		imshow("D1", gray0);

	// find squares in every color plane of the image
	//for (int c = 0; c < 3; c++)
	for (int c = 0; c < 1; c++)
	{
		//int ch[] = { c, 0 };
		//mixChannels(&timg, 1, &gray0, 1, ch, 1);

		// try several threshold levels
		for (int l = 0; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if (l == 0)
			{
				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				Canny(gray0, gray, 5, threshCanny, 5);
				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, Mat(), Point(-1, -1));
			}
			else
			{
				// apply threshold if l!=0:
				//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
				gray = gray0 >= (l + 1) * 255 / N;
			}

			// find contours and store them all as a list
			findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
			
			imshow("D2", gray);

			vector<Point> approx;

			// test each contour
			for (size_t i = 0; i < contours.size(); i++)
			{
				// approximate contour with accuracy proportional
				// to the contour perimeter
				approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

				// square contours should have 4 vertices after approximation
				// relatively large area (to filter out noisy contours)
				// and be convex.
				// Note: absolute value of an area is used because
				// area may be positive or negative - in accordance with the
				// contour orientation
				if (approx.size() == 4 &&
					fabs(contourArea(Mat(approx))) > 1000 &&
					isContourConvex(Mat(approx)))
				{
					double maxCosine = 0;

					for (int j = 2; j < 5; j++)
					{
						// find the maximum cosine of the angle between joint edges
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
	}
}


// the function draws all the squares in the image
static void drawSquares(Mat& image, const vector<vector<Point> >& squares)
{
	printf("Number of squares:%d\n ", squares.size());

	for (size_t i = 0; i < squares.size(); i++)
	{
		const Point* p = &squares[i][0];

		int n = (int)squares[i].size();
		//dont detect the border
		if (p->x > 3 && p->y > 3)
			polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, LINE_AA);
	}

	imshow(wndname, image);
}


//int mainRectangle(int argc, char** argv)
int main(int argc, char** argv)
{
	static const char* names[] = { ".\\pics\\atldata\\\\0000009550.png", "manyStickies.jpg",0 };

	namedWindow("D1", WINDOW_NORMAL);
	namedWindow("D2", WINDOW_NORMAL);
	resizeWindow("D1", 600, 600);
	resizeWindow("D2", 600, 600);

	help();
#if 1
	CommandLineParser parser(argc, argv, "{@input_path |0|input path can be a camera id, like 0,1,2 or a video filename}");

	parser.printMessage();
	string input_path = parser.get<string>(0);
	string video_name = input_path;
	VideoCapture video_in;

	if ((isdigit(input_path[0]) && input_path.size() == 1))
	{
		int camera_no = input_path[0] - '0';
		video_in.open(camera_no);
	}
	else {
		video_in.open(video_name);
	}
	if (!video_in.isOpened()) {
		cerr << "Couldn't open " << video_name << endl;
		return 1;
	}
#endif
	namedWindow(wndname, 1);
	vector<vector<Point> > squares;

#if 0 // Video image
	for (;;)
	{
		Mat image;

		video_in >> image;

		if (image.empty())
		{
			cout << "Couldn't capture image" << endl;
			continue;
		}

		findSquares(image, squares);
		
		drawSquares(image, squares);

		//imwrite( "out", image );
		if (waitKey(1) == 27) break; //quit on ESC button

	}
#endif 
#if 0
	for (int i = 0; names[i] != 0; i++)
	{
		Mat image;
		image = imread(names[i], 1);
		printf("Image: ch%d, type: %d\n", image.channels(), image.type());

		if (image.empty())
		{
			cout << "Couldn't load " << names[i] << endl;
			continue;
		}

		findSquares(image, squares);
		drawSquares(image, squares);
		//imwrite( "out", image );
		int c = waitKey();
		if ((char)c == 27)
			break;
	}

#endif // still image
	createTrackbar("T1", "D1", &threshCanny, 255, NULL);
#if 1
	while(1)
	{
		Mat image;
		image = imread(names[0], 1);
		printf("Image: ch%d, type: %d\n", image.channels(), image.type());

		if (image.empty())
		{
			cout << "Couldn't load " << names[0] << endl;
			continue;
		}

		findSquares(image, squares);
		drawSquares(image, squares);

		//imwrite( "out", image );
		int c = waitKey(10);
		if ((char)c == 27)
			break;
	}

#endif // still image

	return 0;
}
