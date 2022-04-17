#pragma once
#include "visualization_mode.h"

class BarsReflection : public VisualizationMode
{
	ofColor barColor = ofColor::fromHsb(0, 255, 255),
		    edgeColor = ofColor(0, 0, 0);
	float reflectHeight = 0, barLength = 0, barSize = 0, hue_ = 0;
	ofRectangle rect;

public:
	BarsReflection(FftConfig* fftConfig) : VisualizationMode("BarsReflection", fftConfig)
	{
		//Settings
		_sensibility = 900;
		_dtSpeed = 5;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.4);
		configAutoCombineBands(4);
		configAutoDamper(1.04);
		configAutoRescale(1, 0.002, 0.5);

		//Initial Graphics settings
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
		barLength = halfWidth / nBands;
		rect.width = barLength;
	}
	void update() {}

private:
	void drawDefaultLayer0()
	{
		// Clear bars area
		ofSetColor(0, 0, 0);
		ofDrawRectangle(0, 0, width, reflectHeight);
		
		// Draws bars and reflections
		ofSetLineWidth(1);
		for (int i = 0; i < nBands; i++)
		{
			barSize = 10 + _sensibility * fft[i];
			hue_ = dt + 510.0 * (float)i / (float)nBands;
			hue_ = fmodf(hue_, 255);
			barColor.setHue(hue_);

			// Left bars
			barColor.setSaturation(100);
			barColor.setBrightness(180);
			rect.x = halfWidth - (i + 0.5) * barLength;
			rect.height = -barSize / 4;
			rect.y = reflectHeight - barSize / 4;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			);

			// Right bars
			barColor.setSaturation(100);
			barColor.setBrightness(180);
			rect.x = halfWidth + (i + 0.5) * barLength;
			rect.height = -barSize / 4;
			rect.y = reflectHeight - barSize / 4;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			);

			// Left reflections
			barColor.setSaturation(80);
			barColor.setBrightness(50);
			rect.x = halfWidth - (i+0.5) * barLength;
			rect.height = barSize / 12;
			rect.y = reflectHeight + barSize / 12;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			);

			// Right reflections
			barColor.setSaturation(80);
			barColor.setBrightness(50);
			rect.x = halfWidth + (i + 0.5) * barLength;
			rect.height = barSize / 12;
			rect.y = reflectHeight + barSize / 12;
			DRAW_W_EDGES(barColor, edgeColor,
				ofDrawRectangle(rect);
			);
		}

		// Fades reflection area
		ofSetColor(0, 0, 0, 300.f / ofGetFrameRate());
		ofDrawRectangle(0, reflectHeight, width, height - reflectHeight);

		// Separating line
		hue_ = fmodf(dt, 255);
		barColor.setHue(hue_);
		barColor.setBrightness(50);
		barColor.setSaturation(50);
		ofSetColor(barColor);
		ofSetLineWidth(2);
		ofLine(0, reflectHeight, width, reflectHeight);
	}
};