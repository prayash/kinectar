//
//  fft_manager.h
//  Kinectar
//
//  Created by Prayash Thapa on 4/14/18.
//
#pragma once

#ifndef fft_manager_h
#define fft_manager_h

#include <stdio.h>
#include "ofMain.h"

class FFTManager {
public:
    void setup(ofBaseApp *app);
    void update();
    void audioIn(float * input, int bufferSize, int nChannels);
    
    vector<float> left;
    vector<float> right;
    vector<float> volHistory;
    
    int bufferCounter;
    
    float smoothedVol;
    float scaledVol;
    
    ofSoundStream soundStream;
};

#endif /* fft_manager_h */
