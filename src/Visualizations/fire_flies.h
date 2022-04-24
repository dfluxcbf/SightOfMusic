#pragma once
#include "visualization_mode.h"

class FireFlies : public VisualizationMode
{
	static const size_t nPulses = 300;
	float fftAcc[nPulses][2];
	float pulsesR[nPulses] = { 0 };
	float speedRatio = 1;
	float avgFft = 0;

public:
	FireFlies(FftConfig* fftConfig) : VisualizationMode("FireFlies", fftConfig)
	{
		// Settings
		_sensibility = 50;
		_dtSpeed = 1;
		for (int i = 0; i < nPulses; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				fftAcc[i][j] = ofRandom(2 * PI);
			}
		}

		// FFT Post-Processing
		configIgnoredIndices(0, 0.45);
		configAutoCombineBands(nBands / (2 * nPulses));
		configAutoMinRatio();
		configAutoRescale(0.9, 0, 0.4);
		configAutoDamper(1.1);
		configAutoRescale(0.5, 0.2/nBands, 2);

		// Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetLineWidth(1);
		addLayerFunction([this] { drawLayer0(); });
		DEBUG(
			setDebugLayerFunction([this]
				{
				drawFftHistogram(0,height);
				});
		);
		_windowResized();

		post.createPass<FxaaPass>();

		const float bloomS = 0.0004;
		post.getPasses().push_back(
			shared_ptr<BloomPass>(
			new BloomPass(ofVec2f(width, height),
				false, ofVec2f(bloomS, 0.0), ofVec2f(0.0, bloomS),
				1024, true)
			));
		post.createPass<BleachBypassPass>();
	}

	void windowResized()
	{
		speedRatio = height / width;
		post.init(width, height);
	}

	void update()
	{
		static float avg = 0;
		float nFactor = 2 * ofGetFrameRate();
		// Calculates fft accumulators, phase accumulator, and pulse radius
		// Fft accumulators change the position of the points
		// Phase accumulator rotates the entire image
		avgFft = avgFft * (nFactor - 1) / nFactor + averagef(fft, nBands) / nFactor;
		for (int i = 0; i < nPulses; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				fftAcc[i][j] += (fft[i * 2 + j] - avgFft) * 1E-2;
			}
			avg = averagef(&fft[i * 2], 2);
			pulsesR[i] = 1 + sqrtf(avg) * _sensibility;
		}
	}

private:
	void drawLayer0()
	{
		post.begin();
		// Some variables
		static ofPoint position;
		static ofColor fillC = ofColor::fromHsb(0, 125, 255);
		static float phase = 0;
		static float maxPulse = 0;

		// Translate matrix
		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);

		// Every fft pulse is marked by the intersection of 2 lines.
		// The 4 reference points of these 2 lines are fft dependent.
		// Each pulse is reflected multiple times.
		phase = 0;
		maxPulse = maxf(pulsesR, nPulses);
		for (int i = nPulses-1; i >= 0; i--)
		{
			phase += 0.4;

			// Finds the first fft point (line1 point1)
			position.x = 0.92 * halfWidth * sin(phase + fftAcc[i][0] * speedRatio);
			position.y = 0.92 * halfHeight * sin(phase + fftAcc[i][1]);

			// Set Color
			fillC.setHue(fmodf(dt + 100.f * i / nPulses, 255.f));
			fillC.setBrightness(270 * sqrtf(pulsesR[i] / maxPulse));
			fillC.setSaturation(220 * (0.20 + 0.85 * sqrtf(pulsesR[i] / maxPulse)));
			ofSetColor(fillC);
			ofDrawCircle(position, pulsesR[i]);
		}
		ofPopMatrix();
		post.end();
	}
};