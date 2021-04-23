#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

#ifndef IFINGERTIP_DETECTION_H
#define IFINGERTIP_DETECTION_H

class IFingertipDetection
{
public:
	virtual std::vector<Point> GetFingertips(Mat) = 0;
};

#endif