#pragma once
#include <fftw3.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <mutex>

class FffComputer
{
public:
	FffComputer(float* inputAddres, PWAVEFORMATEX pAudioSettings, float recordingPeriod);
	~FffComputer();

	void ComputeFFT();
	void CopyFftData(float* dstFftData);

	// Read-only values
	const bool& hasNewData = _hasNewData;
	const size_t& fftSize = _fftSize;
	const size_t& fftChannels = _nChannels;

private:
	fftwf_plan _plan = NULL;
	fftwf_complex* _fftOutput;
	float* _frequencyData;
	size_t _soundSize;
	size_t _fftSize;
	size_t _nChannels;
	bool _hasNewData = false;
};
