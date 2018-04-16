#include "kinectar.h"

// ***************************************************************
void Kinectar::setup() {
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    loadAssets();
    setupGUI();
    fftManager.setup(this);
    
    // Enable depth -> video image calibration
    kinect.setRegistration(true);
    
    kinect.init();
    
    // Show infrared instead of RGB video image
    // kinect.init(true);
    
    // Disable video image (faster fps)
    // kinect.init(false, false);
    
    kinect.open();
    kinect.setDepthClipping(KINECT_SENSOR_NEAR_LIMIT, KINECT_SENSOR_FAR_LIMIT);
    
    blob.allocate(KINECT_RES_WIDTH, KINECT_RES_HEIGHT, OF_IMAGE_GRAYSCALE);
    
    // Intrinsic IR sensor values
    if (kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }
    
    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    
    nearThreshold = 230;
    farThreshold = 70;
    bThreshWithOpenCV = true;
    
    // Post Processing
    postFx.init(ofGetWidth(), ofGetHeight());
    postFx.createPass<BloomPass>();
    postFx.createPass<FxaaPass>();
}

// ***************************************************************
void Kinectar::update() {
    kinect.update();
    fftManager.update();
    
    if (kinect.isFrameNew()) {
        calculateDelaunay();
        
        // load grayscale depth image from the kinect source
        grayImage.setFromPixels(kinect.getDepthPixels());
        
        // we do two thresholds - one for the far plane and one for the near plane
        // we then do a cvAnd to get the pixels which are a union of the two thresholds
        if (bThreshWithOpenCV) {
            grayThreshNear = grayImage;
            grayThreshFar = grayImage;
            grayThreshNear.threshold(nearThreshold, true);
            grayThreshFar.threshold(farThreshold);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
        } else {
            ofPixels &pix = grayImage.getPixels();
            int numPixels = (int) pix.size();
            
            for (int i = 0; i < numPixels; i++) {
                if (pix[i] < nearThreshold && pix[i] > farThreshold) {
                    pix[i] = 255;
                } else {
                    pix[i] = 0;
                }
            }
        }
        
        // update the cv images
        grayImage.flagImageChanged();
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayImage, 10, (kinect.width * kinect.height) / 2, 20, false);
    }
}

// ***************************************************************
void Kinectar::draw() {
    ofBackgroundHex(0xfafafa);
    glEnable(GL_DEPTH_TEST);
    
    // Draw instructions
    ofSetColor(255, 255, 255);
    stringstream reportStream;
    
    if (kinect.hasAccelControl()) {
        reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
        << ofToString(kinect.getMksAccel().y, 2) << " / "
        << ofToString(kinect.getMksAccel().z, 2) << endl;
    } else {
        reportStream << "Note: this is a newer Xbox Kinect or Kinect For Windows device," << endl
        << "motor / led / accel controls are not currently supported" << endl << endl;
    }
    
    reportStream << "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
    << "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
    << "set near threshold " << nearThreshold << " (press: + -)" << endl
    << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
    << ", fps: " << ofGetFrameRate() << endl
    << "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl;
    
    if (kinect.hasCamTiltControl()) {
        reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
        << "press 1-5 & 0 to change the led mode" << endl;
    }
    
    ofDrawBitmapString(reportStream.str(), 20, 652);
    
    logo.draw(ofGetWidth() - 120, ofGetHeight() - 140, 100, 100);
    font.drawString("K I N E C T A R", ofGetWidth() - 110, ofGetHeight() - 20);
    ofDrawRectangle(ofGetWidth() - 120, ofGetHeight() - 27.5, 5, 5);
    
    // Draw the average volume
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(ofGetWidth() - 200, 8, 0);
    
    ofDrawRectangle(0, 0, 192, 24);
    ofSetColor(245, 58, 135);
    ofFill();
    
    // Volume history
    ofBeginShape();
    for (unsigned int i = 0; i < fftManager.volHistory.size(); i++) {
        if (i == 0) {
            ofVertex(i, 24);
        }
        
        ofVertex(i, 24 - fftManager.volHistory[i] * 200);
        
        if (i == fftManager.volHistory.size() - 1) {
            ofVertex(i, 24);
        }
    }
    
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    
    if (bTriangulate) {
        ofPushMatrix();
        
        camera.begin();
        camera.setScale(1, -1, 1);
        
        ofSetColor(255, 255, 255);
        ofTranslate(0, -80, 1100);
        ofFill();
        
        postFx.begin();
        
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glShadeModel(GL_FLAT);
        glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
        convertedMesh.drawFaces();
        glShadeModel(GL_SMOOTH);
        glPopAttrib();
        
        if (useRealColors) {
            ofSetColor(30, 30, 30, 255);
        } else {
            ofSetColor(124, 136, 128, 255);
        }
        
        ofPushMatrix();
        ofTranslate(0, 0, 0.5);
        wireframeMesh.drawWireframe();
        ofPopMatrix();
        camera.end();
        ofPopMatrix();
        
        postFx.end();
    }
    
    // Point Cloud
    
    if (bDrawPointCloud) {
        camera.begin();
        camera.setScale(1, -1, 1);
        drawPointCloud();
        camera.end();
    } else {
        kinect.drawDepth(10, 10, 400, 300);
        kinect.draw(420, 10, 400, 300);
        
        grayImage.draw(10, 320, 400, 300);
        contourFinder.draw(10, 320, 400, 300);
    }
    
    gui.draw();
}

void Kinectar::drawPointCloud() {
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_POINTS);
    
    int step = 2;
    for (int y = 0; y < kinect.getWidth(); y += step) {
        for (int x = 0; x < kinect.getHeight(); x += step) {
            float dist = kinect.getDistanceAt(x, y);
            
            if (dist > 0 && dist < 1000) {
                if (bPointCloudColor) {
                    mesh.addColor(kinect.getColorAt(x, y));
                }
                
                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
            }
        }
    }
    
    ofSetColor(245, 58, 135);
    
    GLfloat pointSize = MAX(2, fftManager.scaledVol * 100);
    glPointSize(pointSize);
    ofPushMatrix();
    
    // The projected points are 'upside down' and 'backwards'
    ofScale(1, -1, -1);
    
    // Center the points a bit
    ofTranslate(0, 0, -1000);
    ofEnableDepthTest();
    mesh.drawVertices();
    ofDisableDepthTest();
    ofPopMatrix();
}

// ***************************************************************
void Kinectar::exit() {
    kinect.close();
}

// ***************************************************************
void Kinectar::keyPressed(int key) {
    switch (key) {
        case ' ':
            bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
            
        case'p':
            bDrawPointCloud = !bDrawPointCloud;
            break;
            
        case '>':
        case '.':
            farThreshold ++;
            if (farThreshold > 255) farThreshold = 255;
            break;
            
        case '<':
        case ',':
            farThreshold --;
            if (farThreshold < 0) farThreshold = 0;
            break;
            
        case '+':
        case '=':
            nearThreshold ++;
            if (nearThreshold > 255) nearThreshold = 255;
            break;
            
        case '-':
            nearThreshold --;
            if (nearThreshold < 0) nearThreshold = 0;
            break;
            
        case 'w':
            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
            break;
            
        case '1':
            kinect.setLed(ofxKinect::LED_GREEN);
            break;
            
        case '2':
            kinect.setLed(ofxKinect::LED_YELLOW);
            break;
            
        case '3':
            kinect.setLed(ofxKinect::LED_RED);
            break;
            
        case '4':
            kinect.setLed(ofxKinect::LED_BLINK_GREEN);
            break;
            
        case '5':
            kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
            break;
            
        case '0':
            kinect.setLed(ofxKinect::LED_OFF);
            break;
            
        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
    }
}

// ***************************************************************
void Kinectar::audioIn(float * input, int bufferSize, int nChannels){
    fftManager.audioIn(input, bufferSize, nChannels);
}

// ***************************************************************
void Kinectar::mouseDragged(int x, int y, int button) {
    
}

// ***************************************************************
void Kinectar::mousePressed(int x, int y, int button) {
    click.play();
}

// ***************************************************************
void Kinectar::mouseReleased(int x, int y, int button) {
    
}

// ***************************************************************
void Kinectar::mouseEntered(int x, int y) {
    
}

// ***************************************************************
void Kinectar::mouseExited(int x, int y) {
    
}

// ***************************************************************
void Kinectar::windowResized(int w, int h) {
    
}

// ***************************************************************
void Kinectar::loadAssets() {
    logo.load("thumb.png");
    font.load("type.ttf", 12);
    
    click.load("click.wav");
    click.setVolume(0.25);
    click.setLoop(false);
    click.setSpeed(2.0);
}

void Kinectar::setupGUI() {
    gui.setup();
    gui.setDefaultTextPadding(4);
    gui.loadFont("type.ttf", 10);
    gui.add(intSlider.setup("int slider", 0, 0, 300));
    gui.add(floatSlider.setup("floatslider", 33.33, 0.0, 66.66));
    gui.add(bDrawPointCloud.setup("Point Cloud Render", true));
    gui.add(bPointCloudColor.setup("Point Cloud Color", false));
    gui.add(bTriangulate.setup("Triangulate", false));
    gui.add(noiseAmount.setup("Noise Amount", 0.0, 0.0, 20.0));
    gui.add(pointSkip.setup("Point Skip", 5, 4, 20));
    gui.add(useRealColors.setup("Real Colors", true));
    gui.add(colorAlpha.setup("Color Alpha", 200, 0, 255));
    gui.setPosition(ofGetWidth() - 204, 36);
    gui.setName("Point Cloud");
}

void Kinectar::calculateDelaunay() {
    del.reset();
    
    unsigned char* pix = new unsigned char[640 * 480];
    unsigned char* gpix = new unsigned char[640 * 480];
    
    for (int x = 0; x < 640; x++) {
        for (int y = 0; y < 480; y++) {
            float distance = kinect.getDistanceAt(x, y);
            int pIndex = x + y * 640;
            pix[pIndex] = 0;
            
            if (distance > 100 && distance < 1100) {
                pix[pIndex] = 255;
            }
        }
    }
    
    blob.setFromPixels(pix, 640, 480, OF_IMAGE_GRAYSCALE);
    int numPoints = 0;
    
    for (int x = 0; x < 640; x += pointSkip * 2) {
        for (int y = 0; y < 480; y += pointSkip * 2) {
            int pIndex = x + 640 * y;
            
            if (blob.getPixels()[pIndex] > 0) {
                ofVec3f wc = kinect.getWorldCoordinateAt(x, y);
                
                wc.x = x - 320.0;
                wc.y = y - 240.0;
                
                if (abs(wc.z) > 100 && abs(wc.z ) < 2000) {
                    wc.z = -wc.z;
                    
                    wc.x += ofSignedNoise(wc.x,wc.z) * noiseAmount;
                    wc.y += ofSignedNoise(wc.y,wc.z) * noiseAmount;
                    
                    wc.x = ofClamp(wc.x, -320, 320);
                    wc.y = ofClamp(wc.y, -240, 240);
                    
                    del.addPoint(wc);
                }
                
                numPoints++;
            }
        }
    }
    
    
    if (numPoints > 0)
        del.triangulate();
    
    for (int i = 0; i < del.triangleMesh.getNumVertices(); i++) {
        del.triangleMesh.addColor(ofColor(0, 0, 0));
    }
    
    for (int i = 0; i < del.triangleMesh.getNumIndices() / 3; i++) {
        ofVec3f v = del.triangleMesh.getVertex(del.triangleMesh.getIndex(i * 3));
        
        v.x = ofClamp(v.x, -319, 319);
        v.y = ofClamp(v.y, -239, 239);
        
        ofColor c = useRealColors ? kinect.getColorAt(v.x + 320.0, v.y + 240.0) : ofColor(255, 0, 0);
        
        c.a = colorAlpha;
        
        del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3),c);
        del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3+1),c);
        del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3+2),c);
    }
    
    
    convertedMesh.clear();
    wireframeMesh.clear();
    wireframeMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    for (int i = 0; i <del.triangleMesh.getNumIndices() / 3; i++) {
        int indx1 = del.triangleMesh.getIndex(i * 3);
        ofVec3f p1 = del.triangleMesh.getVertex(indx1);
        int indx2 = del.triangleMesh.getIndex(i * 3 + 1);
        ofVec3f p2 = del.triangleMesh.getVertex(indx2);
        int indx3 = del.triangleMesh.getIndex(i * 3 + 2);
        ofVec3f p3 = del.triangleMesh.getVertex(indx3);
        
        ofVec3f triangleCenter = (p1 + p2 + p3) / 3.0;
        triangleCenter.x += 320;
        triangleCenter.y += 240;
        
        triangleCenter.x = floor(ofClamp(triangleCenter.x, 0,640));
        triangleCenter.y = floor(ofClamp(triangleCenter.y, 0, 480));
        
        int pixIndex = triangleCenter.x + triangleCenter.y * 640;
        if (pix[pixIndex] > 0) {
            convertedMesh.addVertex(p1);
            convertedMesh.addColor(del.triangleMesh.getColor(indx1));
            
            convertedMesh.addVertex(p2);
            convertedMesh.addColor(del.triangleMesh.getColor(indx2));
            
            convertedMesh.addVertex(p3);
            convertedMesh.addColor(del.triangleMesh.getColor(indx3));
            
            wireframeMesh.addVertex(p1);
            wireframeMesh.addVertex(p2);
            wireframeMesh.addVertex(p3);
        }
    }
    
    delete[] pix;
    delete[] gpix;
}
