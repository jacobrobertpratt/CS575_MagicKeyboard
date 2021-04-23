#include "SkinMaskBasic.h"

SkinMaskBasic::SkinMaskBasic() {}

Mat SkinMaskBasic::GetSkinMask(Mat frame)
{
	// Image to HSV
	Mat imgHSV;
	cvtColor(frame, imgHSV, COLOR_BGR2HSV);

	// Lower and Upper HSV ranges
	int lowH = 0; int lowS = 48; int lowV = 70;
	int hghH = 50; int hghS = 175; int hghV = 255;

	// Threshold img with ranges
	Mat skinRegionHSV;
	inRange(imgHSV, Scalar(lowH, lowS, lowV), Scalar(hghH, hghS, hghV), skinRegionHSV);

	namedWindow("skinRegionHSV", WINDOW_AUTOSIZE);
	imshow("skinRegionHSV", skinRegionHSV);

	// Blur img
	Mat blurredHSV;
	blur(skinRegionHSV, blurredHSV, Point(2, 2));

	// Theshold
	Mat threshHSV;
	threshold(blurredHSV, threshHSV, 0, 255, THRESH_BINARY);

	return threshHSV;
}