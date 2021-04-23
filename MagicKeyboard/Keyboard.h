#include <vector>
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

#ifndef IKEYBOARD_H
#define IKEYBOARD_H

enum KeyType { UNUSED, REGULAR, SHIFT };
struct Key
{
	KeyType type;
	char character;
	char characterAlt;

	Key() {}

	Key(char characterIn, char characterAltIn) : character(characterIn), characterAlt(characterAltIn)
	{
		type = REGULAR;
	}

	Key(KeyType typeIn) : type(typeIn) {}
};

class IKeyboard
{
public:
	virtual Key GetKeySymbol(Point) = 0;
};

class Keyboard : public IKeyboard
{
	Mat cameraPerspectiveMapped;
	std::vector<Key> keyCharacters;

	void HardcodeKeys();
	bool IsBefore(Rect, Rect);
	void SortKeyContours(std::vector<std::vector<Point>> &);

public:
	Keyboard(Mat);
	Key GetKeySymbol(Point);
};

#endif