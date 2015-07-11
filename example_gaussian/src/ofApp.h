#pragma once

#include "ofMain.h"
#include "ofxGPGPU.h"
#include "shaders/gaussian.h"

class ofApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    gpgpu::Process gaussian;
    gpgpu::Gaussian gaussian_shader;
    void update_gaussian( ofShader& shader );

    ofVideoGrabber video;

};
