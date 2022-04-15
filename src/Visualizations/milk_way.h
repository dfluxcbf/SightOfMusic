#pragma once
#include "visualization_mode.h"

class MilkWay : public VisualizationMode
{
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	const size_t nBranches = 8, removeRings = 5;
	int nRings;
	bool* pulse;
	float movAvgFft = 0, rotation = ofRandom(255),
		  spacing, barSize, hue_, avgFft, n, pulseTrigger,
		  radius, ringRotation, starAngle, centerDis;

public:
	MilkWay(FftConfig* fftConfig) : VisualizationMode("MilkWay", fftConfig)
	{
		_sensibility = 40;
		configAutoDamper(1.03);
		configAutoRescale(1, 0.05, 1);
		configAutoCombineBands(4);
		ofBackground(0, 0, 0);
		ofSetFrameRate(60);
		barColor.setSaturation(255);
		barColor.setBrightness(255);
		_windowResized();
		pulse = (bool*)malloc(nRings * sizeof(bool));
		memset(pulse, 0, nRings * sizeof(bool));
		addLayerFunction([&] {drawDefaultLayer0(); });
		addLayerFunction([&] {drawLayer1(); });
#ifdef _DEBUG
		setDebugLayerFunction([&] {drawDebugLayer(); });
#endif
	}
	~MilkWay()
	{
		free(pulse);
	}
protected:
	void windowResized()
	{
		nRings = 0.7 * nBands_combined / nBranches;
		if (nRings < 1) nRings = 1;
		spacing = halfHeight / (nRings + 1) / 0.65;
	}
	void keyPressed(int key) {}
	void keyReleased(int key) {}
	void update()
	{
		pulseTrigger = 1.05;
		avgFft = averagef(_combinedFft, nBands_combined);
		n = 4 * ofGetFrameRate() + 1;
		movAvgFft = (n - 1) * movAvgFft / n + avgFft / n;
		rotation += 3E-7 * avgFft * avgFft * _sensibility;
	}
	void drawDefaultLayer0()
	{
		ofClear(0);
		ofColor fillC = ofColor::fromHsb(255, 255, 200);
		ofColor edgeC = ofColor::fromHsb(255, 255, 255);
		for (int i = nRings - 1; i >= 0; i--)
		{
			centerDis = 7*(i - 3*(nRings + 1) / 4);
			if (i == 0)
			{
				if (avgFft > movAvgFft * pulseTrigger) pulse[i] = true;
				else pulse[i] = false;
			}
			else
			{
				pulse[i] = pulse[i - 1];
			}
			ofSetLineWidth(pulse[i] ? 4 : 2);
			hue_ = fmodf(50 * rotation + 50 * (float)i / (float)nRings, 255);
			edgeC.setHue(hue_);
			edgeC.setBrightness(pulse[i] ? 60 : 25);
			ofSetColor(edgeC);
		  	ofNoFill();
			radius = (i + 1) * spacing;
			ofSetCircleResolution(calcCircleRes(radius));
			ofDrawEllipse(halfWidth, halfHeight + centerDis, radius * 2, 1.2 * radius);
		}
	}
	void drawLayer1()
	{
		ofClear(0);
		ofColor fillC = ofColor::fromHsb(255, 255, 200);
		ofColor edgeC = ofColor::fromHsb(255, 255, 255);
		for (int i = nRings - 1; i >= 0; i--)
		{
			centerDis = 7 * (i - 3 * (nRings + 1) / 4);
			hue_ = fmodf(50 * rotation + 50 * (float)i / (float)nRings, 255);
			radius = (i + 1) * spacing;
			edgeC.setBrightness(255);
			fillC.setHue(hue_);
			fillC.setBrightness(200);
			edgeC.setHue(hue_);
			ringRotation = rotation * (0.5 * (nRings - i) + 5);
			float x, y, r;
			for (int j = 0; j < nBranches; j++)
			{
				x = halfWidth + (i + 1) * spacing * sinf(starAngle + ringRotation);
				y = halfHeight + centerDis + 0.6 * (i + 1) * spacing * cosf(starAngle + ringRotation);
				r = 1 + _combinedFft[(nRings - 1 - i) * nBranches + j] * _sensibility / 10 * (1 + 0.6 * cosf(starAngle + ringRotation));
				starAngle = 2 * PI * j / nBranches;
				ofSetLineWidth(2);
				DRAW_W_EDGES_A(fillC, edgeC, 0.6 * 255, 1 * 255,
					ofSetCircleResolution(calcCircleRes(r));
					ofDrawCircle(x, y, r);
				)
			}
		}
	}
#ifdef _DEBUG
	void drawDebugLayer()
	{
		ofClear(0);
		static float zoom = 10000;
		float barSize = avgFft * zoom;
		float movBarSize = movAvgFft * zoom;
		if (barSize > height) zoom *= 0.9;
		ofSetColor(255, 255, 255);
		ofFill();
		ofDrawRectangle(0, height, width / 50, -barSize);
		ofSetColor(200, 200, 200);
		ofDrawRectangle(width / 50, height, width / 50, -movBarSize);
		ofSetColor(255, 0, 0);
		ofSetLineWidth(1);
		ofDrawLine(0, height - movBarSize * pulseTrigger, 2 * width / 50, height - movBarSize * pulseTrigger);
	}
#endif

private:
	int calcCircleRes(float radius)
	{
		return (int)(9.7 + radius / 3.9);
	}
};