#pragma once

#include "ofxGPGPU.h"

/*
 * http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter46.html
 */

namespace gpgpu
{

class OddEvenMergeSort : public gpgpu::Shader
{
  private:
    int width, height;
    int stage, pass;

  public: 

  void init( int width, int height )
  {
    this->width = width;
    this->height = height;
    stage = 0;
    pass = 0;
  };

  virtual string name()
  {
    return "gpgpu::OddEvenMergeSort_"+ofToString(ofGetElapsedTimeMicros());
  };

  virtual vector<string> backbuffers()
  {
    vector<string> backbuffer;
    backbuffer.push_back("data");
    return backbuffer;
  };

  void update( ofShader& shader, int _pass )
  { 

    pass--;
    if ( pass < 0 ) 
    {
      // next stage
      stage++;
      pass = stage;
    }

    // perform one step of the current sorting algorithm

    int pstage = (1<<stage);
    int ppass  = (1<<pass);

    shader.setUniform1f("TwoStage", float(pstage+pstage) ); 

    shader.setUniform1f("Pass_mod_Stage", float(ppass%pstage) ); 

    shader.setUniform1f("TwoStage_PmS_1", float((pstage+pstage)-(ppass%pstage)-1) );

    shader.setUniform1f("Width", float(width) );

    shader.setUniform1f("Height", float(height) );

    shader.setUniform1f("Pass", float(ppass) );

    // ???
    //glUniform1iARB(oddevenMergeSort.getUniformLocation("Data"), 0);

    ofLog() << "sort" 
      << " stage " 
      << ofToString(pstage)
      << " pass " 
      << ofToString(ppass);

  };

  string fragment()
  {
    return "#version 120\n #extension GL_ARB_texture_rectangle : enable\n " xstr(

    uniform sampler2DRect data;

    uniform float TwoStage;
    uniform float Pass_mod_Stage;
    uniform float TwoStage_PmS_1;
    uniform float Width;
    uniform float Height;
    uniform float Pass;

    void main()
    {
      vec4 OwnPos = gl_TexCoord[0]; 

      // get self
      vec4 self = texture2DRect(data, OwnPos.xy);
      float i = floor(OwnPos.x) + floor(OwnPos.y) * Width; 

      // my position within the range to merge
      float j = floor(mod(i,TwoStage));

      float compare;

      if ( (j<Pass_mod_Stage) || (j>TwoStage_PmS_1) ) 
        // must copy -> compare with self
        compare = 0.0;
      else
        // must sort
        if ( mod((j+Pass_mod_Stage) / Pass,2.0) < 1.0)
          // we are on the left side -> compare with partner on the right
          compare = 1.0;
        else
          // we are on the right side -> compare with partner on the left
          compare = -1.0;

      // get the partner
      float adr = i + compare*Pass;
      vec4 partner = texture2DRect( data, vec2( floor(mod(adr,Width)), floor(adr / Width) ) );

      // on the left its a < operation, on the right its a >= operation
      gl_FragColor = (self.x*compare < partner.x*compare) ? self : partner;
    }

    ); //fragment code
  }; 

}; //endof class
}; //endof namespace

