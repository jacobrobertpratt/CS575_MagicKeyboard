#include "ISkinMask.h"

using namespace cv;

#ifndef SKIN_MASK_BASIC_H
#define SKIN_MASK_BASIC_H

class SkinMaskBasic : public ISkinMask
{
public:
	SkinMaskBasic();
	Mat GetSkinMask(Mat);
};

#endif
