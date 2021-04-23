#pragma once

#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "common.hpp"

using namespace std;
using namespace cv;

typedef struct hand_jp {

}Hand;

class FingerTips {
private:
	vector< vector<Point> > last_locations;
	vector<Point2f> mome_pnts;
	vector< vector<Point> > contour;
public:
	/* Constructor for Finger tip analsys. */
	FingerTips();

	/* Returns a vector of Points which are the location of the analyzed finger tips */
	vector<Point> getFingerLocations();

	/* Updates the analsysis */
	void update(Mat frame);

	/* Destructor */
	~FingerTips();
};