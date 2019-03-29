//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/opencv.hpp"

#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
//#include "opencv2/ximgproc/disparity_filter.hpp"
#include <opencv2/ximgproc/disparity_filter.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/core/utility.hpp"


#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;


Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
static RNG rng(12345);

/// Function header
void thresh_callback(int, void*);

/** @function main */
int main1(int argc, char** argv)
{
	/// Load source image and convert it to gray
	//src = imread(argv[1], 1);
#if 0
	Mat frame;
	Mat edges;

	VideoCapture   hVLeft;
	int iVideoIDLeft = 0; // Camera ID 0

						  // Open the left side video input
	hVLeft.open(iVideoIDLeft);
	if (!hVLeft.isOpened()) {
		printf("Error: Couldn't open video:%d\n", iVideoIDLeft); return -1;
	}

	// get the initial frame to know the camera frame values
	hVLeft.read(frame);
#endif

	//src = imread(".\\TS01angle30.jpg");//(".\\TS01.jpg", 1);
	src = imread(".\\pics\\cir-ts-01.jpg");//(".\\TS01.jpg", 1);


										   /// Convert image to gray and blur it
	cvtColor(src, src_gray, CV_BGR2GRAY);
	//blur(src_gray, src_gray, Size(3, 3));
	blur(src_gray, src_gray, Size(1, 1));

	/// Create Window
	char source_window[] = "Source";
	namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	imshow(source_window, src);

	createTrackbar(" Threshold:", "Source", &thresh, max_thresh, thresh_callback);
	thresh_callback(0, 0);

	waitKey(0);
	return(0);
}

/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using Threshold
	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	namedWindow("findContours", CV_WINDOW_AUTOSIZE);
	imshow("findContours", threshold_output);

	/// Find the rotated rectangles and ellipses for each contour
	vector<RotatedRect> minRect(contours.size());
	vector<RotatedRect> minEllipse(contours.size());

	printf("contours.size():%d\n", contours.size());

	for (int i = 0; i < contours.size(); i++)
	{

		minRect[i] = minAreaRect(Mat(contours[i]));
		//if (contours[i].size() > 5)
		if (contours[i].size() > 50)
		{
			minEllipse[i] = fitEllipse(Mat(contours[i]));
		}

		printf(" W:%d, H:%d, angle:%d, \n", minEllipse[i].size.width, minEllipse[i].size.height, minEllipse[i].angle);
	}

	/// Draw contours + rotated rects + ellipses
	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		if (abs(minEllipse[i].angle) < 1)
		{
			continue;
		}

		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		// contour
		drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());

		// ellipse
		ellipse(drawing, minEllipse[i], color, 2, 8);

		// rotated rectangle
		Point2f rect_points[4]; minRect[i].points(rect_points);
		for (int j = 0; j < 4; j++)
			line(drawing, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
	}

	/// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", drawing);
}

// References
// https://docs.opencv.org/3.4/de/d62/tutorial_bounding_rotated_ellipses.html
// See
// https://stackoverflow.com/questions/28835644/ambiguity-in-ellipse-detection-in-opencv-c
// 
// Roated rect
// https://stackoverflow.com/questions/41898374/processing-cvrotatedrect-width-and-height
//
// Experiment
// https://github.com/lps683/TrafficsSignDetection

int main2()
{
	cv::String algo = "bm"; // stereo matching method (bm or sgbm)
	int max_disp = 21; //ui->horizontalSlider_num_of_disparity->value();
	int wsize = 20; //ui->horizontalSlider_SAD_window_size->value(); // Window size (SAD)

					//k int mode = ui->mode->itemData(ui->mode->currentIndex()).toInt(); // SGBM 3-way mode

					// post-filtering - Lambda is a parameter defining the amount of regularization during filtering.
					// Larger values force filtered disparity map edges to adhere more to source image edges. Typical value is 8000
	double lambda = 8000; //ui->doubleSpinBox_lambda->value();
						  // post-filtering - SigmaColor is a parameter defining how sensitive the filtering process is to source image edges.
						  // Large values can lead to disparity leakage through low-contrast edges.
						  // Small values can make the filter too sensitive to noise and textures in the source image.
						  // Typical values range from 0.8 to 2.0
	double sigma = 1.0; //ui->doubleSpinBox_sigma->value();

						// used post-filtering (wls_conf or wls_no_conf)
	cv::String filter = "wls_conf"; // "wls_conf"; else filter = "wls_no_conf";

	Mat left_image = imread(".\\pics\\sintel\\final_left\\alley_1\\frame_0001.png"); //(".\\pics\\stereo\\stereo04L.jpg");//(".\\TS01.jpg", 1);
	Mat right_image = imread(".\\pics\\sintel\\final_right\\alley_1\\frame_0001.png"); //(".\\pics\\stereo\\stereo04R.jpg");//(".\\TS01.jpg", 1);
																					   //pics\stereo
																					   // load_views
	cv::Mat left = left_image;
	cv::Mat right = right_image;

	cv::Mat left_for_matcher, right_for_matcher;
	cv::Mat left_disp, right_disp;
	cv::Mat filtered_disp;
	cv::Mat disp_color;

	cv::Mat conf_map = cv::Mat(left.rows, left.cols, CV_8U);

	conf_map = cv::Scalar(255);

	cv::Rect ROI;

	Ptr<cv::ximgproc::DisparityWLSFilter> wls_filter;


	if (!left_image.data) { printf("Could not open left image file!!\n"); getchar(); exit(-1); }
	if (!right_image.data) { printf("Could not open right image file!!\n"); getchar(); exit(-1); }

	//imshow("left image", left_image);
	//	waitKey(0);

	if (filter == "wls_conf") // filtering with confidence (significantly better quality than wls_no_conf)
	{
		left_for_matcher = left.clone();
		right_for_matcher = right.clone();

		if (algo == "bm")
		{
			Ptr<cv::StereoBM> left_matcher = cv::StereoBM::create(max_disp, wsize);
			left_matcher->setPreFilterSize(95); // must be an odd between 5 and 255
			left_matcher->setPreFilterCap(31);  // must be within 1 and 63
			left_matcher->setBlockSize(93);  // must be odd, be within 5..255 and be not larger than image width or height
			left_matcher->setMinDisparity(0); // normally at 0
			left_matcher->setNumDisparities(32);  // must be > 0 and divisible by 16
			left_matcher->setTextureThreshold(50);  // must be non-negative
			left_matcher->setUniquenessRatio(2);  // must be non-negative
			left_matcher->setSpeckleWindowSize(100); // 0 to disable, set to 50-200
			left_matcher->setSpeckleRange(1); // 1 or 2 is good, 0 to disable
			left_matcher->setDisp12MaxDiff(-1); // negative = disabled

			wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);
			Ptr<cv::StereoMatcher> right_matcher = cv::ximgproc::createRightMatcher(left_matcher);

			cv::cvtColor(left_for_matcher, left_for_matcher, cv::COLOR_BGR2GRAY); // Convert to gray, needed by bm function
			cv::cvtColor(right_for_matcher, right_for_matcher, cv::COLOR_BGR2GRAY);

			imshow("left_for_matcher", left_for_matcher);
			imshow("right_for_matcher", right_for_matcher);

			left_matcher->compute(left_for_matcher, right_for_matcher, left_disp);
			right_matcher->compute(right_for_matcher, left_for_matcher, right_disp);

			imshow("left_disp", left_disp);
			imshow("right_disp", right_disp);
#if 0
#endif
		}
		else if (algo == "sgbm")
		{
		}
#if 1
		// filtering
		wls_filter->setLambda(lambda);
		wls_filter->setSigmaColor(sigma);
		wls_filter->filter(left_disp, left, filtered_disp, right_disp);

		conf_map = wls_filter->getConfidenceMap();

		ROI = wls_filter->getROI(); // Get the ROI that was used in the last filter call
#endif
	}
	else if (filter == "wls_no_conf")
	{
		/* There is no convenience function for the case of filtering with no confidence, so we
		will need to set the ROI and matcher parameters manually */


	}

	imshow("confidence", conf_map);
	imshow("filtered_disp", filtered_disp);

#if 1
	// Visualization
	cv::Mat raw_disp_vis;
	cv::ximgproc::getDisparityVis(left_disp, raw_disp_vis, 1);

	Mat filtered_disp_vis;
	cv::ximgproc::getDisparityVis(filtered_disp, filtered_disp_vis, 1);
	imshow("filtered_disp_vis", filtered_disp_vis);
	//computed = true; // Indicate that a depthmap exists

	// convert the image to a QPixmap and display it
	cv::cvtColor(filtered_disp_vis, disp_color, CV_GRAY2RGB); // convert to color, better for saving the file

															  //ShowDepthmap();
	imshow("disp_color", disp_color);
#endif
	waitKey(0);
	//getchar();

	return 0;
}

int main3()
{
	cv::String algo = "bm"; // stereo matching method (bm or sgbm)
	int max_disp = 160; //ui->horizontalSlider_num_of_disparity->value();
	int wsize = 3; //20; //ui->horizontalSlider_SAD_window_size->value(); // Window size (SAD)

				   //k int mode = ui->mode->itemData(ui->mode->currentIndex()).toInt(); // SGBM 3-way mode

				   // post-filtering - Lambda is a parameter defining the amount of regularization during filtering.
				   // Larger values force filtered disparity map edges to adhere more to source image edges. Typical value is 8000
	double lambda = 8000.0; //ui->doubleSpinBox_lambda->value();
							// post-filtering - SigmaColor is a parameter defining how sensitive the filtering process is to source image edges.
							// Large values can lead to disparity leakage through low-contrast edges.
							// Small values can make the filter too sensitive to noise and textures in the source image.
							// Typical values range from 0.8 to 2.0
	double sigma = 1.5; //ui->doubleSpinBox_sigma->value();

						// used post-filtering (wls_conf or wls_no_conf)
	cv::String filter = "wls_conf"; // "wls_conf"; else filter = "wls_no_conf";

									//Mat left_image = imread(".\\pics\\sintel\\final_left\\alley_1\\frame_0001.png"); //(".\\pics\\stereo\\stereo04L.jpg");//(".\\TS01.jpg", 1);
									//Mat right_image = imread(".\\pics\\sintel\\final_right\\alley_1\\frame_0001.png"); //(".\\pics\\stereo\\stereo04R.jpg");//(".\\TS01.jpg", 1);
									//pics\stereo
									// load_views
	Mat left_image = imread(".\\pics\\stereo\\stereo07L.jpg");//(".\\TS01.jpg", 1);
	Mat right_image = imread(".\\pics\\stereo\\stereo07R.jpg");//(".\\TS01.jpg", 1);

	cv::Mat left = left_image;
	cv::Mat right = right_image;

	cv::Mat left_for_matcher, right_for_matcher;
	cv::Mat left_disp, right_disp;
	cv::Mat filtered_disp;
	cv::Mat disp_color;

	cv::Mat conf_map = cv::Mat(left.rows, left.cols, CV_8U);

	conf_map = cv::Scalar(255);

	cv::Rect ROI;

	Ptr<cv::ximgproc::DisparityWLSFilter> wls_filter;


	if (!left_image.data) { printf("Could not open left image file!!\n"); getchar(); exit(-1); }
	if (!right_image.data) { printf("Could not open right image file!!\n"); getchar(); exit(-1); }

	//imshow("left image", left_image);
	//	waitKey(0);

	//if (filter == "wls_conf") // filtering with confidence (significantly better quality than wls_no_conf)
	{
		left_for_matcher = left.clone();
		right_for_matcher = right.clone();

		//if (algo == "bm")
		{
			Ptr<cv::StereoBM> left_matcher = cv::StereoBM::create(max_disp, wsize);
#if 1
			//		left_matcher->setPreFilterSize(95); // must be an odd between 5 and 255
			//		left_matcher->setPreFilterCap(31);  // must be within 1 and 63
			left_matcher->setBlockSize(101);  // must be odd, be within 5..255 and be not larger than image width or height
											  //		left_matcher->setMinDisparity(0); // normally at 0
											  //		left_matcher->setNumDisparities(32);  // must be > 0 and divisible by 16
											  //		left_matcher->setTextureThreshold(50);  // must be non-negative
											  //		left_matcher->setUniquenessRatio(2);  // must be non-negative
											  //		left_matcher->setSpeckleWindowSize(100); // 0 to disable, set to 50-200
											  //		left_matcher->setSpeckleRange(1); // 1 or 2 is good, 0 to disable
											  //		left_matcher->setDisp12MaxDiff(-1); // negative = disabled
#endif
			wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);
			Ptr<cv::StereoMatcher> right_matcher = cv::ximgproc::createRightMatcher(left_matcher);

			cv::cvtColor(left_for_matcher, left_for_matcher, cv::COLOR_BGR2GRAY); // Convert to gray, needed by bm function
			cv::cvtColor(right_for_matcher, right_for_matcher, cv::COLOR_BGR2GRAY);

			imshow("left_for_matcher", left_for_matcher);
			imshow("right_for_matcher", right_for_matcher);

			left_matcher->compute(left_for_matcher, right_for_matcher, left_disp);
			right_matcher->compute(right_for_matcher, left_for_matcher, right_disp);

			imshow("left_disp", left_disp);
			imshow("right_disp", right_disp);
#if 0
#endif
		}
		//else if (algo == "sgbm")
		{
		}
#if 1
		// filtering
		wls_filter->setLambda(lambda);
		wls_filter->setSigmaColor(sigma);
		wls_filter->filter(left_disp, left, filtered_disp, right_disp);

		conf_map = wls_filter->getConfidenceMap();

		ROI = wls_filter->getROI(); // Get the ROI that was used in the last filter call
#endif
	}
	//else if (filter == "wls_no_conf")
	{
		/* There is no convenience function for the case of filtering with no confidence, so we
		will need to set the ROI and matcher parameters manually */


	}

	imshow("confidence", conf_map);
	imshow("filtered_disp", filtered_disp);

#if 1
	// Visualization
	cv::Mat raw_disp_vis;
	cv::ximgproc::getDisparityVis(left_disp, raw_disp_vis, 1);

	Mat filtered_disp_vis;
	cv::ximgproc::getDisparityVis(filtered_disp, filtered_disp_vis, 1);
	imshow("filtered_disp_vis", filtered_disp_vis);
	//computed = true; // Indicate that a depthmap exists

	// convert the image to a QPixmap and display it
	//cv::cvtColor(filtered_disp_vis, disp_color, CV_GRAY2RGB); // convert to color, better for saving the file
	//ShowDepthmap(); 	//imshow("disp_color", disp_color);
#endif
	waitKey(0);
	//getchar();

	return 0;
}

// reference
// https://github.com/opencv/opencv_contrib/blob/master/modules/ximgproc/samples/disparity_filtering.cpp
// Disparity
// https://docs.opencv.org/3.1.0/d3/d14/tutorial_ximgproc_disparity_filtering.html  ***

int main1()
{

	Mat left_image = imread(".\\pics\\cir-ts-03.jpg");//(".\\TS01.jpg", 1);

													  //Mat left_image = imread(".\\pics\\stereo\\stereo07L.jpg");//(".\\TS01.jpg", 1);
	Mat right_image = imread(".\\pics\\stereo\\stereo07R.jpg");//(".\\TS01.jpg", 1);




	if (left_image.empty()) { printf("Could not open left image file!!\n"); getchar(); exit(-1); }
	if (right_image.empty()) { printf("Could not open right image file!!\n"); getchar(); exit(-1); }

	Mat src_gray; // 1st gray
	Mat src_blur; // 2nd blur (see guassian?)
	Mat src_edge; // canny
	Mat src_Hough; // Hough

				   // Edge detection
				   // Blur image to reduce noise
	int iBlurKernelSize = 5;

	// Canny
	// 1. Fiter out any noise. Gaussian filter.
	// 2. Find the intensity gradient of the image. Analogous to Sobel. 
	//    Gradient strength G and direction Theta. 
	//    The direction Theta is rounded to four possible angles, 0, 45, 90 or 135
	int kernel_size = 3;   // the size of the Sobel kernel to be used internally
						   // 3. Non-maximum supression is applied to remove pixels that are not considered to be part of an edge.
						   //    Hence only thin lines (candidate edge) will remain.
						   // 4. Hysteresis: The final step. Canny does use two thresholds (upper and lower).
						   //    a. If a pixel gradient is higher than the upper threshold, the pixel is accepted as an edge
						   //    b. If a pixel gradient value is below the lower threshold, then it is rejected.
						   //    c. If the pixel gradient is between the two thresholds, then it will be accepted only if it is connected to a pixel that is above the upper threshold.
						   //    Canny recommended a upper : lower ratio between 2 : 1 and 3 : 1.
						   //    AA: Experment shows higher (3) is better in reducing noise and detecint traffic object.
	int lowThreshold = 50; // if pixel gradient is below the lower threshold, then it is rejected.
	int ratio = 3;         // if pixel gradient is above ratio x lowThreshold, then it is accepted.


						   // For control window
	int const max_lowThreshold = 100; // Control window. Max lowThreshold 

									  /// Create Window
	char source_window[] = "Source";
	char ProWin[] = "Process";
	char CtrWinName[] = "CtrlWin";

	namedWindow(source_window, CV_WINDOW_NORMAL); 	resizeWindow(source_window, 640, 480);
	namedWindow(ProWin, CV_WINDOW_NORMAL); 	resizeWindow(source_window, 640, 480);
	namedWindow(CtrWinName, CV_WINDOW_AUTOSIZE);

	/// Create a Trackbar for user to enter threshold
	createTrackbar("LowTh:", CtrWinName, &lowThreshold, max_lowThreshold);
	createTrackbar("Ratio:", CtrWinName, &ratio, 3);
	createTrackbar("BlurKer:", CtrWinName, &iBlurKernelSize, 15);




	while (1) {
		src = left_image.clone();
		// Notes:
		// Design can be such that 
		// Step-1: Set the threshold to higher value, ie. Canny (ratio=3, lowThreshold=100), GaussianBlur (Kernel 9x9, 
		/// Convert image to gray and blur it
		cvtColor(src, src_gray, CV_BGR2GRAY);

		if (!(iBlurKernelSize & 1))iBlurKernelSize++; // make sure it is kernel size is odd

													  // blur the image to reduce noise. Note. Canny also reduces noise by Gausiann.
													  //blur(src_gray, src_blur, Size(iBlurKernelSize, iBlurKernelSize));

													  /// Reduce the noise so we avoid false circle detection
													  // 3rd param: - Ksize The Gaussian kernel size; ksize.width and ksize.height can differ, 
													  //              but they both must be positive and odd. Or, they can be zero’s, 
													  //              then they are computed from sigma
													  // sigmaX     – Gaussian kernel standard deviation in X direction.
													  // sigmaY     – Gaussian kernel standard deviation in Y direction; if sigmaY is zero ... 
													  // borderType – pixel extrapolation method (see borderInterpolate() for details).
													  // 
													  // Note: borderType is set to default.
		GaussianBlur(src_gray, src_blur, Size(iBlurKernelSize, iBlurKernelSize), 2, 2); // Size(9, 9)

																						/// Canny detector
		Canny(src_blur, src_edge, lowThreshold, lowThreshold * ratio, kernel_size);

		vector<Vec3f> circles;

		/// Apply the Hough Transform to find the circles
		// 5th Param: Minimum distance between the centers of the detected circles
		// @param param1 First method - specific parameter.In case of #HOUGH_GRADIENT, it is the higher
		//	threshold of the two passed to the Canny edge detector(the lower one is twice smaller).
		//	@param param2 Second method - specific parameter.In case of #HOUGH_GRADIENT, it is the
		//	accumulator threshold for the circle centers at the detection stage.The smaller it is, the more
		//	false circles may be detected.Circles, corresponding to the larger accumulator values, will be
		//	returned first.
		//	@param minRadius Minimum circle radius.
		//	@param maxRadius Maximum circle radius.If <= 0, uses the maximum image dimension.If < 0, returns
		//	centers without finding the radius.
		//
		//	@sa fitEllipse, minEnclosingCircle
		//
		HoughCircles(src_edge, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows / 10, 200, 100, 30, 0);

		printf("Circles size:%d\n", circles.size());
		/// Draw the circles detected
		for (size_t i = 0; i < circles.size(); i++)
		{
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			printf("Circles[%d] C[%d,%d], R:%d\n", i, cvRound(circles[i][0]), cvRound(circles[i][1]), radius);

			// circle center
			circle(src, center, 3, Scalar(0, 255, 0), -1, 8, 0);

			// circle outline
			circle(src, center, radius, Scalar(0, 0, 255), 3, 8, 0);
		}



		/// Convert it to color
		//cvtColor(src_edge, src_gray, CV_BGR2GRAY);
		//imshow(source_window, src_edge);
		imshow(source_window, src);
		imshow(ProWin, src_edge);

		if (waitKey(10) == 27) {
			break;
		}
		//waitKey(0);
	}
	//getchar();

	return 0;
}