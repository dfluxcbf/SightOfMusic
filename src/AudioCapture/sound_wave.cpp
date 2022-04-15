#include "sound_wave.h"
#include "memory_locks.h"
#include "../logging.h"

#include <string>
#include <stdexcept>

void SoundWave::VerifyArrayAccess(WORD channel, size_t sample)
{
	if (channel >= _pAudioFormat->nChannels) throw std::out_of_range("Channel access violation. Channel out of range.");
	if (sample * _pAudioFormat->nChannels + channel > _totalNumFrames) throw std::out_of_range("Sample access violation. Sample out of range.");
}

SoundWave::SoundWave(float recordingPeriod, PWAVEFORMATEX pAudioFormat)
{
	_pAudioFormat = pAudioFormat;
	_totalNumFrames = (size_t)(pAudioFormat->nSamplesPerSec * recordingPeriod);
	_totalNumFrames += _totalNumFrames % sizeof(float); // Padding (Usually returns zero)

	// Allocates memory to store audio data
	_soundWaveData = (float*)malloc(_totalNumFrames);
	if (_soundWaveData == 0) throw std::bad_alloc();
	memset(_soundWaveData, 0, _totalNumFrames);
	
	_alignedSoundWaveData = (float*)malloc(_totalNumFrames);
	if (_alignedSoundWaveData == 0) throw std::bad_alloc();
	memset(_alignedSoundWaveData, 0, _totalNumFrames);

	// Logs
	LOG("SoundWave initialized with configuration:");
	LOG("\t- Buffer Recording Period: %f", recordingPeriod);
	LOG("\t- Channels: %d", _pAudioFormat->nChannels);
	LOG("\t- Number of Frames: %d", _totalNumFrames);
}
SoundWave::~SoundWave()
{
	// Free memory for stored audio data
	free(_soundWaveData);
	free(_alignedSoundWaveData);
}

HRESULT SoundWave::InsertAudioFrames(BYTE* audioData, size_t nFrames)
{
	/*
	 *	struct Locks {
	 *		bool lock_alignedSoundWaveData = false;
	 *		bool lock_soundWaveData = false;
	 *		bool lock_frequencyData = false;
	 *		bool lock_fftOutpurt = false;
	 *	};
	 */
	 //Locks: _soundWaveData
	const SoundMemoryAccess::Locks soundLock{ false, true, false, false };
	SoundMemoryAccess::__Locker__.Lock(soundLock);
	try
	{
		// Data still fits until the end of the 
		if (_dataStartFrame + nFrames <= _totalNumFrames)
		{
			memcpy(_soundWaveData + _dataStartFrame / sizeof(float), audioData, nFrames);
			_dataStartFrame += nFrames;
		}
		else
		{
			memcpy(_soundWaveData + _dataStartFrame / sizeof(float),
				audioData,
				_totalNumFrames - _dataStartFrame);
			memcpy(_soundWaveData,
				audioData + (_totalNumFrames - _dataStartFrame),
				nFrames - (_totalNumFrames - _dataStartFrame));
			_dataStartFrame = nFrames - _dataStartFrame + _totalNumFrames;
		}
	}
	catch (...)
	{
		SoundMemoryAccess::__Locker__.Unlock(soundLock);
		return E_MBN_SMS_INVALID_MEMORY_INDEX;
	}
	SoundMemoryAccess::__Locker__.Unlock(soundLock);
	return S_OK;
}

HRESULT SoundWave::InsertSilence(size_t nFrames)
{
	/*
	 *	struct Locks {
	 *		bool lock_alignedSoundWaveData = false;
	 *		bool lock_soundWaveData = false;
	 *		bool lock_frequencyData = false;
	 *		bool lock_fftOutpurt = false;
	 *	};
	 */
	 //Locks: _soundWaveData
	const SoundMemoryAccess::Locks silenceLock{ false, true, false, false };
	SoundMemoryAccess::__Locker__.Lock(silenceLock);
	try
	{
		if (_dataStartFrame + nFrames <= _totalNumFrames)
		{
			memset(_soundWaveData + _dataStartFrame / sizeof(float), 0, nFrames);
			_dataStartFrame += nFrames;
		}
		else
		{
			memset(_soundWaveData + _dataStartFrame / sizeof(float),
				0,
				_totalNumFrames - _dataStartFrame);
			memset(_soundWaveData,
				0,
				nFrames - (_totalNumFrames - _dataStartFrame));
			_dataStartFrame = nFrames - _dataStartFrame + _totalNumFrames;
		}
	}
	catch (...)
	{
		SoundMemoryAccess::__Locker__.Unlock(silenceLock);
		return E_MBN_SMS_INVALID_MEMORY_INDEX;
	}
	SoundMemoryAccess::__Locker__.Unlock(silenceLock);
	return S_OK;
}

void SoundWave::AlignData()
{
	/*
	 *	struct Locks {
	 *		bool lock_alignedSoundWaveData = false;
	 *		bool lock_soundWaveData = false;
	 *		bool lock_frequencyData = false;
	 *		bool lock_fftOutpurt = false;
	 *	};
	 */
	 //Locks: _alignedSoundWaveData, _soundWaveData
	const SoundMemoryAccess::Locks alignLock{ true, true, false, false };
	SoundMemoryAccess::__Locker__.Lock(alignLock);

	const int nFSamples = _totalNumFrames / _pAudioFormat->nChannels / sizeof(float);
	for (int i = 0; i < _pAudioFormat->nChannels; i ++)
	{
		for (int j = 0; j < nFSamples; j++)
		{
			*(_alignedSoundWaveData + i*nFSamples + j) = (*this)(i,j);
		}
	}
	SoundMemoryAccess::__Locker__.Unlock(alignLock);
}

float SoundWave::operator() (WORD channel, size_t sample)
{
	size_t index;
	// Move index to buffer starting position
	index = _dataStartFrame / sizeof(float);
	// Position in column-major order
	index += sample * _pAudioFormat->nChannels + channel;
	// Update index out of the circular buffer
	index %= (_totalNumFrames / sizeof(float));
	return *(this->_soundWaveData + index);
}