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

//TODO: Organize this class into a cpp
//TODO: Move all post-processing of fft into a new thread
class VisualizationMode
{
public:
	const string _name;

	VisualizationMode(string name, FftConfig* fftConfig) : _name(name)
	{
		_fft = fftConfig->fft;
		_fftBands = fftConfig->fftBands;
		_nBands = _fftBands;
		LOG("______________________________________________");
		std::cout << "Starting Visualization: " << _name << std::endl;
	}
	virtual ~VisualizationMode()
	{
		disableAutoDamper();
		disableAutoRescale();
		disableAutoCombineBands();
		ofSetFrameRate(NULL);
		ofSetLineWidth(1);
		ofFill();
		for (int i = 0; i < layers.size(); i++)
		{
			layers[i].destroy();
		}
#ifdef _DEBUG
		if (debugLayer.isAllocated()) debugLayer.destroy();
#endif
	}

	void draw()
	{
		for (int i = 0; i < layerFunctions.size(); i++)
		{
			layers[i].begin();
			layerFunctions[i]();
			layers[i].end();
			layers[i].draw(0,0);
		}
#ifdef _DEBUG
		if (debugLayerFunction != NULL)
		{
			debugLayer.begin();
			debugLayerFunction();
			debugLayer.end();
			debugLayer.draw(0, 0);
		}
#endif
	}

	void _keyPressed(int key)
	{
		defaultKeyPressed(key);
		keyPressed(key);
	}

	void _keyReleased(int key)
	{
		keyReleased(key);
	}

	void _windowResized()
	{
		_width = ofGetWidth();
		_height = ofGetHeight();
		_halfWidth = _width / 2;
		_halfHeight = _height / 2;
		reallocateLayers();
		windowResized();
	}

	void _update()
	{
		_dt += _dtSpeed / (1 + ofGetFrameRate());
		if (_alwaysRescale) rescaleFft(_RescaleBase, _RescaleMultiplier, _RescalePower);
		if (_alwaysDamp) dampFft(_damper);
		if (_alwaysCombine) combineBands();
		update();
	}

protected:
	float* _fft;
	float* _dampedFft;
	float* _combinedFft;
	
	float _sensibility = 50;
	float _dtSpeed = 1;

	const size_t& fftBands = _fftBands;
	const size_t& ignoreEnd = _ignoreEnd;
	const size_t& nBands = _nBands;
	const size_t& nBands_combined = _nBands_combined;
	const float& dt = _dt;
	const float& width = _width;
	const float& height = _height;
	const float& halfWidth = _halfWidth;
	const float& halfHeight = _halfHeight;

	virtual void keyPressed(int key) = 0;
	virtual void keyReleased(int key) = 0;
	virtual void windowResized() = 0;
	virtual void update() = 0;

	void addLayerFunction(std::function<void()> newLayerFunction)
	{
		layerFunctions.push_back(newLayerFunction);
		ofFbo newLayer;
		newLayer.allocate(width, height, GL_RGBA);
		newLayer.begin();
		ofClear(0);
		newLayer.end();
		layers.push_back(newLayer);
	}
#ifdef _DEBUG
	void setDebugLayerFunction(std::function<void()> newLayerFunction)
	{
		debugLayerFunction = newLayerFunction;
		if(!debugLayer.isAllocated()) debugLayer.allocate(width, height, GL_RGBA);
		debugLayer.begin();
		ofClear(0);
		debugLayer.end();
	}
#endif

	void reallocateLayers()
	{
		for (int i = 0; i < layers.size(); i++)
		{
			layers[i].clear();
			layers[i].allocate(width, height, GL_RGBA);
			layers[i].begin();
			ofClear(0);
			layers[i].end();
		}
#ifdef _DEBUG
		if (debugLayerFunction != NULL)
		{
			debugLayer.clear();
			debugLayer.allocate(width, height, GL_RGBA);
			debugLayer.begin();
			ofClear(0);
			debugLayer.end();
		}
#endif
	}

	void configAutoDamper(float damper)
	{
		_alwaysDamp = true;
		if (damper > 1)
		{
			_damper = damper;
		}
		else
		{
			ERR(" ! Invalid damper value (%f). Default value set (1.2).");
			_damper = 1.2;
		}
		_dampedFft = (float*)malloc(sizeof(float) * _fftBands);
		memset(_dampedFft, 0, sizeof(float)* _fftBands);
		LOG("\t- FFT damper configured: %f", _damper);
	}
	void disableAutoDamper()
	{
		if (!_alwaysDamp) return;
		_alwaysDamp = false;
		free(_dampedFft);
		LOG("- Auto Damper disabled.");
	}

	float fpsRelativeDamp(float _damper)
	{
		return 1 + (_damper - 1) * 60 / (ofGetFrameRate() + 10);
	}

	void configAutoRescale(float base, float multiplier, float power)
	{
		_alwaysRescale = true;
		_RescaleBase = base;
		_RescaleMultiplier = multiplier;
		_RescalePower = power;
		LOG("\t- Auto Rescale enabled. Values:");
		LOG("\t\t - Base %f", _RescaleBase);
		LOG("\t\t - Multiplier %f", _RescaleMultiplier);
		LOG("\t\t - Power %f", _RescalePower);
	}

	void disableAutoRescale()
	{
		_alwaysRescale = false;
		LOG("- Auto Rescale disabled.");
	}

	void configAutoCombineBands(size_t nBandsToCombine)
	{
		if (nBandsToCombine < 1)
		{
			ERR("Invalid value for nBandsToCombine (%d). Values should be bigger then 1.", nBandsToCombine);
			return;
		}
		_nCombinedBands = nBandsToCombine;
		_nBands_combined = _nBands / _nCombinedBands;

		if (_nBands_combined < 3)
		{
			ERR("Invalid value for nBandsToCombine (%d). Value might be too big.", nBandsToCombine);
			return;
		}
		_combinedFft = (float*)malloc(sizeof(float) * _nBands_combined);
		_alwaysCombine = true;
	}

	void disableAutoCombineBands()
	{
		if (!_alwaysCombine) return;
		_alwaysCombine = false;
		free(_combinedFft);
		LOG("- Auto Combine Bands disabled.");
	}

	void configIgnoredIndices(float percentageBegining, float percentageEnd)
	{
		if (percentageBegining + percentageEnd > 0.95)
		{
			ERR(" ! 95% of the FFT bands CAN'T be ignored.\n Pick values where added together are less then 0.95.");
			return;
		}
		if(percentageBegining < 0 || percentageEnd < 0)
		{
			ERR(" ! Only positive values are acceptable.");
			return;
		}
		LOG("TODO: percentageBegining not yet implemented.");
		_ignoreEnd = _fftBands * percentageEnd;
		_nBands = _fftBands - _ignoreEnd;
		LOG("\t- Ignoring %d indices at the end of FFT.", _ignoreEnd);
	}

private:
	const float sensibilityDelta = 0.01; //1%

	size_t _fftBands;
	
	float _dt = ofRandom(255);

	bool _alwaysDamp = false;
	float _damper = 1.2;

	bool _alwaysRescale = false;
	float _RescaleBase = 1;
	float _RescaleMultiplier = 0;
	float _RescalePower = 1;

	size_t _ignoreEnd = 0;
	size_t _nBands;

	bool _alwaysCombine = false;
	size_t _nCombinedBands = 1;
	size_t _nBands_combined;

	float _width, _height, _halfWidth, _halfHeight;

	std::vector<ofFbo> layers;
	std::vector<std::function<void()>> layerFunctions;
#ifdef _DEBUG
	ofFbo debugLayer;
	std::function<void()> debugLayerFunction = NULL;
#endif

	void defaultKeyPressed(int key)
	{
		switch (key)
		{
		case 'w':
			_sensibility *= 1 + sensibilityDelta;
			std::cout << "\t- Sensibility = " << _sensibility << std::endl;
			break;
		case 's':
			_sensibility *= 1 - sensibilityDelta;
			std::cout << "\t- Sensibility = " << _sensibility << std::endl;
			break;
		}
	}

	void dampFft(float damp)
	{
		for (int i = 0; i < _fftBands - _ignoreEnd; i++)
		{
			_dampedFft[i] /= fpsRelativeDamp(damp);
			if (_dampedFft[i] < _fft[i]) _dampedFft[i] = _fft[i];
		}
	}

	void rescaleFft(float base, float multiplier, float power)
	{
		for (int i = 0; i < _fftBands - _ignoreEnd; i++)
		{
			_fft[i] = (base + i * multiplier) * pow(_fft[i], power);
		}
	}

	void combineBands()
	{
		float* fftReference;
		if (_alwaysDamp) fftReference = _dampedFft;
		else fftReference = _fft;
		for (int i = 0; i < _nBands_combined; i++)
		{
			_combinedFft[i] = 0;
			for (int j = 0; j < _nCombinedBands; j++)
			{
				_combinedFft[i] += fftReference[_nCombinedBands * i + j];
			}
			_combinedFft[i] /= _nCombinedBands;
		}
	}
};