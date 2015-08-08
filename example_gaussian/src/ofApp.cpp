#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

  int w = 640;
  int h = 480;

  video.setDeviceID(0);
	video.setDesiredFrameRate(30);
	video.initGrabber( w, h, true );

  gaussian
    .init( w, h )
    .on( "update", this, &ofApp::update_gaussian );
}

//--------------------------------------------------------------
void ofApp::update(){
  video.update();
  if ( video.isFrameNew() )
  {
    gaussian
      .set( "data", video.getTextureReference() )
      .update( 8 ); //horiz + vert
  }
}

void ofApp::update_gaussian( ofShader& shader )
{
  shader.setUniform1f( "sigma", ofMap( ofGetMouseX(), 0, ofGetWidth(), 0, 10 ) );
  shader.setUniform1i( "kernel", ofMap( ofGetMouseY(), 0, ofGetHeight(), 0, 40 ) );
};

//--------------------------------------------------------------
void ofApp::draw(){

  video.draw( 0, 0 );
  gaussian.get().draw( 640, 0 );

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
