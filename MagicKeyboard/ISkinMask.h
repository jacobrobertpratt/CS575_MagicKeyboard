#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

#ifndef ISKIN_MASK_H
#define ISKIN_MASK_H

class ISkinMask
{
public:
	virtual Mat GetSkinMask(Mat) = 0;
};

#endif