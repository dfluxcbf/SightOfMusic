#pragma once
#include "visualization_mode.h"

class SuperNova : public VisualizationMode
{
	const float initDamp = 1.04;
	const size_t nSparks = 7;
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	float* sparks;
	float barLength, sparkSize, damp, dampIncrement,
		  lineSize, sparkPosition,
		  hue_, angle, sin_, cos_;

public:
	SuperNova(FftConfig* fftConfig) : VisualizationMode("SuperNova", fftConfig)
	{
		//Settings
		_sensibility = 300;
		_dtSpeed = 6;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		configAutoCombineBands(4);
		configAutoDamper(initDamp);
		configAutoRescale(0.8, 0.002, 0.3);
		ofSetLineWidth(3);
		ofBackground(0, 0, 0);
		_windowResized();
		addLayerFunction([this] { drawDefaultLayer0(); });

		//Malloc
		sparks = (float*)malloc(nSparks * nBands * sizeof(float));
		memset(sparks, 0, nSparks * nBands * sizeof(float));
	}
	~SuperNova()
	{
		free(sparks);
	}

	void windowResized() override
	{
		barLength = height / 18;
		sparkSize = 2.5 * barLength / nSparks;
	}
	void update()
	{
		dampSparks();
	}

private:
	void drawDefaultLayer0()
	{
		ofClear(0);
		hue_ = fmodf(dt, 255);
		// Draws center explosion
		for (int i = 0; i < nBands; i++)
		{
			barColor.setHue(hue_);
			barColor.setSaturation(200);
			barColor.setBrightness(225);
			ofSetColor(barColor);

			// Explosion size
			lineSize = _sensibility * fft[i];
			
			// Top lines
			angle = PI / 2 + (i + 0.5) * PI / (nBands - 1);
			sin_ = sinf(angle);
			cos_ = cosf(angle);
			// Draw
			ofDrawLine(
				halfWidth,
				halfHeight,
				halfWidth + (lineSize + barLength) * sin_,
				halfHeight + (lineSize + barLength) * cos_
			);

			// Bottom lines
			angle = PI / 2 - (i + 0.5) * PI / (nBands - 1);
			sin_ = sinf(angle);
			cos_ = cosf(angle);
			// Draw
			ofDrawLine(
				halfWidth,
				halfHeight,
				halfWidth + (lineSize + barLength) * sin_,
				halfHeight + (lineSize + barLength) * cos_
			);
			// Draws each spark
			for (int j = 0; j < nSparks; j++)
			{
				barColor.setHue(hue_);
				barColor.setSaturation(140 * (j + 1) / nSparks);
				barColor.setBrightness(50 + 50 * (j + 1) / nSparks);
				ofSetColor(barColor);

				// Distance to center
				sparkPosition = lineSize + 50 * j;

				// Top sparks
				// Angle position
				angle = PI / 2 + (i + 0.5) * PI / (nBands - 1);
				sin_ = sinf(angle);
				cos_ = cosf(angle);
				// Draw
				ofDrawLine(
					halfWidth + (barLength + j * sparkSize + sparks[i * nSparks + j]) * sin_,
					halfHeight + (barLength + j * sparkSize + sparks[i * nSparks + j]) * cos_,
					halfWidth + (barLength + (j + 1) * sparkSize + sparks[i * nSparks + j]) * sin_,
					halfHeight + (barLength + (j + 1) * sparkSize + sparks[i * nSparks + j]) * cos_
				);
				
				// Bottom sparks
				angle = PI / 2 - (i + 0.5) * PI / (nBands - 1);
				sin_ = sinf(angle);
				cos_ = cosf(angle);
				// Draw
				ofDrawLine(
					halfWidth + (barLength + j * sparkSize + sparks[i * nSparks + j]) * sin_,
					halfHeight + (barLength + j * sparkSize + sparks[i * nSparks + j]) * cos_,
					halfWidth + (barLength + (j + 1) * sparkSize + sparks[i * nSparks + j]) * sin_,
					halfHeight + (barLength + (j + 1) * sparkSize + sparks[i * nSparks + j]) * cos_
				);
			}
		}
	}
	void dampSparks()
	{
		// Damps each spark and checks for collision
		damp = initDamp;
		dampIncrement = 8;
		for (int i = 0; i < nBands; i++)
		{
			// Fist spark that reacts to the center explosion
			damp = 1 + (initDamp - 1) / (dampIncrement + 1);
			sparks[i * nSparks] /= fpsRelativeDamp(damp);
			// Checks collision
			if (sparks[i * nSparks] < _sensibility * fft[i])
				sparks[i * nSparks] = _sensibility * fft[i];
			
			// Other sparks that react to other sparks
			for (int j = 1; j < nSparks; j++)
			{
				damp = 1 + (initDamp - 1) / (1 + dampIncrement * (j + 1));
				sparks[i * nSparks + j] /= fpsRelativeDamp(damp);
				// Checks collision
				if (sparks[i * nSparks + j] < sparks[i * nSparks + j - 1])
					sparks[i * nSparks + j] = sparks[i * nSparks + j - 1];

			}
		}
	}

};