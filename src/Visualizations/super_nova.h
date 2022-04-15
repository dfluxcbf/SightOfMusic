#pragma once
#include "visualization_mode.h"

class SuperNova : public VisualizationMode
{
public:
	SuperNova(FftConfig* fftConfig) : VisualizationMode("SuperNova", fftConfig, 1)
	{
		_sensibility = 300;
		_dtSpeed = 6;
		configIgnoredIndices(0, 0.5);
		configAutoDamper(DAMP);
		configAutoRescale(0.8, 0.002, 0.3);
		configAutoCombineBands(4);
		ofSetLineWidth(3);
		ofBackground(0, 0, 0);
		_windowResized();
		sparks = (float*)malloc(N_SPARKS * nBands * sizeof(float));
		memset(sparks, 0, N_SPARKS * nBands * sizeof(float));
	}
	~SuperNova()
	{
		free(sparks);
	}

	void windowResized() override
	{
		barLength = height / 18;
		sparkSize = 2.5 * barLength / N_SPARKS;
	}
	void keyPressed(int key) {}
	void keyReleased(int key) {}

	void update() {}
	void drawDefaultLayer0()
	{
		ofClear(0);
		float lineSize, sparkPosition;
		float hue_;
		float angle;
		float sin_, cos_;
		dampSparks();
		hue_ = fmodf(dt, 255);
		for (int i = 0; i < nBands_combined; i++)
		{
			barColor.setHue(hue_);
			barColor.setSaturation(200);
			barColor.setBrightness(225);
			ofSetColor(barColor);
			lineSize = _sensibility * _combinedFft[i];
			angle = PI / 2 + (i+ 0.5) * PI / (nBands_combined -1);
			sin_ = sinf(angle);
			cos_ = cosf(angle);
			ofDrawLine(
				halfWidth,
				halfHeight,
				halfWidth + (lineSize + barLength) * sin_,
				halfHeight + (lineSize + barLength) * cos_
			);
			angle = PI / 2 - (i + 0.5) * PI / (nBands_combined - 1);
			sin_ = sinf(angle);
			cos_ = cosf(angle);
			ofDrawLine(
				halfWidth,
				halfHeight,
				halfWidth + (lineSize + barLength) * sin_,
				halfHeight + (lineSize + barLength) * cos_
			);
			for (int j = 0; j < N_SPARKS; j++)
			{
				barColor.setHue(hue_);
				barColor.setSaturation(140 * (j+1) / N_SPARKS);
				barColor.setBrightness(50+50 * (j+1) / N_SPARKS);
				ofSetColor(barColor);
				sparkPosition = lineSize + 50 * j;
				angle = PI / 2 + (i + 0.5) * PI / (nBands_combined - 1);
				sin_ = sinf(angle);
				cos_ = cosf(angle);
				ofDrawLine(
					halfWidth + (barLength + j * sparkSize + sparks[i * N_SPARKS + j]) * sin_,
					halfHeight + (barLength + j * sparkSize + sparks[i * N_SPARKS + j]) * cos_,
					halfWidth + (barLength + (j + 1) * sparkSize + sparks[i * N_SPARKS + j]) * sin_,
					halfHeight + (barLength + (j + 1) * sparkSize + sparks[i * N_SPARKS + j]) * cos_
				);
				angle = PI / 2 - (i + 0.5) * PI / (nBands_combined - 1);
				sin_ = sinf(angle);
				cos_ = cosf(angle);
				ofDrawLine(
					halfWidth + (barLength + j * sparkSize + sparks[i * N_SPARKS + j]) * sin_,
					halfHeight + (barLength + j * sparkSize + sparks[i * N_SPARKS + j]) * cos_,
					halfWidth + (barLength + (j + 1) * sparkSize + sparks[i * N_SPARKS + j]) * sin_,
					halfHeight + (barLength + (j + 1) * sparkSize + sparks[i * N_SPARKS + j]) * cos_
				);
			}
		}
	}
	void drawLayer1() {}
	void drawLayer2() {}
	void drawDebugLayer() {}

private:
	ofColor barColor = ofColor::fromHsb(0, 255, 255);
	float barLength;
	float* sparks;
	float sparkSize;
	const float DAMP = 1.0175;
	const size_t N_SPARKS = 7;

	void dampSparks()
	{
		float damp = DAMP;
		float dampIncrement = 8;
		for (int i = 0; i < nBands_combined; i++)
		{
			damp = 1 + (DAMP - 1) / (dampIncrement + 1);
			sparks[i * N_SPARKS] /= fpsRelativeDamp(damp);
			if (sparks[i * N_SPARKS] < _sensibility * _combinedFft[i]) sparks[i * N_SPARKS] = _sensibility * _combinedFft[i];
			for (int j = 1; j < N_SPARKS; j++)
			{
				damp = 1 + (DAMP - 1) / (1 + dampIncrement * (j + 1));
				sparks[i * N_SPARKS + j] /= fpsRelativeDamp(damp);
				if (sparks[i * N_SPARKS + j] < sparks[i * N_SPARKS + j - 1]) sparks[i * N_SPARKS + j] = sparks[i * N_SPARKS + j - 1];

			}
		}
	}

};