#include "DetectionConvex.h"

std::vector<Point> DetectionConvex::GetFingertips(Mat skinMask) {
	// Contours
	std::vector<std::vector<Point> > contours;
	std::vector<Vec4i> hierarchy;
	findContours(skinMask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	//drawContours(img, contours, -1, Scalar(255, 255, 0), 2); // No longer have the original image

	// Convex Hull
	std::vector<std::vector<Point>> hull(contours.size());
	for (size_t i = 0; i < contours.size(); i++)
		convexHull(contours[i], hull[i]);

	//drawContours(img, hull, -1, Scalar(0, 255, 255), 2); // No longer have the original image

	//namedWindow("Hull", WINDOW_AUTOSIZE); // No longer have the original image
	//imshow("Hull", img);

	// TODO - must get all the fingertip locations
	std::vector<Point> fingertipLocations;

	// Convex Defects
	std::vector<std::vector<int>> hullIdx(contours.size());
	std::vector<std::vector<Vec4i>> convexDefects(contours.size());
	for (size_t i = 0; i < contours.size(); i++)
	{
		convexHull(contours[i], hullIdx[i], false);
		convexityDefects(contours[i], hullIdx[i], convexDefects[i]);

		for (size_t k = 0; k < convexDefects[i].size(); k++)
		{
			int ind_0 = convexDefects[i][k][0];
			int ind_1 = convexDefects[i][k][1];
			int ind_2 = convexDefects[i][k][2];

			// No longer have the original image
			//circle(img, contours[i][ind_0], 5, Scalar(255, 0, 0), -1);
			//circle(img, contours[i][ind_1], 5, Scalar(0, 255, 0), -1);
			//circle(img, contours[i][ind_2], 5, Scalar(0, 0, 255), -1);
			//line(img, contours[i][ind_2], contours[i][ind_0], Scalar(0, 0, 255), 1);
			//line(img, contours[i][ind_2], contours[i][ind_1], Scalar(0, 0, 255), 1);
		}
	}

	return fingertipLocations;
}