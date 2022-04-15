#pragma once
#include "visualization_mode.h"

#define _DEVELOPING_ Portal

class Portal : public VisualizationMode
{
    float radius, rotation,
        cosAng1, cosAng2,
        sinAng1, sinAng2,
        lineLength,
        f1 = 0, f2 = 0, f3 = 0,
        f1Acc = 0, f2Acc = 0;
    float centerY, centerX;
    int index = 0;
    ofColor fillColor, edgeColor;

public:
    Portal(FftConfig* fftConfig) : VisualizationMode("Portal", fftConfig, 1)
    {
        _sensibility = 140;
        _dtSpeed = 3E-1;
        configIgnoredIndices(0, 0.5);
        configAutoDamper(1.03);
        configAutoCombineBands(16);
        configAutoRescale(0.7, 0.002, 0.5);
        ofBackground(0, 0, 0);
        ofSetLineWidth(2);
        _windowResized();
        ofClear(0);
        edgeColor = ofColor(0, 0, 0);
        fillColor.setSaturation(100);
        fillColor.setBrightness(255);
        ofSetCircleResolution(nBands_combined);
    }
    ~Portal() {}

    void windowResized()
    {
        radius = min(width, height) / 10;
    }
    void keyPressed(int key) {}
    void keyReleased(int key) {}

    float fps, avg;
    void update() {
        fps = ofGetFrameRate();
        avg = averagef(_combinedFft, nBands_combined);
        avg *= avg + 1.5;
        f1 = 0.98 * f1 + 0.02 * avg;
        f1Acc += 0.312 * f1 / (2 * fps + 1.f);
        f2 = 0.95 * f1 + 0.05 * avg;
        f2Acc += 0.865 * f2 / (2 * fps + 1.f);
        f3 += _sensibility * avg / (200 * fps + 1.f);

        centerX = halfWidth * (0.35 * sin(f1Acc) + 0.05 * sin(f3) + 0.05 * cos(f2Acc) + 0.1 * cos((f2Acc + f1Acc) / 1) * sin((f2Acc + f1Acc)) + 0.025 * sin(f3 - f1Acc));
        centerY = halfHeight * (0.25 * sin(f2Acc) + 0.05 * cos(f3) + 0.05 * cos(f1Acc) + 0.1 * cos((f2Acc + f1Acc) / 1) * sin((f2Acc + f1Acc)) + 0.025 * sin(f3 - f2Acc));

        fillColor.setHue(fmodf(dt * 125, 255.f));
    }
    void drawDefaultLayer0()
    {
        //ofClear(0);
        index = 0;
        ofPushMatrix();
        ofTranslate(halfWidth + centerX, halfHeight + centerY);
        ofRotate(dt*10);
        for (int i = 0; i < 2 * nBands_combined; i++)
        {
            index = (i < nBands_combined ? i : 2 * nBands_combined - 1 - i);
            lineLength = 0.25 * radius + _combinedFft[index] * _sensibility;
            rotation = i * TWO_PI / float(2 * nBands_combined);

            cosAng1 = cos(rotation);
            sinAng1 = sin(rotation);

            rotation = (i + 1) * TWO_PI / float(2 * nBands_combined);

            cosAng2 = cos(rotation);
            sinAng2 = sin(rotation);

            ofSetColor(fillColor);

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
        //ofSetColor(255, 255, 255, 1);
        //ofDrawCircle(0, 0, radius);
        //ofSetColor(255, 255, 255);
        //ofDrawCircle(0, 0, radius * avg / 20);
        ofPopMatrix();
    }
    void drawLayer1() {}
    void drawLayer2() {}
    void drawDebugLayer() {}
};