#include <iostream>

#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "mkberror.hpp"
#include "FileReaderWriter.h"
#include "Keyboard.h"
#include "SkinMaskBasic.h"
#include "DetectionRandom.h"

#include "Analysis.h"

/* */
void HandleKeys(std::vector<Key> keys)
{
	bool isShift = false;

	for (int i = 0; i < keys.size(); i++)
		if (keys[i].type == SHIFT)
		{
			isShift = true;
			break;
		}

	for (int i = 0; i < keys.size(); i++)
	{
		Key key = keys[i];
		switch (key.type)
		{
		case REGULAR:
			if (!isShift)std::cout << key.character;
			else std::cout << key.characterAlt;
			break;
		default:
			break;
		}
	}
}

/* Main entry point for the program */
int main(int argc, char* argv[]) {

	int ret = 0;

	// Handle input arguments 
	//VideoCapture cap = GetVideoCapture("C:\\Users\\jrpratt\\Videos\\vlc-record-2021-04-10-23h58m43s-dshow___-nexus-60-deg.avi");
	VideoCapture cap = GetVideoCapture("C:\\Users\\jrpratt\\Videos\\vlc-record-2021-04-18-13h53m54s-dshow___-.avi");
	//VideoCapture cap = GetVideoCapture("C:\\Users\\jrpratt\\Videos\\vlc-record-2021-04-15-22h08m37s-dshow___-.avi");
	if (!cap.isOpened()) {
		mk_error(ERROR_NULL);
		return -1;
	}


	Mat frame;
	cap.read(frame);

	// Construct Used classes
	Keyboard keyboard = Keyboard(frame);
	Analysis analysis = Analysis(frame);

	// Generate histogram for skin detection
	analysis.loadHistImage("C:\\Users\\jrpratt\\Desktop\\COMS575\\CS575_MagicKeyboard\\images\\skin.jpg");
	analysis.loadHistImage("C:\\Users\\jrpratt\\Desktop\\COMS575\\CS575_MagicKeyboard\\images\\skin2.jpeg");
	analysis.generateHistogram();
	
	while (true) {

		cap.read(frame);
		if (frame.empty()) {
			return -1;
		}

		ret = analysis.analyze(frame, ANALYSIS_HIST);
		if (ret == ERROR_INIT) {
			continue;
		}
		else if (ret) {
			mk_verror(ERROR_FUNC,"analysis function failure");
			break;
		}

		frame.release();
		frame = analysis.getPrintFrame(PRINT_CONTOURS | PRINT_LEFTHANDHULL | \
			PRINT_RIGHTHANDHULL |  PRINT_FOCUSEDROI | PRINT_LEFTFINGERS | PRINT_RIGHTFINGERS | PRINT_OPTFLOW);
		
		if (frame.empty())
			mk_verror(ERROR_NULL,"print frame was NULL",0);
		else {
			imshow("Test", frame);
		}

		//waitKey(0);
		//continue;

		if (waitKey(5) >= 0)
			break;
	}

	// Release video frame information
	cap.release();
	return 0;
}


//int main() {
//	const char* inputVideoString = "typingCapture.avi";
//
//	VideoCapture cap = GetVideoCapture(inputVideoString);
//	if (!cap.isOpened()) return -1;
//
//	Mat frame;
//	cap.read(frame);
//
//	Keyboard keyboard = Keyboard(frame);
//
//	Mat skinMaskFrame;
//	SkinMaskBasic skinMask = SkinMaskBasic();
//	DetectionRandom detection = DetectionRandom();
//	while (cap.read(frame))
//	{
//		skinMaskFrame = skinMask.GetSkinMask(frame);
//		std::vector<Point> fingertipPoints = detection.GetFingertips(skinMaskFrame);
//
//		std::vector<Key> keys;
//		for (int i = 0; i < fingertipPoints.size(); i++)
//			keys.push_back(keyboard.GetKeySymbol(fingertipPoints[i]));
//
//		HandleKeys(keys);
//	}
//
//	cap.release();
//}