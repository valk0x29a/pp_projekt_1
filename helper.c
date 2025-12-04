#include<stdbool.h>

int min(int a, int b)
{
	return a < b ? a : b;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

float minf(float a, float b)
{
	return a < b ? a : b;
}

float maxf(float a, float b)
{
	return a > b ? a : b;
}

int bound(int value, int min, int max)
{
	if(value < min) { return min; }
	if(value > max) { return max; }
	return value;
}

float boundf(float value, float min, float max)
{
	if(value < min) { return min; }
	if(value > max) { return max; }
	return value;
}

bool isInRange(float x1, float y1, float x2, float y2, float range)
{
	return abs(x2 - x1) < range && abs(y2 - y1) < range;
}