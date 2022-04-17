#pragma once
#include "visualization_mode.h"

class MilkWay : public VisualizationMode
{
	ofColor barColor = ofColor::fromHsb(0, 255, 255), fillC, edgeC;
	const size_t nBranches = 8;
	int nRings;
	bool* pulse;
	float movAvgFft = 0, rotation = ofRandom(255),
		  spacing, barSize, hue_, avgFft, n, pulseTrigger,
		  radius, ringRotation, starAngle, centerDis;

public:
	MilkWay(FftConfig* fftConfig) : VisualizationMode("MilkWay", fftConfig)
	{
		//Settings
		_sensibility = 40;
		pulseTrigger = 1.05;

		//FFT Post-Processing
		configAutoCombineBands(4);
		configAutoDamper(1.03);
		configAutoRescale(1, 0.05, 1);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetFrameRate(60);
		barColor.setSaturation(255);
		barColor.setBrightness(255);
		_windowResized();
		addLayerFunction([&] {drawDefaultLayer0(); });
		addLayerFunction([&] {drawLayer1(); });
		DEBUG(
			setDebugLayerFunction([this] {drawDebugLayer(); });
		);

		//Malloc
		pulse = (bool*)malloc(nRings * sizeof(bool));
		memset(pulse, 0, nRings * sizeof(bool));
	}
	~MilkWay()
	{
		free(pulse);
	}
protected:
	void windowResized()
	{
		nRings = 0.7 * nBands / nBranches;
		if (nRings < 1) nRings = 1;
		spacing = halfHeight / (nRings + 1) / 0.65;
	}
	void update()
	{
		avgFft = averagef(fft, nBands);
		n = 4 * ofGetFrameRate() + 1; //Moving avg denominator
		movAvgFft = (n - 1) * movAvgFft / n + avgFft / n;
		rotation += 5E-6 * avgFft * avgFft * _sensibility;
	}

	// Trajectory circles
	void drawDefaultLayer0()
	{
		ofClear(0);
		fillC = ofColor::fromHsb(255, 255, 200);
		edgeC = ofColor::fromHsb(255, 255, 255);
		
		// Draws each ring
		for (int i = nRings - 1; i >= 0; i--)
		{
			if (i == 0) // Center circle
			{
				// Checks if pulse is triggered
				if (avgFft > movAvgFft * pulseTrigger) pulse[i] = true;
				else pulse[i] = false;
			}
			else
			{
				// Moves previous pulse
				pulse[i] = pulse[i - 1];
			}
			// Configures line if pulse is triggered
			ofSetLineWidth(pulse[i] ? 4 : 2);
			edgeC.setBrightness(pulse[i] ? 60 : 25);

			// Draws trajectory circle
			hue_ = fmodf(50 * rotation + 50 * (float)i / (float)nRings, 255);
			edgeC.setHue(hue_);
			ofSetColor(edgeC);
		  	ofNoFill();
			radius = (i + 1) * spacing; // ring radius
			centerDis = 7 * (i - 3 * (nRings + 1) / 4); // Y-axis displacement (3d effect)
			ofSetCircleResolution(calcCircleRes(radius));
			ofDrawEllipse(halfWidth, halfHeight + centerDis, radius * 2, 1.2 * radius);
		}
	}

	// Draws star elements
	float x, y, r; // star position and radius
	void drawLayer1()
	{
		ofClear(0);
		fillC = ofColor::fromHsb(255, 255, 200);
		edgeC = ofColor::fromHsb(255, 255, 255);

		// For each trajectory
		for (int i = nRings - 1; i >= 0; i--)
		{
			hue_ = fmodf(50 * rotation + 50 * (float)i / (float)nRings, 255);
			edgeC.setBrightness(255);
			edgeC.setHue(hue_);
			fillC.setBrightness(200);
			fillC.setHue(hue_);

			// Star reference position inside the trajectory
			ringRotation = rotation * (0.5 * (nRings - i) + 5);
			radius = (i + 1) * spacing;
			
			//Draws each star
			for (int j = 0; j < nBranches; j++)
			{
				// Calculates star position
				starAngle = 2 * PI * j / nBranches;
				centerDis = 7 * (i - 3 * (nRings + 1) / 4);
				x = halfWidth + (i + 1) * spacing * sinf(starAngle + ringRotation);
				y = halfHeight + centerDis + 0.6 * (i + 1) * spacing * cosf(starAngle + ringRotation);
				r = 1 + fft[(nRings - 1 - i) * nBranches + j] * _sensibility / 10 * (1 + 0.6 * cosf(starAngle + ringRotation));
				// Draws
				ofSetLineWidth(2);
				DRAW_W_EDGES_A(fillC, edgeC, 0.6 * 255, 1 * 255,
					ofSetCircleResolution(calcCircleRes(r));
					ofDrawCircle(x, y, r);
				)
			}
		}
	}
	DEBUG(
	// Debug view
	float zoom = 10000;
	float movBarSize = 0;
	void drawDebugLayer()
	{
		ofClear(0);
		ofFill();
		// Draw current fft avg level
		barSize = avgFft * zoom;
		ofSetColor(255, 255, 255);
		ofDrawRectangle(0, height, width / 50, -barSize);
		movBarSize = movAvgFft * zoom;
		// Draw fft moving avg level
		ofSetColor(200, 200, 200);
		ofDrawRectangle(width / 50, height, width / 50, -movBarSize);
		// Draw pulse trigger threshold
		ofSetColor(255, 0, 0);
		ofSetLineWidth(1);
		ofDrawLine(0, height - movBarSize * pulseTrigger, 2 * width / 50, height - movBarSize * pulseTrigger);
		// Ajusts zoom for resolution
		if (barSize > height) zoom *= 0.9;
	}
	); //DEBUG end
};