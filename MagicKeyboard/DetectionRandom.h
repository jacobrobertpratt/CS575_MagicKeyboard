#include "IFingertipDetection.h"

using namespace cv;

#ifndef DETECTION_RNADOM_H
#define DETECTION_RANDOM_H

class DetectionRandom : public IFingertipDetection
{
public:
	std::vector<Point> GetFingertips(Mat);

	DetectionRandom() {}
};

#endif