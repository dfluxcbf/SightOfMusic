#pragma once
#include "visualization_mode.h"

class DancingWaves : public VisualizationMode
{
	const size_t nWaves = 50;
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	float* waves;
	float wavesSize, smallRadius, waveDistance, hue_, barSize, angle;
	ofPolyline polyline;
	ofPoint p;

public:
	DancingWaves(FftConfig* fftConfig) : VisualizationMode("DacingWaves", fftConfig)
	{
		//Settings
		_sensibility = 50;
		_dtSpeed = 20;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		configAutoCombineBands(8);
		configAutoDamper(1.04);
		configAutoRescale(0.5, 0.025, 0.9);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofEnableAlphaBlending();
		ofSetFrameRate(30);
		ofSetLineWidth(1);
		barColor.setSaturation(255);
		barColor.setBrightness(255);
		_windowResized();
		addLayerFunction([&] { drawDefaultLayer0(); });
		
		//Malloc
		waves = (float*)malloc(nWaves * nBands * sizeof(float));
		memset(waves, 0, nWaves * nBands * sizeof(float));
	}
	~DancingWaves()
	{
		free(waves);
	}

	void windowResized()
	{
		smallRadius = halfHeight / 10;
		waveDistance = (height - smallRadius * 2) / (3 * nWaves);
	}
	void update()
	{
		dampWaves();
	}

	void drawDefaultLayer0()
	{
		ofClear(0);
		barColor.setSaturation(125);

		// Draws all waves
		for (int j = nWaves-1; j >= 0; j--)
		{
			hue_ = fmodf(dt + j, 255);
			barColor.setHue(hue_);
			angle = PI;
			p.z = 0;
			polyline.clear();
			polyline.setClosed(true);
			angle -= PI / (nBands - 1) / 2;
			barColor.setBrightness(255*(nWaves - j + 2) / nWaves);
			ofSetColor(barColor,255);
			// Right vertices
			for (int i = 0; i < nBands; i++)
			{
				p.x = halfWidth + (smallRadius + waveDistance * j + waves[i + nBands * j] * _sensibility) * sinf(angle);
				p.y = halfHeight + (smallRadius + waveDistance * j + waves[i + nBands * j] * _sensibility) * cosf(angle);
				polyline.curveTo(p);
				angle += PI / (nBands - 1);
			}
			// Left vertices
			for (int i = nBands - 1; i >= 0; i--)
			{
				p.x = halfWidth + (smallRadius + waveDistance * j + waves[i + nBands * j] * _sensibility) * sinf(angle);
				p.y = halfHeight + (smallRadius + waveDistance * j + waves[i + nBands * j] * _sensibility) * cosf(angle);
				polyline.curveTo(p);
				angle += PI / (nBands - 1);
			}
			polyline.draw();
		}
		// Center area (filled with alpha color)
		angle = PI;
		angle -= PI / (nBands - 1) / 2;
		ofSetColor(barColor, 200);
		ofBeginShape();
		// Right vertices
		for (int i = 0; i < nBands; i++)
		{
			p.x = halfWidth + (smallRadius + fft[i] * _sensibility) * sinf(angle);
			p.y = halfHeight + (smallRadius + fft[i] * _sensibility) * cosf(angle);
			ofCurveVertex(p.x, p.y);
			angle += PI / (nBands - 1);
		}
		// Left vertices
		for (int i = nBands - 1; i >= 0; i--)
		{
			p.x = halfWidth + (smallRadius + fft[i] * _sensibility) * sinf(angle);
			p.y = halfHeight + (smallRadius + fft[i] * _sensibility) * cosf(angle);
			ofCurveVertex(p.x, p.y);
			angle += PI / (nBands - 1);
		}
		ofEndShape();
	}

private:
	void dampWaves()
	{
		// Moves the bands to the following position
		for (int i = nWaves - 1; i > 0; i--)
		{
			memcpy(waves + i * nBands, waves + (i - 1) * nBands, nBands * sizeof(float));
		}
		memcpy(waves, fft, nBands * sizeof(float));
	}
};