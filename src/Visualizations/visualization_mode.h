#pragma once
#include <string>
#include <math.h>

#include "ofMain.h"
#include "../utils.h"

#ifdef _DEBUG
#define N_LAYERS 4 //Last layer is for debug
#else
#define N_LAYERS 3
#endif

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
	const size_t _nLayers;

	VisualizationMode(string name, FftConfig* fftConfig, size_t nLayers) : _name(name), _nLayers(nLayers)
	{
		_fft = fftConfig->fft;
		_fftBands = fftConfig->fftBands;
		_nBands = _fftBands;
		LOG("______________________________________________");
		std::cout << "Starting Visualization: " << _name << std::endl;
		assert(nLayers >= 1);
	}
	virtual ~VisualizationMode()
	{
		disableAutoDamper();
		disableAutoRescale();
		disableAutoCombineBands();
		ofSetFrameRate(NULL);
		ofSetLineWidth(1);
		ofFill();
		for (int i = 0; i < N_LAYERS; i++)
		{
			layers[i].destroy();
		}
	}

	void draw()
	{
		//TODO: Vector of functions
		layers[0].begin();
		drawDefaultLayer0();
		layers[0].end();
		if (_nLayers <= 2)
		{
			layers[1].begin();
			drawLayer1();
			layers[1].end();
		}
		if (_nLayers <= 3)
		{
			layers[2].begin();
			drawLayer2();
			layers[2].end();
		}
#ifdef _DEBUG
		if (usesDebugLayer)
		{
			layers[3].begin();
			drawDebugLayer();
			layers[3].end();
		}
#endif
		for (int i = 0; i < N_LAYERS; i++)
		{
			if(layers[i].isAllocated()) layers[i].draw(0, 0);
		}
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
		allocateFboLayers();
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
	virtual void drawDefaultLayer0() = 0;
	virtual void drawLayer1() = 0;
	virtual void drawLayer2() = 0;
#ifdef _DEBUG
	virtual void drawDebugLayer() = 0;
#endif

	void enableDebugLayers()
	{
		usesDebugLayer = true;
		layers[3].allocate(width, height, GL_RGBA);
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
	const float sensibilityDelta = 0.01;

	ofFbo layers[4];
	bool usesDebugLayer = false;

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

	std::vector<std::function<void()>> layerFunctions;
#ifdef _DEBUG
	std::vector<std::function<void()>> debugLayerFunctions;
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

	void allocateFboLayers()
	{
		//TODO: Vector of functions
		layers[0].allocate(width, height, GL_RGBA);
		layers[0].begin();
		ofClear(0);
		layers[0].end();
		if (_nLayers <= 2)
		{
			layers[1].allocate(width, height, GL_RGBA);
			layers[1].begin();
			ofClear(0);
			layers[1].end();
		}
		if (_nLayers <= 3)
		{
			layers[2].allocate(width, height, GL_RGBA);
			layers[2].begin();
			ofClear(0);
			layers[2].end();
		}
#ifdef _DEBUG
		if (usesDebugLayer)
		{
			layers[3].allocate(width, height, GL_RGBA);
			layers[3].begin();
			ofClear(0);
			layers[3].end();
		}
#endif
	}
};