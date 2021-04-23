#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "common.hpp"

using namespace std;
using namespace cv;

#define GH	cout << "Got here ... " << __LINE__ << endl

#define PRINT_SKIN_MASK		0
#define PRINT_CONTOURS		1
#define PRINT_OUT_HULL		2
#define PRINT_INN_HULL		4
#define PRINT_HAND_MOMES	8
#define PRINT_LEFTFINGERS	16
#define PRINT_LEFTHANDHULL	32
#define PRINT_RIGHTHANDHULL	64
#define PRINT_LEFTMOME		128
#define PRINT_RIGHTMOME		256
#define PRINT_FOCUSEDROI	512
#define PRINT_RIGHTFINGERS	1024
#define PRINT_OPTFLOW		2048

#define ANALYSIS_HIST		1
#define ANALYSIS_RANGE		2

typedef struct hand_jp {

}Hand;

class Analysis {
private:

	/* */
	int width, height;
	Size size;
	int frame_count;

	/* Hands represent all the structures and 
		data that defines a hand including points 
		of the fingers. */
	Hand lefthand;
	Hand righthand;

	/* Internal strict types */
	enum ColorConversionCodes color_type;

	/* Mat objects */
	vector<Mat> frames;
	Mat last_frame;
	Mat skin_mask;
	Mat last_skin_mask;
	Mat print_frame;
	MatND hist;
	Rect focused_roi;
	Point focused_roi_center;

	/* Saved structures & vectors*/
	vector<Mat> hist_images;

	// Contours for the current image
	vector<Vec4i> hierarchy;
	vector< vector<Point> > contours;

	// Indecies for the current image
	vector< vector<int> > hull_indices;
	vector< vector<Point> > hull_points;
	vector< vector<int> > left_hull_indices;
	vector< vector<Point> > left_hand_hull;
	vector< vector<int> > right_hull_indices;
	vector< vector<Point> > right_hand_hull;

	//
	vector<Point2f> mome_points;
	vector<Point2f> left_mome_points;
	vector<Point2f> right_mome_points;

	//
	vector<Point> left_finger_locations;
	vector<Point> right_finger_locations;

	vector<Point2f> fing_flow_points[2];



	//
	int createSkinMaskFromRange(Mat frame);

	// 
	int generateFocusedROI();

	//
	int createSkinMaskFromHist(Mat frame, int probability_thresh, int thresh_type);

	//
	int createContours();

	//
	int createConvexHull();

	//
	int createHullCenterPoints();

	/* Separates the hands and combines minor hulls 
		to a single large hull for each hand. */
	int createHandHulls();

	//
	int estimateFingersLocations();

	// 
	int estimateOpticalFlow(Mat frame, int update_count);






public:
	/* Constructor for Finger tip analsys. */
	Analysis();
	Analysis(Mat frame);
	Analysis(Mat frame, enum ColorConversionCodes color_type);

	// Call repeatedly to load a vector of images to be used in the hue histogram
	int loadHistImage(string filename);

	// Generates the histogram 
	int generateHistogram();

	// Updates the analsysis
	int analyze(Mat frame, int flags);

	/* Returns an image to show analysis results */
	Mat getPrintFrame(int flags);

	/* Destructor */
	~Analysis();
};