#include "ofApp.h"

// mm to mt
float near_mm = 500.;
float far_mm = 4000.;
float depth_xoff = -8.;
//struct intrinsics; //cx, cy, fx, fy;

void ofApp::setup(){

  //ofSetLogLevel(OF_LOG_VERBOSE);
  ofSetFrameRate(60); 

  mouse.x = 300;
  mouse.y = 300;

  kinect.setRegistration(false);
  //kinect.setDepthClipping( near_mm, far_mm ); //mm (50cm - 4mt)
  //kinect.enableDepthNearValueWhite(false);
  //ir, rgb, texture
  kinect.init(false, false, true);
  kinect.open();

  bDrawPointCloud = false;

  int w = kinect.width;
  int h = kinect.height;

  init_ftex( w, h );

  test.init( "glsl/test.frag", w, h );

  p3d.init("glsl/depth_to_p3d.frag",w,h);
  ofAddListener( p3d.on_update, this, &ofApp::p3d_update );

  normals.init("glsl/normals.frag",w,h);
  normals_vis.init("glsl/normals_vis.frag",w,h);

  bilateral.init( "glsl/bilateral.frag", w, h );

  flowfield.init("glsl/flowfield.frag",w,h);

}

void ofApp::p3d_update(ofShader& shader)
{
  //TODO
  //shader.setUniform1f( "width", kinect.width );
  //shader.setUniform1f( "height", kinect.height );
  //shader.setUniform1f( "near_mm", near_mm * 0.001 ); //mm to mt
  //shader.setUniform1f( "far_mm", far_mm * 0.001 ); //mm to mt
  //shader.setUniform1f( "depth_xoff", -8. );
  //shader.setUniform4f( "intrinsics", intrinsics.cx, intrinsics.cy, intrinsics.fx, intrinsics.fx );
}

void ofApp::update()
{

  ofSetWindowTitle(ofToString(ofGetFrameRate(),2));

  kinect.update();

  if ( !kinect.isFrameNew() )
    return;

  uint16_t *mm_depth_pix = kinect.getRawDepthPixels();
  update_ftex( mm_depth_pix ); 

  test.set( "data", ftex ); 
  test.update();

  flowfield.set( "data", ftex ); 
  flowfield.update(); 

  //p3d.set( "data", kinect.getDepthTextureReference() ); 
  p3d.set( "data", ftex );
  //p3d.set( "data", bilateral.get() ); 
  p3d.update();

  normals.set( "data", p3d.get() );
  normals.update();  

  normals_vis.set( "data", normals.get() );
  normals_vis.update();

  //bilateral.set( "data", ftex ); 
  bilateral.set( "data", normals_vis.get() ); 
  bilateral.update();

  if (mouse.x > -1 && mouse.y > -1)
  {
    log( mouse.x, mouse.y );
    mouse = ofVec2f(-1,-1);
  }
}

void ofApp::draw(){

  ofBackground(100);
  ofSetColor(255,255,255);

  if (bDrawPointCloud) 
  {
    easyCam.begin();
    drawPointCloud();
    easyCam.end();
  } 
  else 
  {
    int w = kinect.width; 
    int h = kinect.height;

    kinect.drawDepth(0, 0, w, h);
    cur_proc()->get().draw(w, 0, w, h);
  }

  ofDrawBitmapStringHighlight( "current process: "+cur_proc_str(), 20, ofGetHeight()-20, ofColor::yellow, ofColor::magenta );
}

gpgpu::Process* ofApp::cur_proc()
{
  switch( render_process )
  {
    case _test:
      return &test;

    case _p3d: 
      return &p3d;

    case _normals: 
      return &normals;

    case _normals_vis: 
      return &normals_vis;

    case _bilateral: 
      return &bilateral;

    case _flowfield: 
      return &flowfield;
  }

  return NULL;
}

string ofApp::cur_proc_str()
{
  switch( render_process )
  {
    case _test:
      return "test";

    case _p3d: 
      return "p3d";

    case _normals: 
      return "normals";

    case _normals_vis: 
      return "normals_vis";

    case _bilateral: 
      return "bilateral";

    case _flowfield: 
      return "flowfield";

  }
  return "unnamed process";
}



//ofxCamaraLucida::DepthCamera

void ofApp::update_ftex( uint16_t *mm_depth_pix )
{

  int w = kinect.width;
  int h = kinect.height;

  int len = w * h;

  for (int i = 0; i < len; i++)
  {
    uint16_t mm = mm_depth_pix[ i ];
    fpix[ i ] = mm; 
  }

  ftex.loadData( fpix );
};

void ofApp::init_ftex(int w, int h)
{
  if ( ftex.isAllocated() )
  {
    ofLogWarning() << "float texture already allocated ???";
    return;
  }

  ftex.allocate( w, h, GL_LUMINANCE32F_ARB );
  fpix.allocate( w, h, 1 );
  fpix.set( 0 );
};

void ofApp::log(int x, int y)
{
  ofFloatPixels _fpix;
  ofFloatColor _fcol;

  ofLog() << "==== log ===="; 

  ofLog() << "xy: " << x << "," << y;

  ftex.readToPixels(_fpix);
  _fcol = _fpix.getColor(x,y);
  ofLog() << "ftex.readToPixels: " << _fcol.r << ", " << _fcol.g << ", " << _fcol.b << ", " << _fcol.a; 

  cur_proc()->get("data").readToPixels(_fpix);
  _fcol = _fpix.getColor(x,y);
  ofLog() << cur_proc_str() << " get(\"data\").readToPixels: " << _fcol.r << ", " << _fcol.g << ", " << _fcol.b << ", " << _fcol.a;

  cur_proc()->get().readToPixels(_fpix);
  _fcol = _fpix.getColor(x,y);
  ofLog() << cur_proc_str() << " get().readToPixels: " << _fcol.r << ", " << _fcol.g << ", " << _fcol.b << ", " << _fcol.a; 

  ofLog() << cur_proc_str() << " log(x,y): ";
  cur_proc()->log(x,y);

  ofLog() << "============="; 
}

void ofApp::next_render_process()
{
  int len = _render_process_length;
  ++render_process;
  render_process = render_process == len ? 0 : render_process;
}

void ofApp::prev_render_process()
{
  int len = _render_process_length;
  --render_process;
  render_process = render_process == -1 ? len-1 : render_process;
}

void ofApp::keyPressed(int key){

  switch (key) {

    case 'p':
      bDrawPointCloud = !bDrawPointCloud;
      break;

    case 'w':
      //kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
      break;

    case 'd':
      cur_proc()->log_config();
      ofLog() << cur_proc_str() << " data:";
      cur_proc()->log("data");
      break;

    case 'r':
      cur_proc()->log_config();
      ofLog() << cur_proc_str() << " result:";
      cur_proc()->log();
      break;


    case 'z':
      prev_render_process();
      break;

    case 'x':
      next_render_process();
      break;

  }

}


void ofApp::drawPointCloud() 
{
  int w = 640;
  int h = 480;
  ofMesh mesh;
  mesh.setMode(OF_PRIMITIVE_POINTS);
  int step = 2;
  for (int y = 0; y < h; y += step) 
  {
    for (int x = 0; x < w; x += step) 
    {
      if (kinect.getDistanceAt(x, y) > 0) 
      {
        //mesh.addColor(kinect.getColorAt(x,y));
        mesh.addColor( ofFloatColor(1.f,1.f,1.f) );
        mesh.addVertex( kinect.getWorldCoordinateAt(x, y) );
      }
    }
  }
  glPointSize(3);
  ofPushMatrix();
  // the projected points are 'upside down' and 'backwards' 
  ofScale(1, -1, -1);
  ofTranslate(0, 0, -1000); // center the points a bit
  glEnable(GL_DEPTH_TEST);
  mesh.drawVertices();
  glDisable(GL_DEPTH_TEST);
  ofPopMatrix();
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
void ofApp::mousePressed(int x, int y, int button)
{
  mouse.x = x;
  mouse.y = y;
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
