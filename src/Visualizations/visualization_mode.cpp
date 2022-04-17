#include "visualization_mode.h"

//TODO: Move all post-processing of fft into a new thread
VisualizationMode::VisualizationMode(string name, FftConfig* fftConfig) : _name(name)
{
	_fft = fftConfig->fft;
	_nBands = fftConfig->fftBands;
	LOG("______________________________________________");
	std::cout << "Starting Visualization: " << _name << std::endl;
	config();
}

VisualizationMode::~VisualizationMode()
{
	ofSetFrameRate(NULL);
	ofSetLineWidth(1);
	ofFill();
	for (int i = 0; i < layers.size(); i++)
	{
		layers[i].destroy();
	}
	DEBUG(
		if (debugLayer.isAllocated()) debugLayer.destroy();
	);
}

void VisualizationMode::config()
{
	// Config default keys
	keyActions['w'] = [this] {
		_sensibility *= 1 + sensibilityDelta;
		std::cout << "\t- Sensibility = " << _sensibility << std::endl;
	};
	keyActions['s'] = [this] {
		_sensibility *= 1 - sensibilityDelta;
		std::cout << "\t- Sensibility = " << _sensibility << std::endl;
	};
}

void VisualizationMode::draw()
{
	// Draws every layer
	for (int i = 0; i < layerFunctions.size(); i++)
	{
		layers[i].begin();
		layerFunctions[i]();
		layers[i].end();
		layers[i].draw(0, 0);
	}

	// Debug layers
	DEBUG(
		if (debugLayerFunction != NULL)
		{
			debugLayer.begin();
			debugLayerFunction();
			debugLayer.end();
			debugLayer.draw(0, 0);
		}
	);
}
void VisualizationMode::addKeyAction(char key, std::function<void()> action)
{
	if (keyActions.find(key) == keyActions.end())
	{
		keyActions[key] = action;
	}
	else
	{
		ERR("Key already register in actions.");
	}
}

void VisualizationMode::_keyPressed(int key)
{
	auto action = keyActions.find(key);
	if (action != keyActions.end()) {
		action->second();
	}
}

void VisualizationMode::_windowResized()
{
	_width = ofGetWidth();
	_height = ofGetHeight();
	_halfWidth = _width / 2;
	_halfHeight = _height / 2;
	reallocateLayers();
	windowResized();
}

void VisualizationMode::_update()
{
	_dt += _dtSpeed / (1 + ofGetFrameRate());
	for (auto& ppf : postProcessingFunctions) {
		ppf->execute();
	}
	update();
}

void VisualizationMode::addLayerFunction(std::function<void()> newLayerFunction)
{
	layerFunctions.push_back(newLayerFunction);
	ofFbo newLayer;
	newLayer.allocate(width, height, GL_RGBA);
	newLayer.begin();
	ofClear(0);
	newLayer.end();
	layers.push_back(newLayer);
}

void VisualizationMode::reallocateLayers()
{
	for (int i = 0; i < layers.size(); i++)
	{
		layers[i].clear();
		layers[i].allocate(width, height, GL_RGBA);
		layers[i].begin();
		ofClear(0);
		layers[i].end();
	}
	DEBUG(
		if (debugLayerFunction != NULL)
		{
			debugLayer.clear();
			debugLayer.allocate(width, height, GL_RGBA);
			debugLayer.begin();
			ofClear(0);
			debugLayer.end();
		}
	);
}
void VisualizationMode::configAutoDamper(float damper)
{
	LOG("\t- Configuring FFT damper:");
	if (damper <= 1)
	{
		ERR(" ! Invalid damper value (%f). Default value set (1.2).");
		damper = 1.2;
	}
	LOG("\t\t- Damping value: %f", damper);
	PostProcessingRef* damp = new PostProcessingRef("Damp", &_fft, &_nBands, _nBands);
	damp->registerFunction(
		[this, damp, damper] {
			for (int i = 0; i < damp->_nBandsRefToFunction; i++)
			{
				damp->_fftResultToFunction[i] /= fpsRelativeDamp(damper);
				if (damp->_fftResultToFunction[i] < damp->_fftRefToFunction[i])
					damp->_fftResultToFunction[i] = damp->_fftRefToFunction[i];
			}
		}
	);
	postProcessingFunctions.push_back(damp);
}

float VisualizationMode::fpsRelativeDamp(float _damper)
{
	return 1 + (_damper - 1) * 60 / (ofGetFrameRate() + 10);
}

void VisualizationMode::configAutoRescale(float base, float multiplier, float power)
{
	LOG("\t- Configuring FFT rescale:");
	LOG("\t\t- Base %f", base);
	LOG("\t\t- Multiplier %f", multiplier);
	LOG("\t\t- Power %f", power);
	PostProcessingRef* rescale = new PostProcessingRef("Rescale", &_fft, &_nBands, _nBands);
	rescale->registerFunction(
		[this, rescale, base, multiplier, power] {
			for (int i = 0; i < rescale->_nBandsRefToFunction; i++)
			{
				rescale->_fftResultToFunction[i] = (base + i * multiplier) * pow(rescale->_fftRefToFunction[i], power);
			}
		}
	);
	postProcessingFunctions.push_back(rescale);
}

void VisualizationMode::configAutoCombineBands(size_t nBandsToCombine)
{
	if (nBandsToCombine < 1)
	{
		ERR("Invalid value for nBandsToCombine (%d). Values should be bigger then 1.", nBandsToCombine);
		return;
	}
	size_t nCombinedBands = _nBands / nBandsToCombine;

	if (nCombinedBands < 3)
	{
		ERR("Invalid value for nBandsToCombine (%d). Value might be too big.", nBandsToCombine);
		return;
	}

	LOG("\t- Auto Combine configured.");
	LOG("\t\t- Combining %d bands", nBandsToCombine);
	PostProcessingRef* combine = new PostProcessingRef("Combine", &_fft, &_nBands, nCombinedBands);
	combine->registerFunction(
		[combine, nBandsToCombine] {
			for (int i = 0; i < combine->_nBandsResultToFunction; i++)
			{
				combine->_fftResultToFunction[i] = 0;
				for (int j = 0; j < nBandsToCombine; j++)
				{
					combine->_fftResultToFunction[i] += combine->_fftRefToFunction[nBandsToCombine * i + j];
				}
				combine->_fftResultToFunction[i] /= nBandsToCombine;
			}
		}
	);
	postProcessingFunctions.push_back(combine);
}

void VisualizationMode::configIgnoredIndices(float percentageBegining, float percentageEnd)
{
	if (percentageBegining + percentageEnd > 0.95)
	{
		ERR(" ! 95% of the FFT bands CAN'T be ignored.\n Pick values where added together are less then 0.95.");
		return;
	}
	if (percentageBegining < 0 || percentageEnd < 0)
	{
		ERR(" ! Only positive values are acceptable.");
		return;
	}
	LOG("\t- Configuring FFT band reduction:");
	LOG("\t\t- Current N of Bands: %d", _nBands);
	LOG("TODO: percentageBegining not yet implemented.");
	LOG("\t\t- Ignoring in Begining: %.2f%%", 100 * percentageBegining);
	LOG("\t\t- Ignoring in End: %.2f%%", 100 * percentageEnd);
	_nBands -= _nBands * percentageEnd;
	LOG("\t\t- New N of Bands: %d", _nBands);
}

DEBUG(
	void VisualizationMode::setDebugLayerFunction(std::function<void()> newLayerFunction)
	{
		debugLayerFunction = newLayerFunction;
		if (!debugLayer.isAllocated()) debugLayer.allocate(width, height, GL_RGBA);
		debugLayer.begin();
		ofClear(0);
		debugLayer.end();
	}
);