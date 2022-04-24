#pragma once
#include "visualization_mode.h"

// It has to be a bit less then 45 degrees.
// The point will move from -ANGLE_RANGE to +ANGLE_RANGE, 
//so a bit less then 90 degrees 
// 45 degrees must be avoided se there is no parallel lines
#define ANGLE_RANGE PI * 0.5 * 0.5 * 0.98

class Five5 : public VisualizationMode
{
	const size_t nReflections = 5;
	const float reflectionAngle = 360.f / (float)nReflections;
	static const size_t nPulses = 25;
	static const size_t nRefPoints = 4;
	float fftAcc[nPulses][nRefPoints] = { 0 };
	float pulsesR[nPulses] = { 0 };
	float phaseAcc = 0;
	float boundRadius = 0;

public:
	Five5(FftConfig* fftConfig) : VisualizationMode("Five5", fftConfig)
	{
		// Settings
		_sensibility = 50;
		_dtSpeed = 1;

		// FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		configAutoCombineBands(nBands / (nRefPoints * nPulses));
		configAutoMinRatio();
		configAutoRescale(1, 0, 0.5);
		configAutoDamper(1.1);
		configAutoRescale(0.5, 0.2/nBands, 2);

		// Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetLineWidth(2);
		addLayerFunction([this] { drawLayer1(); });
		DEBUG(
			setDebugLayerFunction([this] {drawDebugLayer(); });
		);
		_windowResized();
	}

	void windowResized()
	{
		boundRadius = halfHeight * 0.95;
	}

	void update()
	{
		static float avg = 0;
		// Calculates fft accumulators, phase accumulator, and pulse radius
		// Fft accumulators change the position of the points
		// Phase accumulator rotates the entire image
		for (int i = 0; i < nPulses; i++)
		{
			for (int j = 0; j < nRefPoints; j++)
			{
				fftAcc[i][j] +=
					sqrt(fft[i * nRefPoints + j]) / 150;
			}
			avg = averagef(&fft[i * nRefPoints], nRefPoints);
			pulsesR[i] = 1 + sqrt(avg) * _sensibility;
		}
		phaseAcc -= 75 * _sensibility * averagef(fft, nBands) / powf(nBands,2);
		avg = height * 0.95 * averagef(fft, nBands) * 2;
	}

private:
	void drawLayer1()
	{
		// Some variables
		static ofPoint inter;
		static Line line1, line2;
		static ofColor fillC = ofColor::fromHsb(0, 125, 255);
		static float movement = 0;
		static float phase = 0;
		static float maxPulse = 0;
		// Fading
		ofFill();
		ofSetColor(0, 0, 0, 5);
		ofDrawRectangle(0, 0, width, height);

		// Translate matrix
		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);
		ofRotate(phaseAcc);

		// Every fft pulse is marked by the intersection of 2 lines.
		// The 4 reference points of these 2 lines are fft dependent.
		// Each pulse is reflected multiple times.
		ofNoFill();
		phase = 0;
		maxPulse = maxf(pulsesR, nPulses);
		for (int i = nPulses-1; i >= 0; i--)
		{
			phase += 0.4;

			// Finds the first fft point (line1 point1)
			movement = ANGLE_RANGE * sin(fftAcc[i][0]);
			line1.p1 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			phase += PI / 2; // Moves 90 degrees

			// Finds the second fft point (line2 point1)
			movement = ANGLE_RANGE * sin(fftAcc[i][1]);
			line2.p1 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			phase += PI / 2; // Moves 90 degrees

			// Finds the third fft point (line1 point2)
			movement = ANGLE_RANGE * sin(fftAcc[i][2]);
			line1.p2 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			phase += PI / 2; // Moves 90 degrees

			// Finds the forth fft point (line2 point2)
			movement = ANGLE_RANGE * sin(fftAcc[i][3]);
			line2.p2 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));

			// Finds the intersection of line1 and line2
			inter = intersection(line1, line2);

			// Set Color
			fillC.setHue(fmodf(dt + 1000.f * i / nPulses, 255.f));
			fillC.setBrightness(255 * 1.15 * (pulsesR[i] / maxPulse));
			fillC.setSaturation(160 * 1.05 * sqrtf(pulsesR[i] / maxPulse));
			ofSetColor(fillC);

			// Repeats itself nReflections times
			ofSetCircleResolution(5);
			for (int j = 0; j < nReflections; j++)
			{
				//ofDrawCircle(inter, pulsesR[i]);
				ofPushMatrix();
				ofTranslate(inter.x, inter.y);
				ofRotate(sumf(fftAcc[i],4) * 20);
				ofDrawCircle(0,0, pulsesR[i]);
				ofPopMatrix();
				ofRotate(reflectionAngle);
			}
		}
		ofPopMatrix();
	}

	void drawDebugLayer()
	{
		static ofPoint inter;
		static Line line1, line2, boundLine1, boundLine2, boundLine3, boundLine4;
		static ofColor fillC = ofColor::fromHsb(0, 125, 255);
		static float movement = 0;
		static float phase = 0;
		static int pulseDebug = 0;
		static std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count() > 20)
		{
			pulseDebug++;
			pulseDebug %= nPulses;
			begin = std::chrono::steady_clock::now();
		}

		ofClear(0);

		// Translate matrix
		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);
		ofRotate(phaseAcc);

		// Same calculation, without reflection
		phase = 0;
		for (int i = nPulses-1; i >= 0; i--)
		{
			phase += 0.4;

			movement = ANGLE_RANGE * sin(fftAcc[i][0]);
			line1.p1 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			boundLine1.p1 = ofPoint(boundRadius * sin(phase + ANGLE_RANGE), boundRadius * cos(phase + ANGLE_RANGE));
			boundLine3.p1 = ofPoint(boundRadius * sin(phase - ANGLE_RANGE), boundRadius * cos(phase - ANGLE_RANGE));
			phase += PI / 2;

			movement = ANGLE_RANGE * sin(fftAcc[i][1]);
			line2.p1 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			boundLine2.p1 = ofPoint(boundRadius * sin(phase + ANGLE_RANGE), boundRadius * cos(phase + ANGLE_RANGE));
			boundLine4.p1 = ofPoint(boundRadius * sin(phase - ANGLE_RANGE), boundRadius * cos(phase - ANGLE_RANGE));
			phase += PI / 2;

			movement = ANGLE_RANGE * sin(fftAcc[i][2]);
			line1.p2 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			boundLine1.p2 = ofPoint(boundRadius * sin(phase + ANGLE_RANGE), boundRadius * cos(phase + ANGLE_RANGE));
			boundLine3.p2 = ofPoint(boundRadius * sin(phase - ANGLE_RANGE), boundRadius * cos(phase - ANGLE_RANGE));
			phase += PI / 2;

			movement = ANGLE_RANGE * sin(fftAcc[i][3]);
			line2.p2 = ofPoint(boundRadius * sin(phase + movement), boundRadius * cos(phase + movement));
			boundLine2.p2 = ofPoint(boundRadius * sin(phase + ANGLE_RANGE), boundRadius * cos(phase + ANGLE_RANGE));
			boundLine4.p2 = ofPoint(boundRadius * sin(phase - ANGLE_RANGE), boundRadius * cos(phase - ANGLE_RANGE));

			// Draws only one pulse at a time.
			if (i != pulseDebug) continue;

			// Reference circle, where reference points are located
			ofNoFill();
			ofSetColor(255, 255, 255);
			ofDrawCircle(0, 0, boundRadius);

			// Draws line1 references
			ofFill();
			ofSetColor(255, 0, 0);
			ofDrawCircle(line1.p1, 20);
			ofDrawCircle(line1.p2, 20);
			ofDrawLine(line1.p1, line1.p2);

			// Draws line2 references
			ofSetColor(0, 255, 0);
			ofDrawCircle(line2.p1, 20);
			ofDrawCircle(line2.p2, 20);
			ofDrawLine(line2.p1, line2.p2);

			// Draws reference position limits on circle
			ofSetColor(255, 0, 0);
			ofDrawCircle(boundLine1.p1, 10);
			ofDrawCircle(boundLine1.p2, 10);
			ofSetColor(0, 255, 0);
			ofDrawCircle(boundLine2.p1, 10);
			ofDrawCircle(boundLine2.p2, 10);
			ofSetColor(255, 0, 0);
			ofDrawCircle(boundLine3.p1, 10);
			ofDrawCircle(boundLine3.p2, 10);
			ofSetColor(0, 255, 0);
			ofDrawCircle(boundLine4.p1, 10);
			ofDrawCircle(boundLine4.p2, 10);

			// Draw pulse location
			inter = intersection(line1, line2);
			ofSetColor(255, 255, 255);
			ofDrawCircle(inter, pulsesR[i]);
			ofSetColor(0, 0, 0);
			ofDrawCircle(inter, pulsesR[i] * 2 / 3);
			ofSetColor(255, 255, 255);
			ofDrawCircle(inter, pulsesR[i] / 3);
		}
		ofPopMatrix();
	}
};