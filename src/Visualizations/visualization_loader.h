#pragma once
#include "ofMain.h"
#include "AudioCapture/audio_capture.h"
#include "visualization_mode.h"

#define N_VIS 8

class VisualizationLoader : public ofBaseApp {
	AudioSink* _sink;
	float* fft;
	size_t fftBands;
	VisualizationMode* vm;
	FftConfig fftConfig;
	int visIndex = 0;
	size_t visOrder[N_VIS];
	std::map<char, std::function<void()>> keyActions;
	
public:
	VisualizationLoader(AudioSink* sink);
	~VisualizationLoader();
	void NextVisualization(int& index);
	void SwitchVisualization(int index);

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	void configLoader();
};