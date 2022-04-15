#include "utils.h"

#include <math.h>

float averagef(float* array_, size_t size_)
{
	float value = 0;
	for (int i = 0; i < size_; i++)
		value += array_[i];
	value /= size_;
	return value;
}

float distancef(coordinate* c1, coordinate* c2)
{
	return sqrt(powf(c1->x - c2->x, 2) +
		        powf(c1->y - c2->y, 2));
}
float distancef(float x1, float y1, float x2, float y2)
{
	return sqrt(powf(x1 - x2, 2) +
				powf(y1 - y2, 2));
}