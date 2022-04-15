#pragma once
#include "visualization_mode.h"

//#define _TEST_MODE_

class TEST_MODE : public VisualizationMode
{
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	float barLength;
	ofRectangle rect;
	ofPolyline polyline;
	ofPoint elementPosition;
public:
	TEST_MODE(FftConfig* fftConfig) : VisualizationMode("TEST", fftConfig)
	{
		_sensibility = 70;
		_dtSpeed = 5;
		configIgnoredIndices(0, 0.4);
		configAutoDamper(1.1);
		ofBackground(0, 0, 0);
		configAutoCombineBands(4);
		barColor.setSaturation(255);
		barColor.setBrightness(255);
		_windowResized();
		addLayerFunction([&] { drawDefaultLayer0(); });
	}
	void deconstruct() {}
	void update() {}

private:
	void drawDefaultLayer0()
	{
		ofClear(0);
		float barSize;
		float hue_;
		for (int i = 0; i < nBands_combined; i++)
		{
			barSize = 1 + _sensibility * _combinedFft[i];
			hue_ = dt + 255.0 * (float)i / (float)nBands_combined;
			hue_ = fmodf(hue_, 255);

			barColor.setHue(hue_);
			ofSetColor(barColor);

			rect.x = halfWidth + i * barLength;
			rect.height = barSize;
			ofDrawRectangle(rect);;
		}


		// Push vertices to polyline
		polyline.clear();
		ofBeginShape();
		float angle = 0;
		for (int i = 0; i <= nBands_combined; i++)
		{
			angle += 2.f * PI / nBands_combined;
			ofPoint p;
			p.x = 200 + 100 * cosf(angle) + _combinedFft[i % nBands_combined] * cosf(angle) * _sensibility;
			p.y = 200 + 100 * sinf(angle) + _combinedFft[i % nBands_combined] * sinf(angle) * _sensibility;
			p.z = 0;
			ofCurveVertex(p.x, p.y);

			p.x += 50 * cosf(angle);
			p.y += 50 * sinf(angle);
			polyline.curveTo(p);
		}
		ofEndShape();
		polyline.draw();
	}
	void windowResized()
	{
		barLength = halfWidth / nBands;
		rect.width = barLength;
		rect.y = halfHeight;
	}
};