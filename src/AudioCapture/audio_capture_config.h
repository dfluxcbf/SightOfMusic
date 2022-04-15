#pragma once
#include "audio_sink.h"

struct AudioCaptureConfig
{
	AudioSink* pAudioSink;
	PWAVEFORMATEX pAudioSettings;
	float audioRecordingPeriod = 0.5;
	size_t nFftBands = 32; //Must be power of 2
	bool ready = false; //TODO: use conditional variable to unlock main proccess
};
