#include "fft_computer.h"
#include "memory_locks.h"
#include "..\logging.h"

#include <math.h>

#define REAL 0
#define IMAG 1

#define LOG_BASE 20

FffComputer::FffComputer(float* inputAddres, PWAVEFORMATEX pAudioSettings, float recordingPeriod)
{
	// Define sizes
	_nChannels = pAudioSettings->nChannels;
	_soundSize = pAudioSettings->nSamplesPerSec * recordingPeriod / _nChannels / sizeof(float);
	_fftSize = _soundSize / 2 + 1;

	// Initialize fftOutput
	_fftOutput = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize * _nChannels);

	// Initialize fft magnitude output
	_frequencyData = (float*)malloc(sizeof(float) * _fftSize);
	memset(_frequencyData, 0, sizeof(float) * _fftSize);

	// Create FFT plan
	if (_nChannels == 1)
	{
		_plan = fftwf_plan_dft_r2c_1d(_soundSize, inputAddres, _fftOutput, FFTW_ESTIMATE);
	}
	else
	{
		_plan = fftwf_plan_dft_r2c_2d(_nChannels, _soundSize, inputAddres, _fftOutput, FFTW_ESTIMATE);
	}

	LOG("FFT Computer configured:");
	LOG("\t- Sound Size: %d", _soundSize);
	LOG("\t- FFT Bands: %d", _fftSize);
	LOG("\t- FFT Channels: %d", _nChannels);
}

FffComputer::~FffComputer()
{
	if (_plan != NULL)
	{
		fftwf_destroy_plan(_plan);
		fftwf_cleanup();
	}
}

void FffComputer::ComputeFFT()
{
	if (_plan != NULL)
	{
		/*
		 *	struct Locks {
		 *		bool lock_alignedSoundWaveData = false;
		 *		bool lock_soundWaveData = false;
		 *		bool lock_frequencyData = false;
		 *		bool lock_fftOutput = false;
		 *	};
		 */
		// Locks: lock_alignedSoundWaveData,lock_fftOutput
		const SoundMemoryAccess::Locks planLock{true, false, false, true};
		SoundMemoryAccess::__Locker__.Lock(planLock);
		// Calculates the FFT
		fftwf_execute(_plan);
		SoundMemoryAccess::__Locker__.Unlock(planLock);

		// Locks: lock_frequencyData,lock_fftOutput
		const SoundMemoryAccess::Locks magnitudeLock{false, false, true, true};
		SoundMemoryAccess::__Locker__.Lock(magnitudeLock);
		// Calculates magnitude of fftOutput
		memset(_frequencyData, 0, _fftSize * sizeof(float));
		const float rescale = 1.f / (float)(_fftSize * _nChannels);
		for (int i = 0; i < _fftSize; i++)
		{
			_frequencyData[i] = 0;
			// Combines channels
			for (int j = 0; j < _nChannels; j++)
			{
				_frequencyData[i] +=
					sqrtf(
						(
							_fftOutput[i + j * _fftSize][REAL] * _fftOutput[i + j * _fftSize][REAL] +
							_fftOutput[i + j * _fftSize][IMAG] * _fftOutput[i + j * _fftSize][IMAG]
							) * rescale
					);
			}
		}

		// Rescale axis to Log10
		float r1, r2;
		float accIndexI, accIndexF;
		float ratio1, ratio2;
		int indexI, indexF;
		float* tempFft = new float[_fftSize];
		for (float i = 0; i < _fftSize; i++)
		{
			r1 = 1 - (i / (float)_fftSize);
			r2 = 1 - ((i + 1) / (float)_fftSize);
			accIndexI = _fftSize * (1 - logf(r1 * (LOG_BASE - 1) + 1) / logf(LOG_BASE));
			accIndexF = _fftSize * (1 - logf(r2 * (LOG_BASE - 1) + 1) / logf(LOG_BASE));
			if (accIndexI < 0) accIndexI = 0;
			if (accIndexF > _fftSize) accIndexF = _fftSize;

			indexI = (int)floorf(accIndexI);
			indexF = (int)floorf(accIndexF);

			if (indexI == indexF)
			{
				ratio1 = accIndexF - accIndexI;
				/*
				LOG("new[%d] = %f * old[%d]", //3
					(int)i,
					(accIndexF - accIndexI),
					(int)indexI);
				*/
				tempFft[(int)i] = ratio1 * _frequencyData[(int)indexI];
			}
			else if (indexI + 1 == indexF)
			{
				ratio1 = ceilf(accIndexI) - accIndexI;
				ratio2 = accIndexF - floorf(accIndexF);
				/*
				LOG("new[%d] = %f * old[%d] + %f * old[%d]", //5
					(int)i,
					ratio1,
					(int)indexI,
					ratio2,
					(int)indexF);
				*/
				tempFft[(int)i] = ratio1 * _frequencyData[(int)indexI] + 
								  ratio2 * _frequencyData[(int)indexF];
			}
			else
			{
				ratio1 = ceilf(accIndexI) - accIndexI;
				ratio2 = accIndexF - floorf(accIndexF);
				/*
				LOG("new[%d] = %f * old[%d] + SUM(old[%d...%d]) + %f * old[%d]", //7
					(int)i,
					ratio1,
					(int)indexI,
					(int)indexI + 1,
					(int)indexF - 1,
					ratio2,
					(int)indexF);
				if (indexF == fftSize) LOG("Ignore index %d", fftSize);
				*/
				tempFft[(int)i] = ratio1 * _frequencyData[(int)indexI];
				for (int sumIndex = (int)indexI + 1; sumIndex <= (int)indexF - 1; sumIndex++)
				{
					tempFft[(int)i] += _frequencyData[sumIndex];
				}
				if((int)indexF != _fftSize) tempFft[(int)i] += ratio2 * _frequencyData[(int)indexF];
			}
		}
		memcpy(_frequencyData, tempFft, sizeof(float) * _fftSize);
		delete tempFft;
		SoundMemoryAccess::__Locker__.Unlock(magnitudeLock);
	}
}

void FffComputer::CopyFftData(float* dstFftData)
{
	/*
	 *	struct Locks {
	 *		bool lock_alignedSoundWaveData = false;
	 *		bool lock_soundWaveData = false;
	 *		bool lock_frequencyData = false;
	 *		bool lock_fftOutput = false;
	 *	};
	 */
	//Locks: lock_frequencyData,lock_fftOutput
	const SoundMemoryAccess::Locks frequencyLock{ false, false, true, false };
	SoundMemoryAccess::__Locker__.Lock(frequencyLock);
	memcpy(dstFftData, _frequencyData, _fftSize * sizeof(float));
	SoundMemoryAccess::__Locker__.Unlock(frequencyLock);
}