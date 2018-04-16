//
//  fft_manager.cpp
//  Kinectar
//
//  Created by Prayash Thapa on 4/14/18.
//

#include "fft_manager.h"

void FFTManager::setup(ofBaseApp *app) {
    soundStream.printDeviceList();
    
    // The device id corresponds to all audio devices, including input-only and output-only
    // soundStream.setDeviceID(0);
    
    int bufferSize = 256;
    
    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    volHistory.assign(192, 0.0);
    
    bufferCounter = 0;
    smoothedVol = 0.0;
    scaledVol = 0.0;
    
    // 0 output channels, 2 input channels, 44100 samples per second
    // 256 samples per buffer and 4 num buffers (latency)
    soundStream.setup(app, 0, 2, 44100, bufferSize, 4);
    soundStream.start();
}

void FFTManager::update() {
    scaledVol = ofMap(smoothedVol, 0.0, 1.0, 0.0, 1.0, true);
    
    volHistory.push_back(scaledVol);
    
    if (volHistory.size() >= 192) {
        volHistory.erase(volHistory.begin(), volHistory.begin() + 1);
    }
    
//    ofLog(OF_LOG_NOTICE, "The current scaled volumed is: " + ofToString(fftManager.scaledVol));
}

void FFTManager::audioIn(float *input, int bufferSize, int nChannels) {
    float curVol = 0.0;
    
    int numCounted = 0;
    
    for (int i = 0; i < bufferSize; i++) {
        left[i] = input[i * 2] * 0.5;
        right[i] = input[i * 2 + 1] * 0.5;
        
        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted += 2;
    }
    
    curVol /= (float) numCounted;
    curVol = sqrt(curVol);
    
    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;
    bufferCounter++;
}
