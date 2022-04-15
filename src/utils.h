#pragma once

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
struct coordinate
{
	float x, y;
};

float averagef(float* array_, size_t size_);
float distancef(coordinate* c1, coordinate* c2);
float distancef(float x1, float y1, float x2, float y2);