#pragma once
#include "visualization_mode.h"

class DancingWaves : public VisualizationMode
{
	const size_t N_WAVES = 50;
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	float* waves;
	float wavesSize, smallRadius, waveDistance;

public:
	DancingWaves(FftConfig* fftConfig) : VisualizationMode("DacingWaves", fftConfig)
	{
		_sensibility = 50;
		_dtSpeed = 2,0;
		configIgnoredIndices(0, 0.5);
		configAutoDamper(1.04);
		configAutoRescale(0.5, 0.025, 0.9);
		ofBackground(0, 0, 0);
		configAutoCombineBands(8);
		ofEnableAlphaBlending();
		ofSetFrameRate(30);
		ofSetLineWidth(1);
		barColor.setSaturation(255);
		barColor.setBrightness(255);
		_windowResized();
		waves = (float*)malloc(N_WAVES * nBands_combined * sizeof(float));
		memset(waves, 0, N_WAVES * nBands_combined * sizeof(float));
		addLayerFunction([&] { drawDefaultLayer0(); });
	}
	~DancingWaves()
	{
		free(waves);
	}

	void windowResized()
	{
		smallRadius = halfHeight / 10;
		waveDistance = (height - smallRadius * 2) / (3 * N_WAVES);
	}
	void keyPressed(int key) {}
	void keyReleased(int key) {}
	void update() {}

	void drawDefaultLayer0()
	{
		ofClear(0);
		dampWaves();
		float hue_;
		float barSize;
		barColor.setSaturation(125);
		ofPolyline polyline;
		ofPoint p;
		float angle = PI;
		for (int j = N_WAVES-1; j >= 0; j--)
		{
			hue_ = fmodf(dt + j, 255);
			barColor.setHue(hue_);
			angle = PI;
			p.z = 0;
			polyline.clear();
			polyline.setClosed(true);
			angle -= PI / (nBands_combined - 1) / 2;
			barColor.setBrightness(255*(N_WAVES - j + 2) / N_WAVES);
			ofSetColor(barColor,255);
			for (int i = 0; i < nBands_combined; i++)
			{
				p.x = halfWidth + (smallRadius + waveDistance * j + waves[i + nBands_combined * j] * _sensibility) * sinf(angle);
				p.y = halfHeight + (smallRadius + waveDistance * j + waves[i + nBands_combined * j] * _sensibility) * cosf(angle);
				polyline.curveTo(p);
				angle += PI / (nBands_combined - 1);
			}
			for (int i = nBands_combined - 1; i >= 0; i--)
			{
				p.x = halfWidth + (smallRadius + waveDistance * j + waves[i + nBands_combined * j] * _sensibility) * sinf(angle);
				p.y = halfHeight + (smallRadius + waveDistance * j + waves[i + nBands_combined * j] * _sensibility) * cosf(angle);
				polyline.curveTo(p);
				angle += PI / (nBands_combined - 1);
			}
			polyline.draw();
		}
		angle = PI;
		angle -= PI / (nBands_combined - 1) / 2;
		ofSetColor(barColor, 120);
		ofBeginShape();
		for (int i = 0; i < nBands_combined; i++)
		{
			p.x = halfWidth + (smallRadius + _combinedFft[i] * _sensibility) * sinf(angle);
			p.y = halfHeight + (smallRadius + _combinedFft[i] * _sensibility) * cosf(angle);
			ofCurveVertex(p.x, p.y);
			angle += PI / (nBands_combined - 1);
		}
		for (int i = nBands_combined - 1; i >= 0; i--)
		{
			p.x = halfWidth + (smallRadius + _combinedFft[i] * _sensibility) * sinf(angle);
			p.y = halfHeight + (smallRadius + _combinedFft[i] * _sensibility) * cosf(angle);
			ofCurveVertex(p.x, p.y);
			angle += PI / (nBands_combined - 1);
		}
		ofEndShape();
	}

private:
	void dampWaves()
	{
		for (int i = N_WAVES - 1; i > 0; i--)
		{
			memcpy(waves + i * nBands_combined, waves + (i - 1) * nBands_combined, nBands_combined * sizeof(float));
		}
		memcpy(waves, _combinedFft, nBands_combined * sizeof(float));
	}
};