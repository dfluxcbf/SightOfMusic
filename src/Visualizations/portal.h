#pragma once
#include "visualization_mode.h"

class Portal : public VisualizationMode
{
    float radius, rotation,
        cosAng1, cosAng2,
        sinAng1, sinAng2,
        lineLength,
        f1 = 0, f2 = 0, f3 = 0,
        f1Acc = 0, f2Acc = 0,
        minFft = 0,
        centerY, centerX,
        fps, avg;
    int index = 0;
    ofColor fillColor, edgeColor;

public:
    Portal(FftConfig* fftConfig) : VisualizationMode("Portal", fftConfig)
    {
        //Settings
        _sensibility = 140;
        _dtSpeed = 3E-1;

        //FFT Post-Processing
        configIgnoredIndices(0, 0.5);
        configAutoCombineBands(16);
        configAutoDamper(1.06);
        configAutoRescale(0.7, 0.002, 0.4);

        //Initial Graphics settings
        ofBackground(0, 0, 0);
        ofSetLineWidth(2);
        _windowResized();
        ofClear(0);
        edgeColor = ofColor(0, 0, 0);
        fillColor.setSaturation(100);
        fillColor.setBrightness(255);
        ofSetCircleResolution(nBands);
        addLayerFunction([&] { drawDefaultLayer0(); });
    }
    ~Portal() {}

    void windowResized()
    {
        radius = min(width, height) / 7;
    }
    void update() {
        fps = ofGetFrameRate();
        avg = averagef(fft, nBands);
        avg *= avg + 1.5;

        // Updates portal position
        // Moving around like crazy
        f1 = 0.98 * f1 + 0.02 * avg;
        f1Acc += 0.312 * (f1 + f2 * 0.01) / (3 * fps + 1.f);
        f2 = 0.95 * f1 + 0.05 * avg;
        f2Acc += 0.865 * (f2 + f1 * 0.05) / (3 * fps + 1.f);
        f3 += _sensibility * (avg + f1 * 0.01 + f2 * 0.02) / (300 * fps + 1.f);
        centerX = halfWidth * (0.35 * sin(f1Acc) + 0.05 * sin(f3) + 0.05 * cos(f2Acc) + 0.1 * cos((f2Acc + f1Acc) / 1) * sin((f2Acc + f1Acc)) + 0.025 * sin(f3 - f1Acc));
        centerY = halfHeight * (0.25 * sin(f2Acc) + 0.05 * cos(f3) + 0.05 * cos(f1Acc) + 0.1 * cos((f2Acc + f1Acc) / 1) * sin((f2Acc + f1Acc)) + 0.025 * sin(f3 - f2Acc));
        
        minFft = minf(fft, nBands) / 4;
        fillColor.setHue(fmodf(dt * 125, 255.f));
    }

private:
    ofColor innerTextureColor;
    void drawDefaultLayer0()
    {
        innerTextureColor = fillColor;
        innerTextureColor.setBrightness(innerTextureColor.getBrightness() / 1.5);

        ofPushMatrix();
        ofTranslate(halfWidth + centerX, halfHeight + centerY); // Portal position
        ofRotate(f2Acc*20); // Portal rotation

        // Draw portal
        index = 0;
        for (int i = 0; i < 2 * nBands; i++)
        {
            index = (i < nBands ? i : 2 * nBands - 1 - i);
            lineLength = 0.25 * radius + (fft[index] - minFft) * _sensibility;

            // Bar start angle
            rotation = i * TWO_PI / float(2 * nBands);
            cosAng1 = cos(rotation);
            sinAng1 = sin(rotation);

            // Bar end angle
            rotation = (i + 1) * TWO_PI / float(2 * nBands);
            cosAng2 = cos(rotation);
            sinAng2 = sin(rotation);

            // Draws inner texture
            ofSetColor(innerTextureColor);
            ofDrawCircle((radius-1) * sinAng1, (radius-1) * cosAng1, 1);

            // Draws bars
            DRAW_W_EDGES(fillColor, edgeColor,
                ofBeginShape();
                ofVertex(
                    radius * sinAng1,
                    radius * cosAng1);
                ofVertex(
                    (radius + lineLength) * sinAng1,
                    (radius + lineLength) * cosAng1);
                ofVertex(
                    (radius + lineLength) * sinAng2,
                    (radius + lineLength) * cosAng2);

                ofVertex(
                    radius * sinAng2,
                    radius * cosAng2);
                ofEndShape(false);
            );

            // Clears outside of portal
            ofSetColor(0, 0, 0);
            ofBeginShape();
            ofVertex(
                (radius + lineLength) * sinAng2,
                (radius + lineLength) * cosAng2);

            ofVertex(
                (radius + lineLength) * sinAng1,
                (radius + lineLength) * cosAng1);

            ofVertex(
                halfWidth * sinAng1,
                halfWidth * cosAng1);

            ofVertex(
                halfWidth * sinAng2,
                halfWidth * cosAng2);
            ofEndShape();
        }
        ofPopMatrix();
    }
};