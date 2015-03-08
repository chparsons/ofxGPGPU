#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxGPGPU.h"

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

    ofxKinect kinect;

    ofEasyCam easyCam;
    bool bDrawPointCloud;
    void drawPointCloud();

    ofTexture ftex;
    ofFloatPixels fpix;
    float *flut;
    void update_ftex( uint16_t *mm_depth_pix );
    void init_ftex( int w, int h );

    gpgpu::Process test;
    gpgpu::Process p3d;
    gpgpu::Process normals;
    gpgpu::Process normals_vis;
    gpgpu::Process bilateral;
    gpgpu::Process flowfield;

    void p3d_update(ofShader& shader); 

    void log(int x, int y);
    ofVec2f mouse;

};
