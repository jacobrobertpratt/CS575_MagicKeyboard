#include "Analysis.h"

using namespace std;
using namespace cv;

static Mat se_ellipse_3_3 = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(0, 0));
static Mat se_ellipse_5_5 = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(0, 0));
static Mat se_rect_3_3 = getStructuringElement(MORPH_RECT, Size(3, 3), Point(0, 0));

Analysis::Analysis() {
	this->width = 0;
	this->height = 0;
	this->frame_count = 0;
	this->color_type = COLOR_BGR2YCrCb;
}

Analysis::Analysis(Mat frame) {
	this->width = frame.cols;
	this->height = frame.rows;
	this->frame_count = 0;
	this->size = Size(this->width, this->height);
	this->color_type = COLOR_BGR2YCrCb;
}

Analysis::Analysis(Mat frame, enum ColorConversionCodes color_type) {
	this->width = frame.cols;
	this->height = frame.rows;
	this->frame_count = 0;
	this->size = Size(this->width, this->height);
	this->color_type = color_type;
}

static bool compareContourArcLength(vector<Point> contour1, vector<Point> contour2) {
	double i = fabs(arcLength(contour1, true));
	double j = fabs(arcLength(contour2, true));
	return (i > j);
}

int Analysis::loadHistImage(string filename) {
	if (filename.empty())
		return ERROR_NULL;
	this->hist_images.push_back(imread(filename, IMREAD_COLOR));
	return 0;
}

int Analysis::generateHistogram() {

	if (this->hist_images.empty())
		return ERROR_INIT;

	int i;

	int ybins = 4, crbins = 16, cbbins = 16;
	int histSize[] = { ybins, crbins, cbbins };

	float yrange[] = { 0, 256 };
	float rrange[] = { 0, 256 };
	float brange[] = { 0, 256 };
	const float* ranges[] = { yrange, rrange, brange };

	int channels[] = { 0, 1, 2 };

	for (i = 0; i < this->hist_images.size(); i++) {
		cvtColor(this->hist_images[i], this->hist_images[i], this->color_type);
		calcHist(&(this->hist_images[i]), 1, channels, Mat(), this->hist, 2, histSize, ranges, true, true);
	}

	normalize(this->hist, this->hist, 0, 255, NORM_MINMAX, -1, Mat());

	this->hist_images.clear();
	return 0;
}

int Analysis::createSkinMaskFromHist(Mat frame, int probability_thresh, int thresh_type) {

	float yrange[] = { 0, 256 };
	float rrange[] = { 0, 256 };
	float brange[] = { 0, 256 };
	const float* ranges[] = { yrange, rrange, brange };

	int channels[] = { 0, 1, 2 };

	Mat tmp_frame;
	normalize(frame, tmp_frame, 0, 255, NORM_MINMAX, -1, Mat());
	calcBackProject(&tmp_frame, 1, channels, this->hist, this->skin_mask, ranges, 1, true);

	threshold(this->skin_mask, this->skin_mask, 25, 255, thresh_type);

	erode(this->skin_mask, this->skin_mask, se_ellipse_5_5);
	dilate(this->skin_mask, this->skin_mask, se_ellipse_5_5);

	return 0;
}

int Analysis::createSkinMaskFromRange(Mat frame) {

	// Get skin range
	inRange(frame, Scalar(0, 133, 77), Scalar(255, 173, 127), this->skin_mask);

	// Remove the little stuff
	erode(this->skin_mask, this->skin_mask, se_ellipse_5_5);
	dilate(this->skin_mask, this->skin_mask, se_ellipse_5_5);

	return 0;
}

int Analysis::generateFocusedROI() {

	if (this->left_hand_hull.empty() && this->right_hand_hull.empty())
		return ERROR_INIT;

	int i, j;
	int min_left_idx = this->width, min_top_idx = this->height;
	// Get left-upper indexs
	for (i = 0; i < this->left_hand_hull.size(); i++) {
		for (j = 0; j < this->left_hand_hull[i].size(); j++) {
			if (this->left_hand_hull[i][j].x < min_left_idx)
				min_left_idx = this->left_hand_hull[i][j].x;
			if (this->left_hand_hull[i][j].y < min_top_idx)
				min_top_idx = this->left_hand_hull[i][j].y;
		}
	}

	int max_right_idx = 0, max_bottom_idx = 0;
	for (i = 0; i < this->right_hand_hull.size(); i++) {
		for (j = 0; j < this->right_hand_hull[i].size(); j++) {
			if (this->right_hand_hull[i][j].x > max_right_idx)
				max_right_idx = this->right_hand_hull[i][j].x;
			if (this->right_hand_hull[i][j].y > max_bottom_idx)
				max_bottom_idx = this->right_hand_hull[i][j].y;
		}
	}

	// Get right-lower indexs
	min_top_idx -= 20;
	min_left_idx -= 20;
	if (min_top_idx < 0) min_top_idx = 0;
	if (min_left_idx < 0) min_left_idx = 0;
	max_right_idx += 20;
	max_bottom_idx += 20;
	if (max_bottom_idx > this->height) max_bottom_idx = this->height;
	if (max_right_idx > this->width) max_right_idx = this->width;

	this->focused_roi = Rect(Point(min_left_idx,min_top_idx), Point(max_right_idx,max_bottom_idx));

	return 0;
}

int Analysis::createContours() {
	int i;

	if (this->skin_mask.empty()) {
		mk_error(ERROR_INIT);
		return ERROR_INIT;
	}

	GaussianBlur(this->skin_mask, this->skin_mask, Size(5, 5), 3, 3);

	Mat y_can_msk;
	Canny(this->skin_mask, y_can_msk, 100, 200, 3);
	dilate(y_can_msk, y_can_msk, se_ellipse_3_3);

	findContours(y_can_msk, this->contours, this->hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	// Get the maximum area
	double min_length = 300;
	double sum = 0;
	vector< vector<Point> > tmp_contours;
	for (i = 2; i < this->contours.size(); i++) {
		double len = arcLength(this->contours[i], false);
		if (len > min_length) {
			tmp_contours.push_back(this->contours[i]);
		}
	}
	this->contours.clear();
	this->contours = vector< vector<Point> >(tmp_contours);

	// Improve the contours based on contour size
	for (i = 0; i < this->contours.size(); i++) {
		approxPolyDP(Mat(this->contours[i]), this->contours[i], 0.5, false);
	}

	return 0;
}

int Analysis::createConvexHull() {

	int i = 0;

	if (this->skin_mask.empty() || this->contours.empty()) {
		return ERROR_INIT;
	}

	// Regular Hull points
	this->hull_points.clear();
	this->hull_points = vector< vector<Point> >(this->contours.size());
	for (i = 0; i < this->contours.size(); i++)
		convexHull(this->contours[i], this->hull_points[i], true);          // true -> return points

	return 0;
}

int Analysis::createHullCenterPoints() {

	int i = 0;

	if (this->hull_points.empty()) {
		mk_error(ERROR_INIT);
		return ERROR_INIT;
	}

	//// Get center of contours
	this->mome_points.clear();
	this->mome_points = vector<Point2f>(this->hull_points.size());
	vector<Moments> momes(this->hull_points.size());
	for (i = 0; i < this->hull_points.size(); i++) {
		momes[i] = moments(this->hull_points[i], false);
		this->mome_points[i] = Point2f(momes[i].m10 / momes[i].m00, momes[i].m01 / momes[i].m00);
	}
	momes.clear();

	return 0;
}

int Analysis::createHandHulls() {

	int i = 0, j = 0;
	vector<Point> combined, result;
	vector<int> result_idxs;

	if ((this->mome_points.size() < 2) || (this->hull_points.empty())) {
		return ERROR_INIT;
	}

	left_hand_hull.clear();
	right_hand_hull.clear();

	// Separate the left and right hands into their own set of hulls
	for (i = 0; i < this->mome_points.size(); i++) {
		if (this->mome_points[i].x < this->width / 2)
			this->left_hand_hull.push_back(this->hull_points[i]);
		else
			this->right_hand_hull.push_back(this->hull_points[i]);
	}

	if (this->left_hand_hull.size() == 1)
		return 0;

	// Construct a single left-hand hull
	for (i = 0; i < this->left_hand_hull.size(); i++) {
		for (j = 0; j < this->left_hand_hull[i].size(); j++) {
			combined.push_back(this->left_hand_hull[i][j]);
		}
	}
	this->left_hand_hull.clear();
	if (!combined.empty()) {
		convexHull(combined, result, true);
		convexHull(combined, result_idxs, false);
		this->left_hand_hull.push_back(vector<Point>(result));
		this->left_hull_indices.push_back(vector<int>(result_idxs));
	}

	combined.clear();
	result.clear();
	result_idxs.clear();

	if (this->right_hand_hull.size() == 1)
		return 0;

	// Construct a single right-hand hull
	for (i = 0; i < this->right_hand_hull.size(); i++) {
		for (j = 0; j < this->right_hand_hull[i].size(); j++) {
			combined.push_back(this->right_hand_hull[i][j]);
		}
	}
	this->right_hand_hull.clear();
	if (!combined.empty()) {
		convexHull(combined, result, true);
		convexHull(combined, result_idxs, false);
		this->right_hand_hull.push_back(vector<Point>(result));
		this->right_hull_indices.push_back(vector<int>(result_idxs));
	}

	combined.clear();
	result.clear();
	result_idxs.clear();
	
	vector<Moments> momes;
	// Set the center of the left-hand based on left-hand convex hull
	if (!this->left_hand_hull.empty()) {
		this->left_mome_points.clear();
		this->left_mome_points = vector<Point2f>(this->left_hand_hull.size());
		momes = vector<Moments>(this->left_hand_hull.size());
		for (i = 0; i < this->left_hand_hull.size(); i++) {
			momes[i] = moments(this->left_hand_hull[i], false);
			this->left_mome_points[i] = Point2f(momes[i].m10 / momes[i].m00, momes[i].m01 / momes[i].m00);
		}
		momes.clear();
	}

	// Set the center of the right-hand based on right-hand convex hull
	if (!this->right_hand_hull.empty()) {
		this->right_mome_points.clear();
		this->right_mome_points = vector<Point2f>(this->right_hand_hull.size());
		momes = vector<Moments>(this->right_hand_hull.size());
		for (i = 0; i < this->right_hand_hull.size(); i++) {
			momes[i] = moments(this->right_hand_hull[i], false);
			this->right_mome_points[i] = Point2f(momes[i].m10 / momes[i].m00, momes[i].m01 / momes[i].m00);
		}
		momes.clear();
	}

	return 0;
}

int Analysis::estimateFingersLocations() {

	int i = 0, j = 0;
	
	if (this->focused_roi.empty() || (this->left_hand_hull.empty()) || (this->right_hand_hull.empty())) {
		return ERROR_INIT;
	}
	
	// Generate center point
	int c_x = this->focused_roi.tl().x + this->focused_roi.width / 2;
	int c_y = this->focused_roi.tl().y + this->focused_roi.height / 2;
	this->focused_roi_center = Point(c_x, c_y);
	
	// GET THUMB AND POINTER FINGERS
	double min_mag = 10000;
	int min_idx = 0;
	
	this->left_finger_locations.clear();

	// Left-hand -> get thumb and index fin
	if (!this->left_hand_hull[0].empty()) {

		// Thumb 
		for (i = 0; i < this->left_hand_hull[0].size(); i++) {
			double mag = sqrt(pow(this->left_hand_hull[0][i].x - this->focused_roi_center.x, 2) + \
				pow(this->left_hand_hull[0][i].y - this->focused_roi_center.y, 2));
			if (mag < min_mag) {
				min_mag = mag;
				min_idx = i;
			}
		}
		this->left_finger_locations.push_back(Point(this->left_hand_hull[0][min_idx]));

		// Find 5 fingers of left-hand (rotate convex polygone clockwise to find fingers)
		int lst_idx = min_idx;
		int next_idx = lst_idx - 1;
		while (this->left_finger_locations.size() < 5) {
			// subtract for clockwise rotation
			if (next_idx < 0) next_idx = this->left_hand_hull[0].size() - next_idx;
			double mag = sqrt(pow(this->left_hand_hull[0][lst_idx].x - this->left_hand_hull[0][next_idx].x, 2) + \
				pow(this->left_hand_hull[0][lst_idx].y - this->left_hand_hull[0][next_idx].y, 2));
			if (mag > 50)
				this->left_finger_locations.push_back(Point(this->left_hand_hull[0][next_idx]));
			lst_idx = next_idx;
			next_idx--;
		}

	}

right_side:

	this->right_finger_locations.clear();

	min_mag = 10000;
	min_idx = 0;

	// Right-hand
	if (!this->right_hand_hull[0].empty()) {

		// Thumb 
		for (i = 0; i < this->right_hand_hull[0].size(); i++) {
			double mag = sqrt(pow(this->right_hand_hull[0][i].x - this->focused_roi_center.x, 2) + \
				pow(this->right_hand_hull[0][i].y - this->focused_roi_center.y, 2));
			if (mag < min_mag) {
				min_mag = mag;
				min_idx = i;
			}
		}
		this->right_finger_locations.push_back(Point(this->right_hand_hull[0][min_idx]));

		// Find 5 fingers of left-hand (rotate convex polygone clockwise to find fingers)
		int lst_idx = min_idx;
		int next_idx = lst_idx + 1;
		while (this->right_finger_locations.size() < 5) {
			// subtract for clockwise rotation
			if (next_idx < 0) next_idx = next_idx % this->right_hand_hull[0].size();
			double mag = sqrt(pow(this->right_hand_hull[0][lst_idx].x - this->right_hand_hull[0][next_idx].x, 2) + \
				pow(this->right_hand_hull[0][lst_idx].y - this->right_hand_hull[0][next_idx].y, 2));
			if (mag > 50)
				this->right_finger_locations.push_back(Point(this->right_hand_hull[0][next_idx]));
			lst_idx = next_idx;
			next_idx++;
		}
	}

end:
	return 0;
}

int Analysis::estimateOpticalFlow(Mat frame, int update_count) {

	int i;

	Mat old_gray, new_gray;
	vector<uchar> status;
	vector<float> err;
	Size window_size(31, 31);
	int pyramid_level_used = 3;
	int max_count = 500;

	TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, update_count, 0.03);

	GH;

	if (!frame.empty())
		return ERROR_PARAM;

	if (!this->last_frame.empty()) {
		cvtColor(this->last_frame, old_gray, COLOR_BGR2GRAY);
		imshow("old_gray", old_gray);
	}

	cvtColor(frame, new_gray, COLOR_BGR2GRAY);

	if (this->last_frame.depth() || (this->frame_count % update_count) == 0) {
		goodFeaturesToTrack(new_gray, this->fing_flow_points[1], max_count, 0.01, 10, Mat(), 3, 3, 0, 0.04);
		cornerSubPix(new_gray, this->fing_flow_points[1], window_size, Size(-1, -1), termcrit);
	}

	if (this->last_skin_mask.empty() || this->skin_mask.empty())
		goto end;

	if ((this->fing_flow_points[0].size() > 0) && (this->fing_flow_points[1].size() > 0)) {
		calcOpticalFlowPyrLK(this->last_skin_mask, this->skin_mask, this->fing_flow_points[0], this->fing_flow_points[1], \
			status, err, window_size, pyramid_level_used, termcrit, 0, 0.001);
	}
	
	//this->fing_flow_points[0].clear();
	//this->fing_flow_points[0] = vector<Point2f>(fing_flow_points[1]);

end:
	return 0;
}

int Analysis::analyze(Mat in_frame, int flags) {

	int i = 0, j = 0, k = 0, l = 0, ret_err = 0;

	if (in_frame.empty()) {
		mk_verror(ERROR_PARAM, "input frame was empty", 0);
		return ERROR_PARAM;
	}

	this->width = in_frame.cols;
	this->height = in_frame.rows;
	this->frame_count++;		// increment count 

	Mat frame_ycrcb;
	// Confirm color type
	cvtColor(in_frame, frame_ycrcb, this->color_type);

	// Separate the skin from the input image
	if (flags & ANALYSIS_HIST)
		ret_err = createSkinMaskFromHist(frame_ycrcb, 80, THRESH_BINARY);
	else if (flags & ANALYSIS_RANGE)
		ret_err = createSkinMaskFromRange(frame_ycrcb);
	else
		ret_err = createSkinMaskFromRange(frame_ycrcb);
	if (ret_err) {
		mk_verror(ret_err, "failed to generate skin mask", 0);
		goto end;
	}

	// Generate the contours for the hand mask
	ret_err = createContours();
	if (ret_err) {
		mk_verror(ret_err, "failed to generate contours", 0);
		goto end;
	}

	// Construct the set of hulls 
	ret_err = createConvexHull();
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate outer hulls", 0);
		goto end;
	}
	else if (ret_err) {
		goto end;
	}

	// Create center points for all generated hulls
	ret_err = createHullCenterPoints();
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate inside hull moment points", 0);
		goto end;
	}

	// Separate each hand and create single hulls
	ret_err = createHandHulls();
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate hand hulls", 0);
		goto end;
	}

	// Constructs a center point between the two hands (used for thumb and index detection)
	ret_err = generateFocusedROI();
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate focused roi of hands", 0);
		goto end;
	}

	// Do again since we now have hull for each hand
	ret_err = createHullCenterPoints();
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate inside hull moment points", 0);
		goto end;
	}

	// 
	ret_err = estimateFingersLocations();
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate finger tip points", 0);
		goto end;
	}

	ret_err = estimateOpticalFlow(in_frame,10);
	if (ret_err == ERROR_FUNC) {
		mk_verror(ret_err, "failed to generate optical flow points", 0);
		goto end;
	}

	// Update last used frames
	this->last_frame.release();
	this->last_frame = Mat(in_frame);

	this->last_skin_mask.release();
	this->last_skin_mask = Mat(this->skin_mask);

end:
	return ret_err;
}

Mat Analysis::getPrintFrame(int flags) {

	int i = 0, j = 0;

	Mat output;

	// First sections is for 8-bit images (i.e. masks)
	if (flags & PRINT_SKIN_MASK) {
		output = Mat(this->skin_mask);
	}
	else { // Section is for 3-channel color images

		if (this->size.empty())
			goto end;

		// Initialize output image
		output = Mat(this->size, CV_8UC3);
		output.setTo(Scalar(0, 0, 0));

		if (output.empty())
			cout << "output emtpy" << endl;

		if (flags & PRINT_FOCUSEDROI) {
			int roi_x = this->focused_roi.tl().x + this->focused_roi.width / 2;
			int roi_y = this->focused_roi.tl().y + this->focused_roi.height / 2;
			circle(output, Point(roi_x, roi_y), 4, Scalar(255, 255, 0), 10, FILLED, 0);
			//rectangle(output, this->focused_roi.tl(), this->focused_roi.br(), Scalar(255, 255, 0), 2, LINE_8, 0);
		}

		// Draw contours of the hands
		if (flags & PRINT_CONTOURS) {
			for (i = 0; i < this->contours.size(); i++)
				drawContours(output, this->contours, i, Scalar(0, 255, 0), 1, 8, vector<Vec4i>(), 0, Point());
		}

		// Draw the main outer hull
		if (flags & PRINT_OUT_HULL) {
			for (i = 0; i < this->hull_points.size(); i++) {
				drawContours(output, this->hull_points, i, Scalar(0, 255, 255), 2, 8, vector<Vec4i>(), 0, Point());
			}
		}
		else {
			// Draws the combined left-hand hull
			if (flags & PRINT_LEFTHANDHULL) {
				if (!this->left_hand_hull.empty()) {
					for (i = 0; i < this->left_hand_hull.size(); i++) {
						drawContours(output, this->left_hand_hull, i, Scalar(0, 255, 255), 2, 8, vector<Vec4i>(), 0, Point());
					}
				}
			}

			// Draws the combined right-hand hull
			if (flags & PRINT_RIGHTHANDHULL) {
				if (!this->right_hand_hull.empty()) {
					for (i = 0; i < this->right_hand_hull.size(); i++) {
						drawContours(output, this->right_hand_hull, i, Scalar(0, 255, 255), 2, 8, vector<Vec4i>(), 0, Point());
					}
				}
			}
		}

		// Draw momemnt circles
		if (flags & PRINT_HAND_MOMES) {
			for (i = 0; i < this->mome_points.size(); i++) {
				circle(output, this->mome_points[i], 4, Scalar(0, 0, 255), 10, FILLED, 0);
			}
		}
		else {
			
			if (flags & PRINT_LEFTMOME) {
				if (!this->left_mome_points.empty()) {
					for (i = 0; i < this->left_mome_points.size(); i++) {
						circle(output, this->left_mome_points[i], 4, Scalar(0, 0, 255), 10, FILLED, 0);
					}
				}
			}
			if (flags & PRINT_RIGHTMOME) {
				if (!this->right_mome_points.empty()) {
					for (i = 0; i < this->right_mome_points.size(); i++) {
						circle(output, this->right_mome_points[i], 4, Scalar(0, 0, 255), 10, FILLED, 0);
					}
				}
			}
		}

		// Draw finger tip locations
		if (flags & PRINT_LEFTFINGERS) {
			if (!this->left_finger_locations.empty()) {
				for (i = 0; i < this->left_finger_locations.size(); i++) {
					circle(output, this->left_finger_locations[i], 4, Scalar(255, 0, 255), 6, FILLED, 0);
				}
			}
		}

		if (flags & PRINT_RIGHTFINGERS) {
			if (!this->right_finger_locations.empty()) {
				for (i = 0; i < this->right_finger_locations.size(); i++) {
					circle(output, this->right_finger_locations[i], 4, Scalar(255, 0, 255), 6, FILLED, 0);
				}
			}
		}

		if (flags & PRINT_OPTFLOW) {
			if (!this->fing_flow_points[1].empty()) {
				for (i = 0; i < this->fing_flow_points[1].size(); i++) {
					circle(output, this->fing_flow_points[1][i], 4, Scalar(0, 255, 255), 6, FILLED, 0);
				}
			}
		}

	}
end:
	return Mat(output);
}

Analysis::~Analysis() {

	if (!this->contours.empty())
		this->contours.clear();

	if (!this->hierarchy.empty())
		this->hierarchy.clear();

	if (!this->hull_indices.empty())
		this->hull_indices.clear();

	if (!this->hull_points.empty())
		this->hull_points.clear();

	if (!this->mome_points.empty())
		this->mome_points.clear();

}

//// Get the defects of the contours
	//this->convDef.clear();
	//this->convDef = vector<vector<Vec4i> >(this->contours.size());
	//for (i = 0; i < this->contours.size(); i++) {
	//    convexityDefects(this->contours[i], this->hull_indices[i], this->convDef[i]);
	//}

	//this->finger_locations.clear();
	//if (this->hull_indices.size() > 0 && this->convDef.size() > 0) {

	//    for (i = 0; i < this->contours.size(); i++) {

	//        for (k = 0; k < this->hull_indices[0].size(); k++) {

	//            for (j = 0; j < this->convDef[i].size(); j++) {

	//                if (this->convDef[i][j][3] < 256 * 2)
	//                    continue;

	//                // Get points from contours
	//                Point red = this->contours[i][this->convDef[i][j][0]];  //start point
	//                Point blue = this->contours[i][this->convDef[i][j][1]]; //end point
	//               float dist = sqrtf(powf((red.x - blue.x), 2) + powf((red.y - blue.y), 2));
	//                
	//                /* TESTS for Points */
	//                double test = -1;
	//                ////Check if point is inside major polygon and outside minor polygon
	//                //for (l = 0; l < inner_hull_pnts.size(); l++) {
	//                //    test = pointPolygonTest(inner_hull_pnts[l], red, false);
	//                //    if (test > 0)
	//                //        break;
	//                //}
	//                //if (test > 0) continue;

	//                // Check if points are below the center major hull points (i.e. the center points of the hands)
	//                for (l = 0; l < this->mome_points.size(); l++) {
	//                    if ((this->mome_points[l].y > red.y) || (this->mome_points[l].y > blue.y)) {
	//                        test = 1; // Assume everything above the hand centers is false
	//                        break;
	//                    }
	//                }
	//                if (test > 0) continue;

	//                // Add possibe finger locations to possible finger location vector
	//                this->finger_locations.push_back(Point(red));
	//            }
	//        }
	//    }
	//}