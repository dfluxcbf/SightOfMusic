#pragma once
#include "visualization_mode.h"

class BarsReflection : public VisualizationMode
{
	ofColor barColor = ofColor::fromHsb(0, 255, 255),
		    edgeColor = ofColor(0, 0, 0);
	float reflectHeight, barLength;
	ofRectangle rect;

public:
	BarsReflection(FftConfig* fftConfig) : VisualizationMode("BarsReflection", fftConfig)
	{
		_sensibility = 900;
		_dtSpeed = 5;
		configIgnoredIndices(0, 0.4);
		configAutoDamper(1.04);
		configAutoRescale(1, 0.002, 0.5);
		configAutoCombineBands(4);
		ofBackground(0, 0, 0);
		ofSetLineWidth(1);
		barColor.setSaturation(100);
		barColor.setBrightness(180);
		_windowResized();
		addLayerFunction([&] { drawDefaultLayer0(); });
	}

	void windowResized()
	{
		reflectHeight = halfHeight * 1.4;
		barLength = 4 * halfWidth / nBands;
		rect.width = barLength;
	}
	void keyPressed(int key) {}
	void keyReleased(int key) {}
	void update() {}

private:
	void drawDefaultLayer0()
	{
		ofClear(0);
		float barSize;
		float hue_;
		for (int i = 0; i < nBands_combined; i++)
		{
			barSize = 10 + _sensibility * _combinedFft[i];
			
			hue_ = dt + 510.0 * (float)i / (float)nBands;
			hue_ = fmodf(hue_, 255);

			barColor.setHue(hue_);

			barColor.setSaturation(80);
			barColor.setBrightness(50);
			rect.x = halfWidth + (i+0.5) * barLength;
			rect.height = barSize / 12;
			rect.y = reflectHeight + barSize/12;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			)

			barColor.setSaturation(100);
			barColor.setBrightness(180);
			rect.height = -barSize/4;
			rect.y = reflectHeight - barSize / 4;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			)

			barColor.setSaturation(80);
			barColor.setBrightness(50);
			rect.x = halfWidth - (i+0.5) * barLength;
			rect.height = barSize / 12;
			rect.y = reflectHeight + barSize / 12;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			)

			barColor.setSaturation(100);
			barColor.setBrightness(180);
			rect.height = -barSize/4;
			rect.y = reflectHeight - barSize / 4;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			)
		}
		hue_ = fmodf(dt, 255);
		barColor.setHue(hue_);
		barColor.setBrightness(50);
		barColor.setSaturation(50);
		ofSetColor(barColor);
		ofSetLineWidth(2);
		ofLine(0, reflectHeight, width, reflectHeight);
		ofSetLineWidth(1);
	}
};