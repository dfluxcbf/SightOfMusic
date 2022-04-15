#include "audio_sink.h"
#include "../logging.h"

AudioSink::AudioSink(PWAVEFORMATEX pAudioSettings, float recordingPeriod) : _recordingPeriod(recordingPeriod)
{
	// Audio Format
	_pAudioSettings = pAudioSettings;

	// Audio Buffer
	_pRecordedSoundWave = new SoundWave(_recordingPeriod, _pAudioSettings);
	_fftComputer = new FffComputer(_pRecordedSoundWave->_alignedSoundWaveData, pAudioSettings, recordingPeriod);

	// Logs
	LOG("Audio Sink configured.");
	LOG("Audio settings :");
	LOG("\t- Recording period: %f seconds.", _recordingPeriod);
	LOG("\t- Sample rate: %d Hz", _pAudioSettings->nSamplesPerSec);
	LOG("\t- Channels: %d", _pAudioSettings->nChannels);
	LOG("\t- Bits per sample %d", _pAudioSettings->wBitsPerSample);
	LOG("\t- Block alignment: %d", _pAudioSettings->nBlockAlign);
}

AudioSink::~AudioSink()
{
	delete _fftComputer;
	delete _pRecordedSoundWave;
}

HRESULT AudioSink::InsertAudioData(BYTE* pData, UINT32 NumFrames)
{
	HRESULT hr;
	if (pData == NULL)
	{
		//No data to write
		hr = _pRecordedSoundWave->InsertSilence(NumFrames);
	}
	else
	{
		//Write data
		hr = _pRecordedSoundWave->InsertAudioFrames(pData, NumFrames);
	}
	return hr;
}

void AudioSink::GetFftData(float* fftDst)
{
	_pRecordedSoundWave->AlignData();
	_fftComputer->ComputeFFT();
	_fftComputer->CopyFftData(fftDst);
}

void AudioSink::QuickGetFftData(float* fftDst)
{
	_fftComputer->CopyFftData(fftDst);
}

size_t AudioSink::GetFftDimensions()
{
	return _fftComputer->fftSize;
}
