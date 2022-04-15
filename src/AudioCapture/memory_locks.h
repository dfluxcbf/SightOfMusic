#pragma once
#include <mutex>

namespace SoundMemoryAccess
{
	struct Locks {
		bool lock_alignedSoundWaveData = false;
		bool lock_soundWaveData = false;
		bool lock_frequencyData = false;
		bool lock_fftOutput = false;
	};

	class Locker
	{
	public:
		void Lock(const Locks locks)
		{
			// Aligned SoundWave Data
			if (locks.lock_alignedSoundWaveData) _mtx_alignedSoundWaveData.lock();
			// SoundWave Data
			if (locks.lock_soundWaveData) _mtx_soundWaveData.lock();
			// Frequency Data
			if (locks.lock_frequencyData) _mtx_frequencyData.lock();
			// FFT Output
			if (locks.lock_fftOutput) _mtx_fftOutput.lock();
		}
		void Unlock(const Locks locks)
		{
			// Aligned SoundWave Data
			if (locks.lock_alignedSoundWaveData) _mtx_alignedSoundWaveData.unlock();
			// SoundWave Data
			if (locks.lock_soundWaveData) _mtx_soundWaveData.unlock();
			// Frequency Data
			if (locks.lock_frequencyData) _mtx_frequencyData.unlock();
			// FFT Output
			if (locks.lock_fftOutput) _mtx_fftOutput.unlock();
		}
	private:
		std::mutex _mtx_alignedSoundWaveData;
		std::mutex _mtx_soundWaveData;
		std::mutex _mtx_frequencyData;
		std::mutex _mtx_fftOutput;
	};
	static Locker __Locker__;
}