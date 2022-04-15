#pragma once
#include "sound_wave.h"
#include "fft_computer.h"

class AudioSink
{
public:
	AudioSink(PWAVEFORMATEX pAudioSettings, float recordingPeriod);
	~AudioSink();

	// Copies a data buffer into the AudioSink
	HRESULT InsertAudioData(BYTE* pData, UINT32 NumFrames);

	// Copy fft data so it can be used
	// Aligns data and calculates FFT before copying
	void GetFftData(float* fftDst);
	// Just copies data
	void QuickGetFftData(float* fftDst);
	// Get FFT Dimensions
	size_t AudioSink::GetFftDimensions();

	// Read-only values
	const WAVEFORMATEX& audioSettings = *_pAudioSettings;

private:
	// Audio Format
	WAVEFORMATEX* _pAudioSettings;
	// Audio Buffer
	SoundWave* _pRecordedSoundWave;
	// Fft Computer
	FffComputer* _fftComputer;

	const float _recordingPeriod;
};