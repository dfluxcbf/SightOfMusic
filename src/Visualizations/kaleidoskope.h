#pragma once
#include "visualization_mode.h"

#define _DEVELOPING_ Kaleidoskope

class Kaleidoskope : public VisualizationMode
{
	float* accFft;
	ofVec3f* floatingLines;
	ofVec3f* floatingCircles;
	const float hueRange = 150;
	float sum = 0, sumAcc = 0, fLine = 0, hueShift = 0;

public:
	Kaleidoskope(FftConfig* fftConfig) : VisualizationMode("Kaleidoskope", fftConfig)
	{
		// Settings
		_sensibility = 1;
		_dtSpeed = 1;

		// FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		configAutoCombineBands(16);
		configAutoMinRatio();
		configAutoRescale(0.8, 0.1/nBands, 0.3);
		configAutoDamper(1.03);
		configAutoRescale(0.5, 0.2 / nBands, 2);

		// Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetLineWidth(2);

		addLayerFunction([this] {
			post.begin();
			drawLayer1();
			post.end(); });
		_windowResized();

		// Initial array values
		accFft = new float[nBands];
		memset(accFft, 0, nBands * sizeof(float));
		floatingLines = new ofVec3f[2 * nBands];
		floatingCircles = new ofVec3f[2 * nBands];
		for (int i = 0; i < 2 * nBands; i++)
		{
			floatingLines[i].x = 2 * width;
			floatingCircles[i].y = 2 * width;
		}

		//Configs graphics postprocessing
		post.createPass<ConvolutionPass>();
		post.getPasses().push_back(
			shared_ptr<KaleidoscopePass>(
				new KaleidoscopePass(ofVec2f(width, height),
					false, 2)
				));
		post.createPass<NoiseWarpPass>();
		post.getPasses().push_back(
			shared_ptr<KaleidoscopePass>(
				new KaleidoscopePass(ofVec2f(width, height),
					false, 2)
				));
		post.getPasses().push_back(
			shared_ptr<KaleidoscopePass>(
				new KaleidoscopePass(ofVec2f(width, height),
					false, 3)
				));
		post.createPass<RGBShiftPass>();
		post.createPass<BleachBypassPass>();
	}

	void windowResized()
	{
		post.init(width,height);
		fLine = width / 10;
	}

	void update()
	{
		// Accumulates fft over time
		for (int i = 0; i < nBands; i++)
		{
			accFft[i] += fft[i] * _sensibility / 360;
		}
		// Sum of current fft
		sum = sumf(fft, nBands);
		// Accumulator of sum over time
		sumAcc += sum / 360;
		// Hue shift over time
		hueShift += 5 / (ofGetFrameRate() + 1);
		if (hueShift > 255) hueShift = 0;

		// Calculates position of floating lines and circles
		for (int i = 0; i < 2*nBands; i++)
		{
			// Floating lines (z is line width)
			floatingLines[i].x += PI*fft[i% nBands]* _sensibility;
			if (floatingLines[i].x > halfWidth)
			{
				floatingLines[i].x = -halfWidth - fLine;
				floatingLines[i].y = ofRandomf() * halfHeight;
			}
			floatingLines[i].z = fft[i % nBands] * _sensibility * halfHeight;

			// Floating circles (z is radius)
			floatingCircles[i].y += PI*2.5 * fft[i % nBands] * _sensibility;
			floatingCircles[i].z = fft[i % nBands] * fft[i % nBands] * _sensibility * height / PI;
			if (floatingCircles[i].y > halfHeight + floatingCircles[i].z)
			{
				floatingCircles[i].y = - halfHeight - floatingCircles[i].z;
				floatingCircles[i].x = ofRandomf() * halfWidth;
			}
		}
	}

private:
	void drawLayer1()
	{
		static ofColor fillC = ofColor::fromHsb(0, 255, 255);
		
		//Center everything
		ofPushMatrix();
		ofTranslate(halfWidth, halfHeight);

		//Background circle
		ofSetColor(sum*400/nBands);
		ofDrawCircle(0, 0, height);

		//Color wheel
		ofPushMatrix();
		ofRotate(-sumAcc);
		for (int i = nBands-1; i >= 0; i--)
		{
			// Color
			fillC.setHue(fmodf(hueShift + i * hueRange / nBands,255));
			fillC.setBrightness(130 + fft[i] * 70);
			fillC.setSaturation(120 + fft[i] * 100);
			ofSetColor(fillC);

			// Triangle
			ofDrawTriangle(
				// Center
				{ 0,0 },
				// Vertice one
				{ height * sin(sqrt(accFft[i]) + i * 2 * PI / nBands),
				  height * cos(sqrt(accFft[i]) + i * 2 * PI / nBands)},
				// Next vertice
				{ height * sin(sqrt(accFft[i]) + (i + 1) * 2 * PI / nBands),
				  height * cos(sqrt(accFft[i]) + (i + 1) * 2 * PI / nBands) });
		}
		ofPopMatrix();

		// Bubbling squares
		float rSize;
		for (int i = 0; i < nBands; i++)
		{
			// Color
			fillC.setHue(fmodf(hueShift + i * hueRange / nBands, 255));
			fillC.setBrightness(150 + fft[i] * 80);
			fillC.setSaturation(170 + fft[i] * 50);
			ofSetColor(fillC);

			// Square center location
			ofPushMatrix();
			ofRotate(sumAcc + i * 360 / nBands);
			ofTranslate(-halfWidth * 2 * cos(accFft[i] * PI) / 3, 0);
			// Rotate square
			ofRotate(accFft[i]);
			// Draw square
			rSize = halfWidth * sin(accFft[i] * PI * 10) / 3.5;
			ofDrawRectRounded(ofRectangle(-rSize / 2, -rSize / 2, rSize, rSize), 5);
			ofPopMatrix();
		}

		// Draws floating lines and circles
		for (int i = 0; i < 2*nBands; i++)
		{
			//Color
			fillC.setHue(fmodf(hueShift + i * hueRange / nBands, 255));
			fillC.setBrightness(170 + 10*fft[i % nBands]);
			fillC.setSaturation(100 + 100*fft[i%nBands]);
			ofSetColor(fillC);

			// Lines
			ofSetLineWidth(floatingLines[i].z);
			ofDrawLine(floatingLines[i].x, floatingLines[i].y,
				       floatingLines[i].x + fLine, floatingLines[i].y);

			// Circles
			ofSetLineWidth(2);
			ofDrawCircle(floatingCircles[i].x, floatingCircles[i].y, floatingCircles[i].z);
		}

		// FFT histogram
		static ofVec2f p(0, 0), l(0, 0);
		// Calc initial position
		p.x = -halfWidth * 0.75 + halfHeight * sin(sumAcc) / 10;
		for (int i = 0; i < nBands; i++)
		{
			// Color
			fillC.setHue(fmodf(hueShift + i * hueRange / nBands, 255));
			fillC.setBrightness(175);
			fillC.setSaturation(230);
			ofSetColor(fillC);

			// Move to rectangle[i] position
			ofPushMatrix();
			ofTranslate(p);
			// Calc width and height based on fft[i] value
			l.x = fft[i] * halfWidth / sum / 0.75;
			l.y = fft[i] * 1.25*height;
			// Draw rect
			ofDrawRectangle(0, -l.y / 2, l.x, l.y);
			// Calc next rectangle position
			p.x += l.x;
			ofPopMatrix();
		}
		ofPopMatrix();
	}
};