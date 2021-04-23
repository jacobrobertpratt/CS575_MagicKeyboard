#include "IFingertipDetection.h"

using namespace cv;

#ifndef DETECTION_CONVEX_H
#define DETECTION_CONVEX_H

class DetectionConvex : public IFingertipDetection {
private:

public:
	std::vector<Point> GetFingertips(Mat);
};

#endif