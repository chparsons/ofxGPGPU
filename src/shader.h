#pragma once

#include "ofxGPGPU.h"
#include "ofShader.h"

#define STRINGIFY(x) #x

namespace gpgpu
{

  class Shader
  {
    public: 

      virtual void init( int width, int height ) = 0;
      virtual void update( ofShader& shader ) = 0;
      virtual string frag() = 0;
      virtual vector<string> backbuffers() = 0;

    protected:
      int width, height;
  };

};
 
