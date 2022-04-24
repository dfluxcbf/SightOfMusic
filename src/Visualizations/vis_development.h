#pragma once
#include "visualization_mode.h"

//#define _DEVELOPING_ Develop

class Develop : public VisualizationMode
{

public:
	Develop(FftConfig* fftConfig) : VisualizationMode("Development", fftConfig)
	{
		//Settings
		_sensibility = 100;
		_dtSpeed = 1000;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		//configAutoCombineBands(2);
		//configAutoRescale(1, 0, 0.5);
		//configAutoDamper(1.03);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetLineWidth(1);
		addLayerFunction([this] { drawHair(); });
		//addLayerFunction([this] { drawPulses(); });
		//addLayerFunction([this] { drawMechanicAxis(); });
		//addLayerFunction([this] { drawIntersectionPoint(); });
		//addLayerFunction([this] { drawRandomMovement(); });
		//addLayerFunction([this] { draw_position_based_on_distance(); });
		_windowResized();
	}

	void windowResized() {}
	void update() {}

private:
	static const size_t XW = 270;
	static const size_t YH = 150;
	Line hair[XW][YH];
	void drawHair()
	{
		static ofPoint blow(500+ofRandom(width- 1000), 500+ofRandom(height - 1000));
		static bool init = false;
		const float dd = 7.5;
		if (!init)
		{
			ofSetLineWidth(2);
			for (int i = 0; i < XW; i++)
			{
				for (int j = 0; j < YH; j++)
				{
					hair[i][j] =
						Line
					{
						ofPoint(i * dd, j * dd),
						ofPoint(i * dd, j * dd)
					};
				}
			}
			init = true;
		}
		ofClear(0);
		ofSetColor(255, 255, 255);
		ofSetLineWidth(1);
		float d;
		//blow.x += 10.5 * sin(dt/1700) + ofRandomf() * 1E-3;
		//blow.y += 5.5 * cos(dt/2000) + ofRandomf() * 1E-3;
		blow.x = halfWidth * (1 + 0.75*sin(dt/5633));
		blow.y = halfHeight * (1 + 0.75 * sin(dt/3751));
		for (int i = 0; i < XW; i++)
		{
			for (int j = 0; j < YH; j++)
			{
				hair[i][j].p2.y += 0.5;
				d = distancef(&blow, &hair[i][j].p1);
				if (d > 0 && d < 100)
				{
					hair[i][j].p2.x += 300*sin(asin((hair[i][j].p1.x - blow.x) / d)) / sqrt(d);
					hair[i][j].p2.y += 300*cos(acos((hair[i][j].p1.y - blow.y) / d)) / sqrt(d);
				}
				d = hair[i][j].length();
				if (d > 50)
				{
					hair[i][j].p2 = hair[i][j].p1 + 50 * (hair[i][j].p2 - hair[i][j].p1) / d;
				}
				ofDrawLine(hair[i][j].p1, hair[i][j].p2);
			}
		}
	}

	void drawPulses()
	{
		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);
		ofClear(0);
		ofSetColor(255,255,255);
		float x, y;
		float d = 15;
		for (int i = -halfWidth/d; i < halfWidth / d; i++)
		{
			for (int j = -halfHeight/d; j < halfHeight / d; j++)
			{
				x = d * i;
				y = d * j;

				float pulse1, pulse2, pulse3;
				pulse1 =
					abs(
						distancef(0, 0, x, y) -
						distancef(0, 0, 0, fmodf(dt, width))
					) / 20;
				pulse2 =
					abs(
						distancef(halfWidth / 2, 0, x, y) -
						distancef(halfWidth / 2, 0, halfWidth / 2, fmodf(PI * dt, width))
					) / 20;
				pulse3 =
					abs(
						distancef(0, halfHeight, x, y) -
						distancef(0, halfHeight, fmodf(2.7531 * dt, width), halfHeight)
					) / 20;
				pulse1 *= pulse1;
				pulse2 *= pulse2;
				pulse3 *= pulse3;
				pulse1 += 1;
				pulse2 += 1;
				pulse3 += 1;
				ofDrawCircle(
					x,
					y,
					1 + d / pulse1 + d / pulse2 + d / pulse3
					);
			}
		}
		ofPopMatrix();
	}
	
	void drawMechanicAxis()
	{
		static float r = halfHeight * 0.9;
		static bool init = false;
		static Line line1, line2;
		static float acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
		static ofColor fillC = ofColor::fromHsb(0, 255, 255);
		if (!init)
		{
			init = true;
		}
		//ofClear(0);
		ofSetColor(0, 0, 0, 3);
		ofDrawRectangle(0, 0, width, height);

		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);

		acc1 += averagef(fft, nBands / 4) / 20;
		acc2 += averagef(fft + nBands / 4, nBands / 4) / 20;
		acc3 += averagef(fft + 2 * nBands / 4, nBands / 4) / 20;
		acc4 += averagef(fft + 3 * nBands / 4, nBands / 4) / 20;

		static bool D = false;
		static float pulseR = 0;
		static float accPhase = 0;

		pulseR = 1 + averagef(fft, nBands) * averagef(fft, nBands) * 200;
		accPhase += 0.0001+averagef(fft, nBands)/90;
		float phase = accPhase;
		float nLoops = 4;
		for (int i = 0; i < nLoops; i++)
		{
			float movement = (D ? 2 : 1) * 0.2375 * PI * sin(acc1);
			line1.p1 = ofPoint(r * sin(phase + movement), r * cos(phase + movement));

			phase += PI / 2;
			movement = (D ? 2 : 1) * 0.2375 * PI * sin(acc2);
			line2.p1 = ofPoint(r * sin(phase + movement), r * cos(phase + movement));

			phase += PI / 2;
			movement = (D ? 2 : 1) * 0.2375 * PI * sin(acc3);
			line1.p2 = ofPoint(r * sin(phase + movement), r * cos(phase + movement));

			phase += PI / 2;
			movement = (D ? 2 : 1) * 0.2375 * PI * sin(acc4);
			line2.p2 = ofPoint(r * sin(phase + movement), r * cos(phase + movement));

			ofPoint inter = intersection(line1, line2);
			fillC.setHue(fillC.getHue() + 1);
			if (isInsideScren(inter, ofRectangle(-halfWidth, -halfHeight, width, height)))
			{
				ofNoFill();
				ofSetColor(0,0,255);
				ofDrawCircle(inter, pulseR);
			}
		}
		ofPopMatrix();
	}

	void drawIntersectionPoint()
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