#pragma once
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <mutex>

struct SoundWave
{
public:
	SoundWave(float recordingPeriod, PWAVEFORMATEX pAudioFormat);
	~SoundWave();

	// Insert a sequense of bytes inside the circular audio buffer.
	HRESULT InsertAudioFrames(BYTE* audioData, size_t nFrames);
	// Insert a sequence of zeros inside the circular audio buffer.
	HRESULT InsertSilence(size_t nFrames);
	
	/*
	 * Converts _soundWaveData into _alignedSoundWaveData
	 * _soundWaveData is a circular buffer in column-major order.
	 * _alignedSoundWaveData is a linear buffer in row-major order.
	 */
	void AlignData();

	// Read-only values
	const size_t& nFrames = _totalNumFrames;
	const WAVEFORMATEX& audioFormat = (*_pAudioFormat);

	// Operators
	// Gets data from circular buffer _soundWaveData
	float operator() (WORD channel, size_t sample);

	/*
	 * _alignedSoundWaveData is a buffer used to read the audio data.
	 * Storing multiple channels will result in a row-major order array. This is used for FFT computation.
	 * The first audio sample or the first channel will be aligned with the begining of the buffer.
	 */
	float* _alignedSoundWaveData;

private:
	/*
	 *	_soundWaveData is a circular buffer for the audio data.
	 *  Storing multiple channels will result in a column-major order array.
	 *	This is used to continuously get audio data avoiding excessive memory read/write.
	 *	_dataStartFrame marks the position of the first audio sample inside this buffer.
	 */
	float* _soundWaveData;

	size_t _dataStartFrame = 0;
	size_t _totalNumFrames;
	PWAVEFORMATEX _pAudioFormat;

	// Verify that the sample position exists inside the buffer.
	void VerifyArrayAccess(WORD channel, size_t sample);
};