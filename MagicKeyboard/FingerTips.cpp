#include "FingerTips.h"

static bool compareContourArcLength(vector<Point> contour1, vector<Point> contour2) {
    double i = fabs(arcLength(contour1, true));
    double j = fabs(arcLength(contour2, true));
    return (i > j);
}


FingerTips::FingerTips() {

}


vector<Point> FingerTips::getFingerLocations() {

}

void FingerTips::update(Mat frame) {
    if (frame.empty()) {
        mk_error(ERROR_NULL);
        return;
    }


}