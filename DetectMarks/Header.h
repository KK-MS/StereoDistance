#pragma once

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string>

using namespace std;

void FindRectangles(std::string strImageSrc, cv::Mat &image);
//void drawSquares(cv::Mat &image, const vector<vector<cv::Point> >& squares);

void FindShapes   ( std::string strImageSrc, cv::Mat& image);
void Lines        ( std::string strImageSrc, cv::Mat& image);
int FindDisparity ( std::string strImageSrc, cv::Mat& image);
void DistanceToLane_Main(std::string strImgSrc, cv::Mat & image);