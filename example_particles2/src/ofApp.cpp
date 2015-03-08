#include "ofApp.h"


int w = 1000;
int h = 1000;

void ofApp::setup()
{
  ofSetFrameRate(60);
  ofSetLogLevel(OF_LOG_VERBOSE);

  ofBackground(0);

  pos.init("update_pos.frag", w, h, "pos");
  vel.init("update_vel.frag", w, h, "vel");

  float* _pos = new float[w * h * 4];
  for (unsigned y = 0; y < h; ++y)
  {
    for (unsigned x = 0; x < w; ++x)
    {
      unsigned idx = y * w + x;
      _pos[idx * 4] = 400.f * x / (float)w - 200.f; // particle x
      _pos[idx * 4 + 1] = 400.f * y / (float)h - 200.f; // particle y
      _pos[idx * 4 + 2] = 0.f; // particle z
      _pos[idx * 4 + 3] = 0.f; // dummy
    }
  }

  // set initial state

  pos.set("pos", _pos);
  vel.set("pos", pos.get("pos"));
  pos.set("vel", vel.get("vel"));

  delete[] _pos;

  ofAddListener(pos.on_update, this, &ofApp::pos_update);
  ofAddListener(vel.on_update, this, &ofApp::vel_update);

  // mesh
  mesh.clear();
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      mesh.addVertex(ofVec3f(200.f * x / (float)w - 100.f, 200.f * y / (float)h - 100.f, -500.f));
      mesh.addTexCoord(ofVec2f(x, y));
    }
  }
  mesh.setMode(OF_PRIMITIVE_POINTS);

  drawShader.load("draw"); 
}

void ofApp::update()
{
  ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));

  vel.set("pos", pos.get("pos"));
  vel.update();
  pos.set("vel", vel.get("vel"));
  pos.update();
}

void ofApp::draw()
{
  cam.begin();
  ofEnableBlendMode(OF_BLENDMODE_ADD);

  drawShader.begin();
  drawShader.setUniformTexture("postex", pos.get("pos"), 0);
  mesh.draw();
  drawShader.end();

  ofDisableBlendMode();
  cam.end();
}

void ofApp::vel_update(ofShader& shader)
{
  ofVec3f mouse(
      ofGetMouseX()-.5f*ofGetWidth(), 
      //(float)ofGetMouseX()/ofGetWidth()-.5f, 
      .5f*ofGetHeight()-ofGetMouseY(), 
      //.5f-(float)ofGetMouseY()/ofGetHeight(), 
      0.f);
  shader.setUniform3fv("mouse", mouse.getPtr());
  shader.setUniform1f("elapsed", ofGetLastFrameTime());
  shader.setUniform1f("radiusSquared", 200.f * 200.f);
}

void ofApp::pos_update(ofShader& shader)
{
  shader.setUniform1f("elapsed", ofGetLastFrameTime());
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

