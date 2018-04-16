#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxGui.h"
#include "ofxDelaunay.h"
#include "ofxPostProcessing.h"
#include "fft_manager.h"

#define KINECT_RES_WIDTH  (640)
#define KINECT_RES_HEIGHT (480)

#define KINECT_SENSOR_NEAR_LIMIT (400.0f)
#define KINECT_SENSOR_FAR_LIMIT  (4000.0f)

// ***************************************************************

class Kinectar : public ofBaseApp {
public:
    
    void setup();
    void update();
    void draw();
    void exit();
    
    void drawPointCloud();
    void loadAssets();
    void setupGUI();
    void calculateDelaunay();
    
    void keyPressed(int key);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    
    void audioIn(float * input, int bufferSize, int nChannels);
    
    ofxKinect kinect;
    
    ofImage logo;
    ofTrueTypeFont font;
    ofSoundPlayer click;
    
    FFTManager fftManager;
    
    ofxPanel gui;
    ofxIntSlider intSlider;
    ofxFloatSlider floatSlider;
    ofxToggle bDrawPointCloud;
    ofxToggle bPointCloudColor;
    ofxToggle bTriangulate;
    ofxSlider<int> colorAlpha;
    ofxSlider<float> noiseAmount;
    ofxToggle useRealColors;
    ofxSlider<int> pointSkip;
    
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    
    ofxCvContourFinder contourFinder;
    
    bool bThreshWithOpenCV;
    
    int nearThreshold;
    int farThreshold;
    
    int angle = 30;
    ofEasyCam camera;
    
    ofMesh convertedMesh;
    ofMesh wireframeMesh;
    
    ofxDelaunay del;
    
    ofImage blob;
    
    ofxPostProcessing postFx;
};
