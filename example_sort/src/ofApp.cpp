#include "ofApp.h"


int w = 5;
int h = 5;


void ofApp::setup(){

    ofBackground(0);
    ofSetFrameRate(60);

    int channels = 4;
    float* data = new float[w * h * channels];
    for (int y = 0; y < h; ++y)
    {
      for (int x = 0; x < w; ++x)
      {
        int i = (y * w + x) * channels;
        data[i + 0] = ofRandom(1.0);
        data[i + 1] = ofRandom(1.0);
        data[i + 2] = ofRandom(1.0);
        data[i + 3] = ofRandom(1.0);
      }
    }

    sort.init( &oddevenmergesort, w, h );

    // set initial state
    sort.set("data", data);

    //ofAddListener( sort.on_update, this, &gpgpu::OddEvenMergeSort::update );

    ofLog() << "initial state";
    sort.log("data");
}

void ofApp::update(){

  ofSetWindowTitle( ofToString( ofGetFrameRate(), 2 ) );

  sort.update();
  sort.log("data");

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

  if ( key == ' ')
    sort.log("data");

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
