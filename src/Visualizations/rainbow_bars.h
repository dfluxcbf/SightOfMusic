#pragma once
#include "visualization_mode.h"

#define _DEVELOPING_ RainbowBars

class RainbowBars : public VisualizationMode
{
	ofColor fillColor = ofColor::fromHsb(0, 255, 255);
	float barLength = 0, minVal = 0, barSize = 0, hue_ = 0;
	ofRectangle rect;

public:
	RainbowBars(FftConfig* fftConfig) : VisualizationMode("RainbowBars", fftConfig)
	{
		//Settings
		_sensibility = 220;
		_dtSpeed = 35;
		configIgnoredIndices(0,0.5);
		configAutoRescale(1, 0.01f / nBands, 0.4);
		configAutoDamper(1.04);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofFill();
		fillColor.setSaturation(255);
		fillColor.setBrightness(255);
		_windowResized();
		addLayerFunction([&] { drawDefaultLayer0(); });
	}

	void windowResized()
	{
		barLength = halfWidth / nBands;
		rect.width = barLength;
		rect.y = halfHeight;
	}
	void update()
	{
		minVal = minf(fft, nBands);
	}

private:
	void drawDefaultLayer0()
	{
		for (int i = 0; i < nBands-1; i++)
		{
			barSize = 1 + _sensibility * (fft[i] + fft[i+1]/5 - minVal);
			hue_ = dt + 255.f * (float)i / (float)nBands;
			hue_ = fmodf(hue_, 255);

			fillColor.setHue(hue_);
			ofSetColor(fillColor);

			// Bottom Right
			rect.x = halfWidth + i * barLength;
			rect.height = barSize;
			ofDrawRectangle(rect);

			// Top Right
			rect.height = -rect.height;
			ofDrawRectangle(rect);

			// Bottom Left
			rect.x = halfWidth - i * barLength;
			rect.height = barSize;
			ofDrawRectangle(rect);

			// Top left
			rect.height = -rect.height;
			ofDrawRectangle(rect);
		}
		// Fading
		ofSetColor(0, 0, 0, 600.f / ofGetFrameRate());
		ofDrawRectangle(0,0,width,height);
	}
};