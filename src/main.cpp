#include "logging.h"
#include "ofMain.h"
#include "Visualizations/visualization_loader.h"
#include "AudioCapture/audio_capture.h"

//TODO: Change all pointers to array to smart pointers for safe free

#include <math.h>
int main( ){
	AudioCaptureConfig acc;
	acc.audioRecordingPeriod = 0.1;

	LoopbackAudioCaptureThread lacThread(&acc);
	if (!lacThread.Start())
	{
		ERR("Failed to start LAC thread.");
		//TODO: Create safe progam exit
		return -1;
	}
	
	/*
	FFTCaptureThread fftThread(&acc);
	if (!fftThread.Start())
	{
		ERR("Failed to start FFT thread.");
		//TODO: Create safe progam exit
		return -1;
	}
	*/

	//TODO: use conditional variable to unlock main proccess
	while (!acc.ready) {}

	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new VisualizationLoader(acc.pAudioSink));
}