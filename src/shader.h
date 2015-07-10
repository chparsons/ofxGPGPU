#pragma once

#include "ofxGPGPU.h"

namespace gpgpu
{
  class Shader
  {
    public: 

      virtual string fragment() = 0;
      virtual string name() = 0;

      virtual void update( ofShader& shader, int pass ){};

      virtual vector<string> backbuffers()
      {
        vector<string> backbuffer;
        return backbuffer;
      };

  };
};
 
