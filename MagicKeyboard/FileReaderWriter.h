#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

#ifndef FILE_READER_WRITER_H
#define FILE_READER_WRITER_H

Mat GetImage(const char*);
Mat GetImage(std::string);
VideoCapture GetVideoCapture(const char*);
VideoCapture GetVideoCapture(std::string);
VideoWriter GetVideoWriter(const char*, VideoCapture);
VideoWriter GetVideoWriter(std::string, VideoCapture);

#endif