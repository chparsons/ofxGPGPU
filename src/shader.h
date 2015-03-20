#pragma once

#include "ofxGPGPU.h"
#include "ofShader.h"

#define STRINGIFY(x) #x

namespace gpgpu
{
  class Shader
  {
    public: 

      virtual string fragment() = 0;

      virtual vector<string> backbuffers()
      {
        vector<string> backbuffer;
        backbuffer.push_back("data");
        return backbuffer;
      };

      virtual void update( ofShader& shader ){};
  };
};
 
