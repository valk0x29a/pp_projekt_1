#include<stdbool.h>
//#include "helper.c"
#ifndef _HELPER_H
#define _HELPER_H

int min(int a, int b);
int max(int a, int b);
float minf(float a, float b);
float maxf(float a, float b);
int bound(int value, int min, int max);
float boundf(float value, float min, float max);
bool isInRange(float x1, float y1, float x2, float y2, float range);

#endif