#pragma once
#include "audio_capture_config.h"

class LoopbackAudioCaptureThread
{
public:
	LoopbackAudioCaptureThread(AudioCaptureConfig* pAudioCaptureConfig);
	~LoopbackAudioCaptureThread();

	/* Starts thread
	 * This will configure the audio capture according to the audio output settings
	 * and set the capture stream into an AudioSink.
	 * The audio format will be recorded into _pAudioCaptureConfig.
	 * The AudioSink is available in _pAudioCaptureConfig.
	 */
	bool Start();

private:
	AudioCaptureConfig* _pAudioCaptureConfig;
	HANDLE _hThread;
};