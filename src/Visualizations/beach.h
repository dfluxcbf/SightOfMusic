#pragma once
#include "visualization_mode.h"

class Beach : public VisualizationMode
{
	const ofColor sandColor = ofColor(240, 221, 173);
	const ofColor waterColor = ofColor(3, 157, 255);
	const ofColor waterEdge = ofColor(179, 240, 242);
	const size_t nWaves = 20;
	float waveStrength = 0;

	float windowRadius;
	ofPolyline polyline;
	ofPoint p;

public:
	Beach(FftConfig* fftConfig) : VisualizationMode("Beach", fftConfig)
	{
		//Settings
		_sensibility = 150;
		_dtSpeed = 0.5;

		//FFT Post-Processing
		configIgnoredIndices(0, 0.55);
		configAutoCombineBands(32);
		configAutoRescale(0.2, 0.1/nBands, 0.35);
		configAutoMinRatio();
		configAutoDamper(1.1);

		//Initial Graphics settings
		ofBackground(0, 0, 0);
		ofSetFrameRate(60);
		_windowResized();
		addLayerFunction([this] { drawBackground(); });
		for (int i = 0; i < nWaves; i++)
		{
			addLayerFunction([this, i] { drawWave(i); });
		}
		addLayerFunction([this] { drawForeground(); });
	}
	~Beach() {}

	void windowResized()
	{
		windowRadius = halfHeight;
		drawSand = true;
		drawFrame = true;
		waveStrength = _sensibility * windowRadius / 100;
	}
	void update()
	{
		layerToRedraw++;
		layerToRedraw %= nWaves;
	}

private:
	bool drawSand = true;
	bool drawFrame = true;
	void drawBackground()
	{
		if (drawSand)
		{
			ofClear(0);
			ofTranslate(halfWidth, halfHeight);
			{
				ofFill();
				ofSetColor(sandColor);
				ofDrawRectangle(-windowRadius, -windowRadius, 2*windowRadius, 2* windowRadius);
				
				for (float y = -windowRadius; y < windowRadius; y += ofRandom(5))
				{
					for (float x = -windowRadius; x < windowRadius; x += ofRandom(5))
					{
						ofSetColor(0, 0, 0, ofRandom(10));
						ofDrawCircle(x + ofRandom(20) - 10, y + ofRandom(20) - 10, ofRandom(5 + y/200 + windowRadius/200));
					}
				}
			}
			ofPopMatrix();
			drawSand = false;
		}
	}
	void drawForeground()
	{
		if (drawFrame)
		{
			ofPushMatrix();
			ofTranslate(halfWidth, halfHeight);
			ofSetLineWidth(5);
			polyline.clear();
			polyline.setClosed(true);
			ofFill();
			ofSetColor(0, 0, 0);
			for (int i = 0; i < 6; i++)
			{
				ofRotate(60);
				ofDrawRectangle(-windowRadius, windowRadius / 1.15, 2 * windowRadius, windowRadius);
				p.x = windowRadius * sin(PI / 6 + 2 * PI * i / 6);
				p.y = windowRadius * cos(PI / 6 + 2 * PI * i / 6);
				polyline.addVertex(p);
			}
			ofSetColor(waterEdge);
			polyline.draw();
			ofPopMatrix();
			drawFrame = false;
		}
	}
	
	int layerToRedraw = 0;
	void drawWave(int layerIndex)
	{
		if (layerIndex != layerToRedraw) return;
		float avg = averagef(fft, nBands);
		avg = avg * avg * _sensibility * 2;
		avg = 1.5 * windowRadius * sigmoidf(avg - 3) / 1.23606;

		ofClear(0);
		ofPushMatrix();
		/// 1.23606 + halfHeight * (1 + 1 / sqrt(75))
		ofTranslate(halfWidth, halfHeight);
		ofRotate(3*sin(dt));
		ofTranslate(0, windowRadius/1.15);
		{
			// Filled wave
			ofSetColor(waterColor, 75);
			ofBeginShape();
			// Left vertices
			ofCurveVertex(-windowRadius, windowRadius);
			ofCurveVertex(-windowRadius, windowRadius);
			ofCurveVertex(-windowRadius * 1.05, -avg);
			for (int i = nBands - 1; i >= 0; i--)
			{
				p.x = -(i + 0.5) * windowRadius / nBands;
				p.y = -fft[i] * waveStrength - avg;
				ofCurveVertex(p.x, p.y);
			}
			// Right vertices
			for (int i = 0; i < nBands; i++)
			{
				p.x = (i + 0.5) * windowRadius / nBands;
				p.y = -fft[i] * waveStrength - avg;
				ofCurveVertex(p.x, p.y);
			}
			ofCurveVertex(windowRadius*1.05, -avg);
			ofCurveVertex(windowRadius, windowRadius);
			ofCurveVertex(windowRadius, windowRadius);
			ofEndShape();

			for (int j = 0; j < 5; j++)
			{
				// Waves edges
				p.z = 0;
				polyline.clear();
				polyline.setClosed(true);
				// Left vertices
				polyline.curveTo(-windowRadius, windowRadius);
				polyline.curveTo(-windowRadius, windowRadius);
				polyline.curveTo(-windowRadius * 1.05, -avg);
				ofSetColor(255, 255, 255, 150);
				for (int i = nBands - 1; i >= 0; i--)
				{
					p.x = -(i + 0.5) * windowRadius / nBands + (j + (j == 0 ? 0 : .5)) * windowRadius / 60;
					p.y = -fft[i] * waveStrength - avg + j * ofRandom(windowRadius / 40);
					if(j != 0) ofDrawCircle(p,10);
					polyline.curveTo(p);
				}
				// Right vertices
				for (int i = 0; i < nBands; i++)
				{
					p.x = (i + 0.5) * windowRadius / nBands - (j + (j == 0 ? 0 : .5)) * windowRadius / 60;
					p.y = -fft[i] * waveStrength - avg + j * ofRandom(windowRadius / 40);
					ofDrawCircle(p, 10);
					polyline.curveTo(p);
				}
				polyline.curveTo(windowRadius * 1.05, -avg);
				polyline.curveTo(windowRadius, windowRadius);
				polyline.curveTo(windowRadius, windowRadius);
				
				if (j == 0) ofSetColor(waterEdge, 200);
				polyline.draw();
			}
		}
		ofPopMatrix();
	}
};