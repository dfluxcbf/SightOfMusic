#pragma once
#include "visualization_mode.h"

class RainbowBars : public VisualizationMode
{
	ofColor fillColor = ofColor::fromHsb(0, 255, 255);
	float barLength = 0, barSize = 0, hue_ = 0;
	ofRectangle rect;

public:
	RainbowBars(FftConfig* fftConfig) : VisualizationMode("RainbowBars", fftConfig)
	{
		//Settings
		_sensibility = 220;
		configIgnoredIndices(0,0.5);
		configAutoRescale(0.9, 0.02f / nBands, 0.33);
		configAutoDamper(1.04);
		configAutoMinRatio();

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofFill();
		fillColor.setSaturation(255);
		fillColor.setBrightness(255);
		_windowResized();
		addLayerFunction([this] { drawDefaultLayer0(); });
	}

	void windowResized()
	{
		barLength = halfWidth / (nBands+1);
		rect.width = barLength;
		rect.y = halfHeight;
	}
	void update()
	{
		avg = averagef(fft, nBands);
		_dtSpeed = 600 * avg * avg * avg;
	}

private:
	float avg;
	void drawDefaultLayer0()
	{
		for (int i = 0; i < nBands; i++)
		{
			barSize = 1 + _sensibility * fft[i];
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
		ofSetColor(0, 0, 0, 800.f / ofGetFrameRate());
		ofDrawRectangle(0,0,width,height);
	}
};