#include "logging.h"
#include "visualization_loader.h"
#include "visualization_list.h"

#define RANDOM_VIS
#define N_VIS 7

VisualizationLoader::VisualizationLoader(AudioSink* sink)
{
    vm = nullptr;
    _sink = sink;
    fftBands = sink->GetFftDimensions();
    fft = (float*)malloc(sizeof(float) * fftBands);
    memset(fft, 0, sizeof(float) * fftBands);
    fftConfig.fft = fft;
    fftConfig.fftBands = fftBands;

#if defined(_DEVELOPING_) && defined(_DEBUG)
    vm = (VisualizationMode*) new _DEVELOPING_(&fftConfig);
#elif defined(_TEST_MODE_) && defined(_DEBUG)
    vm = (VisualizationMode*) new TEST_MODE(&fftConfig);
#elif defined(RANDOM_VIS);
    visIndex = ofRandom(0, N_VIS + 1);
    SwitchVisualization(visIndex);
    ofResetElapsedTimeCounter();
#else
    SwitchVisualization(0);
#endif

}
void VisualizationLoader::setup() {
}
void VisualizationLoader::update() {
    std::stringstream strm;
    strm << vm->_name << " - FPS: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());
}
void VisualizationLoader::draw() {
    _sink->GetFftData(fft);
    vm->_update();
    vm->draw();
#ifdef RANDOM_VIS
    if (ofGetElapsedTimef() >= 600)
    {
        int newVis;
        do
        {
            newVis = ofRandom(0, N_VIS + 1);
        }
        while (newVis == visIndex);
        visIndex = newVis;
        SwitchVisualization(visIndex);
        ofResetElapsedTimeCounter();
    }
#endif
}
void VisualizationLoader::keyPressed(int key) {
    vm->_keyPressed(key);
    if (key == 'p') NextVisualization(visIndex);
    if (key == 'f')
    {
        ofToggleFullscreen();
        if (ofGetWindowMode() == OF_FULLSCREEN) ofHideCursor();
        else ofShowCursor();
    }
}
void VisualizationLoader::keyReleased(int key) {
    vm->_keyReleased(key);
}
void VisualizationLoader::windowResized(int w, int h) {
    vm->_windowResized();
}
void VisualizationLoader::NextVisualization(int& index)
{
    index = (index + 1)% N_VIS;
    SwitchVisualization(index);
}

void VisualizationLoader::SwitchVisualization(int index)
{
    index = index % N_VIS;
    if(vm != nullptr) delete vm;
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

// Unused functions
void VisualizationLoader::mouseMoved(int x, int y) {}
void VisualizationLoader::mouseDragged(int x, int y, int button) {}
void VisualizationLoader::mousePressed(int x, int y, int button) {}
void VisualizationLoader::mouseReleased(int x, int y, int button) {}
void VisualizationLoader::mouseEntered(int x, int y) {}
void VisualizationLoader::mouseExited(int x, int y) {}
void VisualizationLoader::gotMessage(ofMessage msg) {}
void VisualizationLoader::dragEvent(ofDragInfo dragInfo) {}
