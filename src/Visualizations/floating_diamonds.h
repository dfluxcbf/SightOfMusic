#pragma once
#include "visualization_mode.h"

class FloatingDiamonds : public VisualizationMode
{
	const float nSqrSide = 14, permanence = 200, growLevel = 2.5;
	const size_t nSquares = (int)(nSqrSide * nSqrSide + (nSqrSide - 1) * (nSqrSide - 1));
	float increase, currentSize, spacing, hue;
	size_t index;
	float* pulse;
	ofColor edgeC, fillC;
	ofRectangle rect;
	ofPoint p;

public:
	FloatingDiamonds(FftConfig* fftConfig) : VisualizationMode("FloatingDiamonds", fftConfig)
	{
		LOG("\t- Number of diamonds: %d", nSquares);
		//Settings
		_sensibility = 10;
		_dtSpeed = 8;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.5);
		configAutoCombineBands(nBands/nSquares);
		configIgnoredIndices(0, (float)(nBands - nSquares) / (float)nBands);
		configAutoRescale(1, 0, 0.5);
		configAutoDamper(1.09);

		//Initial Graphics settings
		ofBackground(0,0,0);
		ofSetLineWidth(2);
		addLayerFunction([this] { drawDefaultLayer0(); });
		_windowResized();

		//Malloc
		pulse = (float*)malloc(nBands * sizeof(float));
		memset(pulse, 0, nBands * sizeof(float));
	}

	void windowResized()
	{
		spacing = halfHeight / (nSqrSide + 1);
	}
	void update()
	{
		increase = 0;
		index = 0;
		// Pulse is the size of each square based on a single frequency
		// Changes are smoothen
		// It grows with growLevel
		// It reduces with permanence
		for (int i = 0; i < nBands; i++)
		{
			pulse[i] = growLevel * fft[i] / permanence + pulse[i] * (permanence - 1) / permanence;
		}
	}

private:
	void drawDefaultLayer0()
	{
		ofClear(0);
		edgeC.setBrightness(230); //90%
		edgeC.setSaturation(191); //75%

		// Draw all squares, line by line
		for (float i = -nSqrSide + 1; i <= nSqrSide - 1; i++)
		{
			// Draw each square in the line
			for (float j = -(increase + 1) / 2; j < (increase + 1) / 2; j++, index++) {
				currentSize = min(pulse[index]* _sensibility + 2, spacing + 2);
				p.x = halfWidth + (j + 0.5) * spacing;
				p.y = halfHeight + i * spacing;
				hue = fmodf((255 * distancef(p.x, p.y, halfWidth, halfHeight) / halfHeight + dt) , 255);
				fillC.setBrightness(200 * min(_sensibility*fft[index]/50, 1.f));
				fillC.setSaturation(191); //75%
				fillC.setHue(hue);
				edgeC.setHue(hue);

				ofPushMatrix();
				ofTranslate(p.x, p.y); // Center the square
				ofRotate(45); // Rotate the square
				rect.x = -currentSize / 2;
				rect.y = -currentSize / 2;
				rect.width = currentSize;
				rect.height = currentSize;
				DRAW_W_EDGES(fillC, edgeC,
					ofDrawRectangle(rect);
				)
				ofPopMatrix();
			}
			// Changes the number of squares per line
			// First line has 1 square, second has 3, so on.
			// After half of them is drawn, it reduces the number of squares
			if (i < 0) increase += 2;
			else increase -= 2;
		}
	}
};