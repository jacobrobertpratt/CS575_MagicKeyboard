#include "FileReaderWriter.h"
#include <iostream>

using namespace cv;

Mat GetImage(const char* imgString)
{
	return GetImage(std::string(imgString));
}

Mat GetImage(std::string imgString)
{
	Mat rgb = imread(imgString, IMREAD_COLOR);

	if (!rgb.data) std::cout << "Image '" << imgString << "' not found!" << std::endl;

	return rgb;
}

VideoCapture GetVideoCapture(const char* inputFileString)
{
	return GetVideoCapture(std::string(inputFileString));
}

VideoCapture GetVideoCapture(std::string inputFileString)
{
	VideoCapture input_cap(inputFileString);

	// Check validity of target file
	if (!input_cap.isOpened()) std::cout << "Input video " << inputFileString << " not found." << std::endl;

	return input_cap;
}

VideoWriter GetVideoWriter(const char* outputString, VideoCapture input_cap)
{
	return GetVideoWriter(std::string(outputString), input_cap);
}

VideoWriter GetVideoWriter(std::string outputString, VideoCapture input_cap)
{
	VideoWriter output_cap(outputString,
		VideoWriter::fourcc('H', '2', '6', '4'),
		input_cap.get(CAP_PROP_FPS),
		Size(input_cap.get(CAP_PROP_FRAME_WIDTH),
			input_cap.get(CAP_PROP_FRAME_HEIGHT)));

	if (!output_cap.isOpened()) std::cout << "Could not create output file " << outputString << std::endl;

	return output_cap;
}