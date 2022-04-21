#pragma once
#include "visualization_mode.h"

#define _DEVELOPING_ Develop

class Develop : public VisualizationMode
{

public:
	Develop(FftConfig* fftConfig) : VisualizationMode("Development", fftConfig)
	{
		//Settings
		_sensibility = 100;
		_dtSpeed = 0.05;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		//configAutoCombineBands(2);
		//configAutoRescale(1, 0, 0.5);
		//configAutoDamper(1.03);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetLineWidth(1);
		addLayerFunction([this] { drawMechanicAxis(); });
		//addLayerFunction([this] { drawRandomMovement(); });
		//addLayerFunction([this] { draw_position_based_on_distance(); });
		_windowResized();
	}

	void windowResized() {}
	void update() {}

private:
	void drawMechanicAxis()
	{
		static Line ab, cd;
		static ofPoint p;
		static bool init = false;
		static ofColor color = ofColor::fromHsb(0, 255, 255);
		if (!init)
		{
			ab.p1.x = 0;
			ab.p2.y = height;

			cd.p1.y = 0;
			cd.p2.x = 0;
			cd.p2.y = height;
			init = true;
		}
		ab.p2.x = 1.1 * width * (0.5 + 0.45 * cos(2*PI*dt*log(dt)));
		ab.p1.y = height * (0.4 + 0.4 * cos(10*dt));
		cd.p1.x = width * (2 + 1.9 * cos(dt* sqrt(dt)));
		p = intersection(ab, cd);

		//ofClear(0);
		ofSetColor(0,0,0,4);
		ofDrawRectangle(0,0,width,height);

		ofSetColor(255,0,0);
		ofDrawLine(ab.p1.x, ab.p1.y, ab.p2.x, ab.p2.y);
		ofDrawLine(cd.p1.x, cd.p1.y, cd.p2.x, cd.p2.y);

		color.setHue(fmodf(color.getHue()+1, 255.f));
		ofSetColor(color);
		ofDrawCircle(p.x, p.y, 10);
	}

	void drawRandomMovement()
	{
		//ofClear(0);
		static bool initiated = false;
		static ofPoint flow[100][100];
		if (!initiated)
		{
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < 100; j++)
				{
					flow[i][j].x = ofRandomf() * 50;
					flow[i][j].y = ofRandomf() * 50;
				}
			}
			initiated = true;
		}
		static ofPoint p(halfWidth, halfHeight);
		ofSetColor(0, 0, 0, 5);
		ofDrawRectangle(0,0,width,height);
		ofSetColor(255,255,255);
		ofDrawCircle(p.x, p.y,10);
		float flowX = 99 * p.x / width, flowY = 99 * p.y / height;
		/*
		LOG("flowX: %f, flowY: %f", flowX, flowY);
		LOG("[%d][%d]%f", (int)floorf(flowX), (int)floorf(flowY),
			distancef(flowX, flowY, floorf(flowX), floorf(flowY)) / sqrt(8));
		LOG("[%d][%d]%f", (int)ceilf(flowX), (int)floorf(flowY),
			distancef(flowX, flowY, ceilf(flowX), floorf(flowY)) / sqrt(8));
		LOG("[%d][%d]%f", (int)floorf(flowX), (int)ceilf(flowY),
			distancef(flowX, flowY, floorf(flowX), ceilf(flowY)) / sqrt(8));
		LOG("[%d][%d]%f", (int)ceilf(flowX), (int)ceilf(flowY),
			distancef(flowX, flowY, ceilf(flowX), ceilf(flowY)) / sqrt(8));
		LOG("SUM: %f", 
			(distancef(flowX, flowY, floorf(flowX), floorf(flowY)) +
				distancef(flowX, flowY, ceilf(flowX), floorf(flowY)) +
				distancef(flowX, flowY, floorf(flowX), ceilf(flowY)) +
				distancef(flowX, flowY, ceilf(flowX), ceilf(flowY))) / sqrt(8)
			);
		*/
		
		p.x += flow[(int)floorf(flowX)][(int)floorf(flowY)].x * distancef(flowX, flowY, floorf(flowX), floorf(flowY)) / sqrt(8);
		p.x += flow[(int)ceilf(flowX)][(int)floorf(flowY)].x * distancef(flowX, flowY, ceilf(flowX), floorf(flowY)) / sqrt(8);
		p.x += flow[(int)floorf(flowX)][(int)ceilf(flowY)].x * distancef(flowX, flowY, floorf(flowX), ceilf(flowY)) / sqrt(8);
		p.x += flow[(int)ceilf(flowX)][(int)ceilf(flowY)].x * distancef(flowX, flowY, ceilf(flowX), ceilf(flowY)) / sqrt(8);
		
		p.y += flow[(int)floorf(flowX)][(int)floorf(flowY)].y * distancef(flowX, flowY, floorf(flowX), floorf(flowY)) / sqrt(8);
		p.y += flow[(int)ceilf(flowX)][(int)floorf(flowY)].y * distancef(flowX, flowY, ceilf(flowX), floorf(flowY)) / sqrt(8);
		p.y += flow[(int)floorf(flowX)][(int)ceilf(flowY)].y * distancef(flowX, flowY, floorf(flowX), ceilf(flowY)) / sqrt(8);
		p.y += flow[(int)ceilf(flowX)][(int)ceilf(flowY)].y * distancef(flowX, flowY, ceilf(flowX), ceilf(flowY)) / sqrt(8);
		if (p.x > width) p.x = width;
		if (p.y > height) p.y = height;
		if (p.x < 0) p.x = 0;
		if (p.y < 0) p.y = 0;
	}
	void draw_position_based_on_distance()
	{
		//ofClear(0);
		ofSetColor(0, 0, 0, 5);
		ofDrawRectangle(0,0,width,height);
		ofSetColor(255, 255, 255);
		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);
		ofRotate(dt);
		float d, avg, r;
		ofPoint p;
		static bool b = true;
		avg = averagef(fft,nBands)*_sensibility;
		for (int i = -39; i < 40; i++) 
		{
			for (int j = -39; j < 40; j++)
			{
				p.x = height * i / 81;
				p.y = height * j / 81;

				d = distancef(p.x, p.y,0,0);
				if (d > halfHeight * 0.95) continue;
				
				r = halfHeight / 21 / (d /20 + 1);
				if (r < 1) r = 1;
				if (d != 0)
				{
					p.x += 10 * sin(asin(p.x / d)) * avg / (d/100+1);
					p.y += 10 * cos(acos(p.y / d)) * avg / (d/100+1);
					ofDrawCircle(p.x, p.y, r, r);
				}
			}
		}
		b = false;
		ofPopMatrix();
	}
};