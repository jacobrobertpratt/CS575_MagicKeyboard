#include "DetectionRandom.h"
#include <time.h>

std::vector<Point> DetectionRandom::GetFingertips(Mat skinMask)
{
	srand(time(NULL));

	int x = rand() % skinMask.cols;
	int y = rand() % skinMask.rows;

	std::vector<Point> points;
	points.push_back(Point(x, y));

	return points;
}