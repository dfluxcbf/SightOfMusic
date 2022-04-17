#pragma once
#include <string>
#include <math.h>

#include "ofMain.h"
#include "../utils.h"
#include "../logging.h"

struct FftConfig
{
	float* fft;
	size_t fftBands;
};

class PostProcessingRef
{
	std::function<void()> _postProcessingFunction;
public:
	const std::string _name;
	const size_t _nBandsRefToFunction;
	const size_t _nBandsResultToFunction;
	float* const _fftRefToFunction;
	float* const _fftResultToFunction;
	PostProcessingRef(std::string name, float** fftReference, size_t* nBandsRefence, size_t nBandsResult) :
		_name(name), _nBandsRefToFunction(*nBandsRefence),
		_nBandsResultToFunction(nBandsResult), _fftRefToFunction(*fftReference),
		_fftResultToFunction((float*)malloc(nBandsResult * sizeof(float)))
	{	
		memset(_fftResultToFunction, 0, nBandsResult * sizeof(float));
		*fftReference = _fftResultToFunction;
		*nBandsRefence = nBandsResult;
		LOG("\t\t- FFT Reference: 0x%p [%d]", _fftRefToFunction, _nBandsRefToFunction);
		LOG("\t\t- FFT Output: 0x%p [%d]", _fftResultToFunction, _nBandsResultToFunction);
	}
	~PostProcessingRef()
	{
		free(_fftResultToFunction);
	}
	void execute()
	{
		_postProcessingFunction();
	}
	void registerFunction(std::function<void()> postProcessingFunction)
	{
		_postProcessingFunction = postProcessingFunction;
	}
};

//TODO: Move all post-processing of fft into a new thread
class VisualizationMode
{
	float* _fft;
	size_t _nBands;

	const float sensibilityDelta = 0.01; //1%

	float _width, _height, _halfWidth, _halfHeight,
		  _dt = ofRandom(255);
	std::map<char, std::function<void()>> keyActions;

	std::vector<ofFbo> layers;
	std::vector<std::function<void()>> layerFunctions;
	std::vector<PostProcessingRef*> postProcessingFunctions;
	DEBUG(
		ofFbo debugLayer;
		std::function<void()> debugLayerFunction = NULL;
	);

public:
	const string _name;

	VisualizationMode(string name, FftConfig* fftConfig);
	virtual ~VisualizationMode();
	void draw();
	void _keyPressed(int key);
	void _windowResized();
	void _update();

protected:
	float _sensibility = 50, _dtSpeed = 1;

	const float* const& fft = _fft;
	const size_t& nBands = _nBands;

	const float& dt = _dt;
	const float& width = _width;
	const float& height = _height;
	const float& halfWidth = _halfWidth;
	const float& halfHeight = _halfHeight;

	virtual void windowResized() = 0;
	virtual void update() = 0;

	void addLayerFunction(std::function<void()> newLayerFunction);
	void reallocateLayers();
	void configAutoDamper(float damper);
	float fpsRelativeDamp(float _damper);
	void configAutoRescale(float base, float multiplier, float power);
	void configAutoCombineBands(size_t nBandsToCombine);
	void configIgnoredIndices(float percentageBegining, float percentageEnd);
	void addKeyAction(char key, std::function<void()> action);
	DEBUG(
		void setDebugLayerFunction(std::function<void()> newLayerFunction);
	);

private:
	void config();
};