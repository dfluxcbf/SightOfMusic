#pragma once
#include "visualization_mode.h"

class Pillars : public VisualizationMode
{
	ofColor fillC = ofColor(0, 0, 0),
			fillCL = ofColor::fromHsb(0, 150, 110),
			backC = ofColor::fromHsb(0, 200, 40),
			edgeCC = ofColor::fromHsb(0, 150, 255),
			edgeCL = ofColor::fromHsb(0, 140, 150);
	float circleDist = 0, xSize = 0, ySize = 0, backProportion = 0.75;
public:
	Pillars(FftConfig* fftConfig) : VisualizationMode("Pillars", fftConfig)
	{
		//Settings
		_sensibility = 200;
		_dtSpeed = 2;
		configIgnoredIndices(0, 0.5);
		configAutoCombineBands(3);
		configAutoRescale(0.9, 0.01, 0.4);
		configAutoDamper(1.08);
		configAutoMinRatio();
		configAutoDamper(1.03);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetLineWidth(2);
		_windowResized();
		addLayerFunction([this] {
			static bool frameSkip = false;
			frameSkip = !frameSkip;
			if (frameSkip)
			{
				drawBackground();
			} 
		});
		addLayerFunction([this] { drawDefaultLayer0(); });
	}
	void windowResized()
	{
		xSize = height / 20;
		ySize = xSize / 2;
		circleDist = 3 * 0.75 * 0.8 * height / nBands;
		height15 = height * 0.15;
		width10 = width * 0.1;
		height15ySize = height * 0.15 + 2 * ySize;
	}

	void update()
	{
		edgeCC.setHue(fmodf(dt, 255.0));
		edgeCL.setHue(fmodf(dt + 100, 255.0));
		fillCL.setHue(fmodf(dt + 100, 255.0));
		backC.setHue(fmodf(dt + 100, 255.0));
		avg = averagef(fft, nBands);
	}

private:
	ofPoint p, rp;
	float height15, width10, height15ySize;
	float avg;
	void drawDefaultLayer0()
	{
		ofClear(0);
		ofPushMatrix();
		ofTranslate(halfWidth, height*0.875);
		ofRotate(180);
		for (int i = 0; i < nBands/3; i++)
		{
			p.x = halfWidth/2;
			p.y = i * circleDist + height15;
			rp.x = xSize + _sensibility * fft[3*i];
			rp.y = ySize + _sensibility * fft[3*i] / 2;
			rp.x *= backProportion;
			rp.y *= backProportion;
			ofSetCircleResolution(calcCircleRes(rp.x));
			DRAW_W_EDGES(
				fillC, edgeCC,
				ofDrawEllipse(p.x, p.y, rp.x, rp.y);
			);
			p.y = i * circleDist;
			rp.x = xSize + _sensibility * fft[3 * i + 1];
			rp.y = ySize + _sensibility * fft[3 * i + 1] / 2;
			ofSetCircleResolution(calcCircleRes(rp.x));
			DRAW_W_EDGES(
				fillC, edgeCC,
				ofDrawEllipse(0, p.y, rp.x, rp.y);
			);
			p.y = i * circleDist + height15;
			rp.x = xSize + _sensibility * fft[3 * i + 2];
			rp.y = ySize + _sensibility * fft[3 * i + 2] / 2;
			rp.x *= backProportion;
			rp.y *= backProportion;
			ofSetCircleResolution(calcCircleRes(rp.x));
			DRAW_W_EDGES(
				fillC, edgeCC,
				ofDrawEllipse(-p.x, p.y, rp.x, rp.y);
			);
		}
		ofPopMatrix();
	}
	void drawBackground()
	{
		ofClear(0);
		ofPushMatrix();
		ofTranslate(halfWidth, height * 0.875);
		ofRotate(180);

		p.x = halfWidth / 2 + 3*xSize;

		ofSetColor(backC);
		ofBeginShape();
		ofVertex(p.x + width10, height15ySize);
		ofVertex(0, -ySize / 2 - halfWidth / 10);
		ofVertex(-p.x - width10, height15ySize);
		ofEndShape(true);

		ofSetColor(edgeCL);
		ofBeginShape();
		ofVertex(p.x, height15ySize);
		ofVertex(0, -ySize / 2);
		ofVertex(-p.x, height15ySize);
		ofEndShape(true);

		ofSetColor(backC);
		ofDrawRectangle(p.x + width10 - 1, height15ySize, - 2 * (p.x + width10 - 1), height * 0.576);

		ofSetColor(fillCL);
		for (int i = 0; i < nBands / 9; i++)
		{
			p.y = (3 * i + 1) * circleDist + height15ySize;
			ofSetLineWidth(2+2*sumf(&fft[9*i], 9));
			ofDrawLine(p.x + width10, p.y, -p.x - width10, p.y);
		}
		ofSetLineWidth(3 + 10 * avg * avg);
		for (int i = 0; i < 5; i++)
		{
			ofDrawLine(0, -ySize / 2 - halfWidth * i / 40,
				p.x + halfWidth * i / 20, height15ySize);
			ofDrawLine(0, -ySize / 2 - halfWidth * i / 40,
				-p.x - halfWidth * i / 20, height15ySize);
		}
		ofDrawLine(p.x + width10, height15ySize, -p.x - width10, height15ySize);
		p.y = height * 0.776;
		ofSetLineWidth(2);
		ofDrawLine(p.x + width10 - 1, p.y, p.x + width10 - 1, height15ySize);
		ofDrawLine(-p.x - width10 + 1, p.y, -p.x - width10 + 1, height15ySize);
		ofPopMatrix();
	}
};