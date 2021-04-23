#include "Keyboard.h"

using namespace cv;

Keyboard::Keyboard(Mat keyboardImg)
{
	HardcodeKeys();

	cameraPerspectiveMapped = keyboardImg.clone();
	cameraPerspectiveMapped.setTo(Scalar(0, 0, 0));

	Mat gray;
	cvtColor(keyboardImg, gray, COLOR_BGR2GRAY);

	Mat thresh;
	adaptiveThreshold(gray, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 3, 20);

	std::vector<std::vector<Point>> contours;
	std::vector<Vec4i> hierarchy;
	findContours(thresh, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<Point>> keyContours;
	for (size_t i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect(contours[i]);
		double area = rect.area();

		double heightToWidth = rect.height / rect.width;

		if (area >= 1800 && area <= 30000 && heightToWidth < 2)
			keyContours.push_back(contours[i]);
	}

	SortKeyContours(keyContours);

	int keyCount = 1;
	for (int i = 0; i < keyContours.size(); i++)
	{
		std::vector<std::vector<Point>> keyContour;
		keyContour.push_back(keyContours[i]);

		Scalar color(keyCount, (10 * (i + 1)) % 255, abs(255 - (20 * i)) % 255);

		Rect rect = boundingRect(keyContours[i]);
		if (rect.area() / contourArea(keyContours[i]) >= 2)
			rectangle(cameraPerspectiveMapped, rect, color, FILLED);
		else
			drawContours(cameraPerspectiveMapped, keyContour, 0, color, FILLED, LINE_8);

		keyCount++;
	}

	//const char* window = "Window";
	//namedWindow(window, WINDOW_AUTOSIZE);
	//imshow(window, cameraPerspectiveMapped);

	//waitKey(0);
}

bool Keyboard::IsBefore(Rect prev, Rect next)
{
	Point centerPrev = (prev.br() + prev.tl()) * 0.5;
	Point centerNext = (next.br() + next.tl()) * 0.5;
	int yDif = centerPrev.y - centerNext.y; // findContours goes bottom to top, correct dif is ~0 or positive
	int xDif = centerPrev.x - centerNext.x; // findContours goes right to left, correct dif is positive

	bool isNextAboveRow = yDif >= 25;
	bool isNextLeft = xDif >= 50;

	return isNextAboveRow || isNextLeft;
}

void Keyboard::SortKeyContours(std::vector<std::vector<Point>> &keyContours)
{
	for (int i = 0; i < keyContours.size() - 1; i++)
	{
		for (int j = 0; j < keyContours.size() - i - 1; j++)
		{
			Rect prev = boundingRect(keyContours[j]);
			Rect next = boundingRect(keyContours[j + 1]);
			if (!IsBefore(prev, next))
				std::swap(keyContours[j], keyContours[j + 1]);
		}
	}
}

Key Keyboard::GetKeySymbol(Point fingertipPoint)
{
	Vec3b color = cameraPerspectiveMapped.at<Vec3b>(fingertipPoint);
	int idx = color[0] - 1;

	Key key;
	if (idx >= 0) return keyCharacters[idx];
	else return key;
}

// Ugly and space-consuming but should be consistent
void Keyboard::HardcodeKeys()
{
	// Top row
	keyCharacters.push_back(Key('`', '~')); // 14
	keyCharacters.push_back(Key('1', '!'));
	keyCharacters.push_back(Key('2', '@'));
	keyCharacters.push_back(Key('3', '#'));
	keyCharacters.push_back(Key('4', '$'));
	keyCharacters.push_back(Key('5', '%'));
	keyCharacters.push_back(Key('6', '^'));
	keyCharacters.push_back(Key('7', '&'));
	keyCharacters.push_back(Key('8', '*'));
	keyCharacters.push_back(Key('9', '('));
	keyCharacters.push_back(Key('0', ')'));
	keyCharacters.push_back(Key('-', '_'));
	keyCharacters.push_back(Key('=', '+'));
	keyCharacters.push_back(Key(UNUSED));

	keyCharacters.push_back(Key(UNUSED)); // 14
	keyCharacters.push_back(Key('q', 'Q'));
	keyCharacters.push_back(Key('w', 'W'));
	keyCharacters.push_back(Key('e', 'E'));
	keyCharacters.push_back(Key('r', 'R'));
	keyCharacters.push_back(Key('t', 'T'));
	keyCharacters.push_back(Key('y', 'Y'));
	keyCharacters.push_back(Key('u', 'U'));
	keyCharacters.push_back(Key('i', 'I'));
	keyCharacters.push_back(Key('o', 'O'));
	keyCharacters.push_back(Key('p', 'P'));
	keyCharacters.push_back(Key('[', '{'));
	keyCharacters.push_back(Key(']', '}'));
	keyCharacters.push_back(Key('\\', '\|'));

	keyCharacters.push_back(Key(UNUSED)); // 13
	keyCharacters.push_back(Key('a', 'A'));
	keyCharacters.push_back(Key('s', 'S'));
	keyCharacters.push_back(Key('d', 'D'));
	keyCharacters.push_back(Key('f', 'F'));
	keyCharacters.push_back(Key('g', 'G'));
	keyCharacters.push_back(Key('h', 'H'));
	keyCharacters.push_back(Key('j', 'J'));
	keyCharacters.push_back(Key('k', 'K'));
	keyCharacters.push_back(Key('l', 'L'));
	keyCharacters.push_back(Key(';', ':'));
	keyCharacters.push_back(Key('\'', '\"'));
	keyCharacters.push_back(Key('\n', '\n'));

	keyCharacters.push_back(Key(SHIFT)); // 12
	keyCharacters.push_back(Key('z', 'Z'));
	keyCharacters.push_back(Key('x', 'X'));
	keyCharacters.push_back(Key('c', 'C'));
	keyCharacters.push_back(Key('v', 'V'));
	keyCharacters.push_back(Key('b', 'B'));
	keyCharacters.push_back(Key('n', 'N'));
	keyCharacters.push_back(Key('m', 'M'));
	keyCharacters.push_back(Key(',', '<'));
	keyCharacters.push_back(Key('.', '>'));
	keyCharacters.push_back(Key('/', '?'));
	keyCharacters.push_back(Key(SHIFT));

	keyCharacters.push_back(Key(UNUSED)); // 7
	keyCharacters.push_back(Key(UNUSED));
	keyCharacters.push_back(Key(UNUSED));
	keyCharacters.push_back(Key(' ', ' '));
	keyCharacters.push_back(Key(UNUSED));
	keyCharacters.push_back(Key(UNUSED));
	keyCharacters.push_back(Key(UNUSED));
}