#include "audio_capture.h"
#include "../logging.h"

#include <mmdeviceapi.h>
#include <audioclient.h>

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres);  \
              if (FAILED(hres)) { ERR("Exiting with error code 0x%X.", hres); goto Exit; }
#define SAFE_RELEASE(punk);  \
              if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

DWORD WINAPI AudioCap_ThreadFunction(LPVOID pContex);
HRESULT StablishAudioStream(AudioCaptureConfig * pAudioCaptureConfig);

LoopbackAudioCaptureThread::LoopbackAudioCaptureThread(AudioCaptureConfig * pAudioCaptureConfig)
{
	/*
	 * Stores the reference for _pAudioCaptureConfig.
	 * This is where the audio format and audio sink are stored for external use.
	 */
	_pAudioCaptureConfig = pAudioCaptureConfig;
	_hThread = NULL;
}

LoopbackAudioCaptureThread::~LoopbackAudioCaptureThread()
{
	// Makes sure the thread is closed properly when deinitializing LoopbackAudioCaptureThread
	if (_hThread != NULL && !CloseHandle(_hThread))
	{
		ERR("LoopbackAudioCaptureThread closed with error:\n\t%u", GetLastError());
	}
	else if (_hThread == NULL)
	{
		ERR("LoopbackAudioCaptureThread deconstructed with NULL Thread.");
	}
	else
	{
		LOG("LAC Thread closed successfully.");
	}
}


bool LoopbackAudioCaptureThread::Start()
{
	// Create thread
	_hThread = CreateThread(
		NULL, 0,
		AudioCap_ThreadFunction, _pAudioCaptureConfig,
		0, NULL
	);
	if (_hThread == NULL) // Fail to start thread
	{
		ERR("CreateThread failed: %u", GetLastError());
		return false;
	}
	LOG("LAC Thread started successfully.");
	return true;
}


// Thread function to capture audio loopback
DWORD WINAPI AudioCap_ThreadFunction(LPVOID pContex)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		ERR(L"CoInitialize failed with code: 0x%08x", hr);
		return hr;
	}

	// Audio stream is a loop that colects audio data
	StablishAudioStream((AudioCaptureConfig*)pContex);
	CoUninitialize();
	return 0;
}

// Audio stream is a loop that colects audio data
HRESULT StablishAudioStream(AudioCaptureConfig* pAudioCaptureConfig)
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	AudioSink* pAudioStreamSink;
	UINT32 packetLength = 0;
	BOOL isDone = FALSE;
	BYTE* audioData;
	DWORD flags;

	// Initial configuration
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr);

	hr = pEnumerator->GetDefaultAudioEndpoint(
		eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr);

	hr = pDevice->Activate(
		IID_IAudioClient, CLSCTX_ALL,
		NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->GetMixFormat(&pwfx); // Get audio data format
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_LOOPBACK,
		hnsRequestedDuration,
		0,
		pwfx,
		NULL);
	EXIT_ON_ERROR(hr);
	pAudioCaptureConfig->pAudioSettings = pwfx;

	hr = pAudioClient->GetService(
		IID_IAudioCaptureClient,
		(void**)&pCaptureClient);
	EXIT_ON_ERROR(hr);

	// Instantiate audio sink
	try
	{
		pAudioCaptureConfig->pAudioSink = new AudioSink(pAudioCaptureConfig->pAudioSettings, pAudioCaptureConfig->audioRecordingPeriod);
	}
	catch (...)
	{
		EXIT_ON_ERROR(CO_E_INIT_MEMORY_ALLOCATOR);
	}
	pAudioStreamSink = pAudioCaptureConfig->pAudioSink;

	// Start capturing audio
	hr = pAudioClient->Start();  
	EXIT_ON_ERROR(hr);
	//TODO: use conditional variable to unlock main proccess
	pAudioCaptureConfig->ready = true;
	LOG("Audio Capture has finished configuring.");
	while (isDone == FALSE)
	{
		// Sleep for a little while. (~166FPS)
		Sleep(6);

		hr = pCaptureClient->GetNextPacketSize(&packetLength);
		EXIT_ON_ERROR(hr);

		//LOG("Packet Length: %d", packetLength);
		while (packetLength != 0)
		{
			// Get the available data in the shared buffer
			hr = pCaptureClient->GetBuffer(
				&audioData,
				&numFramesAvailable,
				&flags, NULL, NULL);
			EXIT_ON_ERROR(hr);

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				audioData = NULL;  // Tell CopyData to write silence
			}

			// Copy the available capture data to the audio sink
			//LOG(" - Frames available: %d", numFramesAvailable);
			hr = pAudioStreamSink->InsertAudioData(audioData, numFramesAvailable);
			EXIT_ON_ERROR(hr);

			hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
			EXIT_ON_ERROR(hr);

			hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr);
		}
	}

	hr = pAudioClient->Stop();
	EXIT_ON_ERROR(hr);

Exit: //EXIT_ON_ERROR
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pCaptureClient);

	return hr;
}