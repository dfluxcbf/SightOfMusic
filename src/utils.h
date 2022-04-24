#pragma once
#include "ofMain.h"

#define INT(x) (int)(x)

#define DRAW_W_EDGES(fC,eC,drawF) ofSetColor(fC); \
                            ofFill(); \
                            drawF \
                            ofSetColor(eC); \
                            ofNoFill(); \
                            drawF \
                            ofFill();

#define DRAW_W_EDGES_A(fC,eC,fA,eA, drawF) ofSetColor(fC, fA); \
                            ofFill(); \
                            drawF \
                            ofSetColor(eC, eA); \
                            ofNoFill(); \
                            drawF \
                            ofFill();

inline float sigmoidf(float x)
{
    return 1 / (1 + expf(-x));
}

#ifdef _DEBUG
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

struct Line
{
    ofPoint p1, p2;
    float length()
    {
        return sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));
    }
};

float averagef(const float* const array_, size_t size_);
float distancef(ofPoint* p1, ofPoint* p2);
float distancef(float x1, float y1, float x2, float y2);
float minf(const float* const array, size_t size);
float maxf(const float* const array, size_t size);
float sumf(const float* const array, size_t size);
int calcCircleRes(float radius);
ofPoint intersection(Line AB, Line CD);
bool isInsideScren(ofPoint p, ofRectangle screen);