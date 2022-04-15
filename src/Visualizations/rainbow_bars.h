#pragma once
#include "visualization_mode.h"

class RainbowBars : public VisualizationMode
{
public:
	RainbowBars(FftConfig* fftConfig) : VisualizationMode("RainbowBars", fftConfig, 1)
	{
		_sensibility = 220;
		_dtSpeed = 5;
		configIgnoredIndices(0,0.4);
		configAutoDamper(1.09);
		configAutoRescale(1, -1.f/(nBands*0.98), 0.4);
		ofBackground(0, 0, 0);
		ofFill();
		barColor.setSaturation(255);
		barColor.setBrightness(255);
		_windowResized();
	}

	void windowResized()
	{
		barLength = halfWidth / nBands;
		rect.width = barLength;
		rect.y = halfHeight;
	}
	void keyPressed(int key) {}
	void keyReleased(int key) {}
	float find_smallest_number(float* random_array, size_t size)
	{
		float minPtr = *random_array;
		for (int i = 1; i < size; i++)
		{
			if (*(random_array + i) < minPtr)
				minPtr = *(random_array + i);
		}
		return minPtr;
	}
	void update() {}
	void drawDefaultLayer0()
	{
		ofClear(0);
		float barSize;
		float hue_;
		for (int i = 0; i < nBands; i++)
		{
			barSize = 1+_sensibility * (_dampedFft[i] - find_smallest_number(_dampedFft, nBands));
			hue_ = dt + 255.0 * (float)i / (float)nBands;
			hue_ = fmodf(hue_, 255);

			barColor.setHue(hue_);
			ofSetColor(barColor);

			rect.x = halfWidth + i * barLength;
			rect.height = barSize;
			ofDrawRectangle(rect);

			rect.height = -rect.height;
			ofDrawRectangle(rect);

			rect.x = halfWidth - i * barLength;
			rect.height = barSize;
			ofDrawRectangle(rect);

			rect.height = -rect.height;
			ofDrawRectangle(rect);
		}
	}
	void drawLayer1() {}
	void drawLayer2() {}
	void drawDebugLayer() {}

private:
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	float barLength;
	ofRectangle rect;
};