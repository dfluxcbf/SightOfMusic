#include "utils.h"

#include <math.h>

float averagef(const float* const array_, size_t size_)
{
	float value = 0;
	for (int i = 0; i < size_; i++)
		value += array_[i];
	value /= size_;
	return value;
}

float distancef(ofPoint* p1, ofPoint* p2)
{
	return sqrt(powf(p1->x - p2->x, 2) +
		        powf(p1->y - p2->y, 2));
}
float distancef(float x1, float y1, float x2, float y2)
{
	return sqrt(powf(x1 - x2, 2) +
				powf(y1 - y2, 2));
}

float minf(const float* const array, size_t size)
{
	float _minVal = array[0];
	for (int i = 1; i < size; i++)
	{
		if (array[i] < _minVal) _minVal = array[i];
	}
	return _minVal;
}

float maxf(const float* const array, size_t size)
{
	float _maxVal = array[0];
	for (int i = 1; i < size; i++)
	{
		if (array[i] > _maxVal) _maxVal = array[i];
	}
	return _maxVal;
}

float sumf(const float* const array, size_t size)
{
	float sum = 0;
	for (int i = 0; i < size; i++)
	{
		sum += array[i];
	}
	return sum;
}

int calcCircleRes(float radius)
{
	return (int)(9.7 + radius / 3.9);
}

ofPoint intersection(Line AB, Line CD) {
	// Line AB represented as a1x + b1y = c1
	float a = AB.p2.y - AB.p1.y;
	float b = AB.p1.x - AB.p2.x;
	float c = a * (AB.p1.x) + b * (AB.p1.y);
	// Line CD represented as a2x + b2y = c2
	float a1 = CD.p2.y - CD.p1.y;
	float b1 = CD.p1.x - CD.p2.x;
	float c1 = a1 * (CD.p1.x) + b1 * (CD.p1.y);

	float det = a * b1 - a1 * b;
	if (det == 0) {
		return ofPoint{ INFINITY, INFINITY };
	}
	else {
		float x = (b1 * c - b * c1) / det;
		float y = (a * c1 - a1 * c) / det;
		return ofPoint{ x, y };
	}
}

bool isInsideScren(ofPoint p, ofRectangle screen)
{
	if (p.x < screen.x)
	{
		return false;
	}
	if (p.x > screen.x + screen.width)
	{
		return false;
	}
	if (p.y < screen.y)
	{
		return false;
	}
	if (p.y > screen.y + screen.height)
	{
		return false;
	}
	return true;
}