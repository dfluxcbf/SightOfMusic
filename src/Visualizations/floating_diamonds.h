#pragma once
#include "visualization_mode.h"

class FloatingDiamonds : public VisualizationMode
{
private:
	float increase, currentSize, spacing, hue, x, y;
	const float nSqrSide = 10;
	const size_t nSquares = (int)(nSqrSide * nSqrSide + (nSqrSide - 1) * (nSqrSide - 1));
	size_t index;
	float* pulse;
	const float permanence = 200;
	const float growLevel = 1.2;
	ofColor edgeC, fillC;
	ofRectangle rect;
public:
	FloatingDiamonds(FftConfig* fftConfig) : VisualizationMode("FloatingDiamonds", fftConfig, 1)
	{
		_sensibility = 15;
		_dtSpeed = 5;
		configIgnoredIndices(0, 0.4);
		configAutoDamper(1.09);
		configAutoRescale(1, -1.f / (nBands * 0.98), 0.4);
		ofBackground(0,0,0);
		ofSetLineWidth(2);
		_windowResized();
		pulse = (float*)malloc(nBands * sizeof(float));
		memset(pulse, 0, nBands * sizeof(float));
		fillC.setSaturation(191); //75%
		edgeC.setSaturation(191); //75%
		edgeC.setBrightness(230); //90%
	}

	void windowResized()
	{
		spacing = halfHeight / (nSqrSide + 1);
	}
	void keyPressed(int key) {}
	void keyReleased(int key) {}
	
	void update()
	{
		increase = 0;
		index = 0;
		for (int i = 0; i < nBands; i++)
		{
			pulse[i] = growLevel * _fft[i] / permanence + pulse[i] * (permanence - 1) / permanence;
		}
	}


	void drawDefaultLayer0()
	{
		ofClear(0);
		for (float i = -nSqrSide + 1; i <= nSqrSide - 1; i++)
		{
			for (float j = -(increase + 1) / 2; j < (increase + 1) / 2; j++) {
				currentSize = min(pulse[index]* _sensibility + 2, spacing + 2);
				x = halfWidth + (j + 0.5) * spacing;
				y = halfHeight + i * spacing;
				hue = fmodf((255 * distancef(x, y, halfWidth, halfHeight) / halfHeight + dt) , 255);
				fillC.setHue(hue);
				fillC.setBrightness(200*min(_fft[index], spacing));
				edgeC.setHue(hue);

				ofPushMatrix();
				ofTranslate(x, y);
				ofRotate(45);

				rect.x = -currentSize / 2;
				rect.y = -currentSize / 2;
				rect.width = currentSize;
				rect.height = currentSize;
				DRAW_W_EDGES(fillC, edgeC,
					ofDrawRectangle(rect);
				)
				ofPopMatrix();
				index++;
			}

			if (i < 0) increase += 2;
			else increase -= 2;
		}
	}
	void drawLayer1() {}
	void drawLayer2() {}
	void drawDebugLayer() {}
};