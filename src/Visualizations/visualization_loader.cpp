#include "logging.h"
#include "visualization_loader.h"
#include "visualization_list.h"

#include <algorithm>

#define RANDOM_VIS

VisualizationLoader::VisualizationLoader(AudioSink* sink)
{
    vm = nullptr;
    _sink = sink;
    fftBands = sink->GetFftDimensions();
    fft = (float*)malloc(sizeof(float) * fftBands);
    memset(fft, 0, sizeof(float) * fftBands);
    fftConfig.fft = fft;
    fftConfig.fftBands = fftBands;

    for (int i = 0; i < N_VIS; i++)
    {
        visOrder[i] = i;
    }
#if defined(_DEVELOPING_) && defined(_DEBUG)
    vm = (VisualizationMode*) new _DEVELOPING_(&fftConfig);
#elif defined(RANDOM_VIS);
    random_shuffle(std::begin(visOrder), std::end(visOrder));
    NextVisualization(visIndex);
    ofResetElapsedTimeCounter();
#else
    SwitchVisualization(0);
#endif
    configLoader();
}
VisualizationLoader::~VisualizationLoader()
{
    keyActions.clear();
    delete vm;
    free(fft);
}

void VisualizationLoader::setup() {}
void VisualizationLoader::update() {
    std::stringstream strm;
    strm << vm->_name << " - FPS: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}
void VisualizationLoader::draw() {
    _sink->GetFftData(fft);
    vm->_update();
    vm->draw();
    if (ofGetElapsedTimef() >= 600)
    {
        NextVisualization(visIndex);
    }
}
void VisualizationLoader::keyPressed(int key) {
    auto action = keyActions.find(key);
    if (action != keyActions.end()) {
        action->second();
    }
    else
    {
        vm->_keyPressed(key);
    }
}
void VisualizationLoader::windowResized(int w, int h) {
    vm->_windowResized();
}
void VisualizationLoader::NextVisualization(int& index)
{
    index = (index + 1)% N_VIS;
    SwitchVisualization(visOrder[index]);
    ofResetElapsedTimeCounter();
}

void VisualizationLoader::SwitchVisualization(int index)
{
    index = index % N_VIS;
    if(vm != nullptr) delete vm;
    LOG("Switching to %d", index);
    switch (index)
    {
    case 0: // RainbowBars
        vm = (VisualizationMode*) new RainbowBars(&fftConfig);
        break;
    case 1: // BarsReflections
        vm = (VisualizationMode*) new BarsReflection(&fftConfig);
        break;
    case 2: // SuperNova
        vm = (VisualizationMode*) new SuperNova(&fftConfig);
        break;
    case 3: // DancingWaves
        vm = (VisualizationMode*) new DancingWaves(&fftConfig);
        break;
    case 4: // SolarSystem
        vm = (VisualizationMode*) new MilkWay(&fftConfig);
        break;
    case 5: // FloatingDiamonds
        vm = (VisualizationMode*) new FloatingDiamonds(&fftConfig);
        break;
    case 6: // Portal
        vm = (VisualizationMode*) new Portal(&fftConfig);
        break;

    // Default
    default:
        vm = (VisualizationMode*) new RainbowBars(&fftConfig);
        break;
    }
}

void VisualizationLoader::configLoader()
{
    keyActions['p'] = [this] {NextVisualization(visIndex); };
    keyActions['f'] = [this] {
        ofToggleFullscreen();
        if (ofGetWindowMode() == OF_FULLSCREEN) ofHideCursor();
        else ofShowCursor(); 
    };
}

// Unused functions
void VisualizationLoader::keyReleased(int key) {}
void VisualizationLoader::mouseMoved(int x, int y) {}
void VisualizationLoader::mouseDragged(int x, int y, int button) {}
void VisualizationLoader::mousePressed(int x, int y, int button) {}
void VisualizationLoader::mouseReleased(int x, int y, int button) {}
void VisualizationLoader::mouseEntered(int x, int y) {}
void VisualizationLoader::mouseExited(int x, int y) {}
void VisualizationLoader::gotMessage(ofMessage msg) {}
void VisualizationLoader::dragEvent(ofDragInfo dragInfo) {}