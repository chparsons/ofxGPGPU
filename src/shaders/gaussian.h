#pragma once

#include "ofxGPGPU.h"

/*
 * gaussian filter
 * code from GPU Gems 3
 * Chapter 40. Incremental Computation of the Gaussian
 */

namespace gpgpu
{

class Gaussian : public gpgpu::Shader
{
  private:
    float pass;

  public: 

    Gaussian()
    { 
      pass = 1.;
    };

    virtual string name()
    {
      return "gpgpu::Gaussian_"+ofToString(ofGetElapsedTimeMicros());
    };

    virtual vector<string> backbuffers()
    {
      vector<string> backbuffer;
      backbuffer.push_back("data");
      return backbuffer;
    };

    void update( ofShader& shader )
    {
      if (++pass > 1.) pass = 0.;
      shader.setUniform1i( "pass", pass ); 
    }; 

    string fragment()
    {
      return "#version 120 \n #extension GL_ARB_texture_rectangle : enable \n #extension GL_EXT_gpu_shader4 : enable \n" xstr(

      uniform vec2 size;

      uniform sampler2DRect data; // source image

      uniform int pass;
      uniform float sigma;
      uniform int kernel;

      vec2 dir; // horiz=(1.0, 0.0), vert=(0.0, 1.0)

      const float PI = 3.14159265f;

      void main()
      {
        dir = vec2( pass, 1.-pass );

        float norm = 1.0 / ( sqrt ( 2.0 * PI ) * sigma );

        vec2 data_size = vec2(textureSize2DRect(data,0));
        vec2 loc = gl_FragCoord.xy / size * data_size;
        //vec2 loc = gl_FragCoord.xy; // center pixel cooordinate

        vec4 acc; // accumulator
        acc = texture2DRect( data, loc ); // accumulate center pixel

        for ( int i = 1; i <= kernel; i++ ) 
        {
          float coeff = exp(-0.5 * float(i) * float(i) / (sigma * sigma));
          acc += (texture2DRect( data, loc - float(i) * dir) ) * coeff; //L
          acc += (texture2DRect( data, loc + float(i) * dir) ) * coeff; //R
        }

        acc *= norm; // normalize for unity gain
        gl_FragColor = acc;
      }

      ); //fragment code
    };


    /*
     * http://callumhay.blogspot.com.ar/2010/09/gaussian-blur-shader-glsl.html
     */

    //string fragment()
    //{
      //return "#version 120\n #extension GL_ARB_texture_rectangle : enable\n #extension GL_EXT_gpu_shader4 : enable\n" xstr(

      //uniform int pass;

      //uniform float sigma; 
      ////gaussian sigma: higher value means more blur

      //uniform int kernel;
      //// a good value for 9x9 is around 3 to 5
      //// a good value for 7x7 is around 2.5 to 4
      //// a good value for 5x5 is around 2 to 3.5

      //vec2 dir; // horiz=(1.0, 0.0), vert=(0.0, 1.0)

      //float size;
      //// This should usually be equal to
      //// 1.0f / texture_pixel_width for a horizontal blur, and
      //// 1.0f / texture_pixel_height for a vertical blur.

      //uniform sampler2DRect data;  
      //// Texture that will be blurred by this shader

      //const float PI = 3.14159265f;

      //void main() 
      //{
        //dir = vec2( 1., 0. );

        //vec2 loc = gl_TexCoord[0].xy; // center pixel cooordinate
        //size = 1.0f / textureSize2DRect(data,0).x;

        //// Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
        //vec3 gauss;
        //gauss.x = 1.0f / (sqrt(2.0f * PI) * sigma);
        //gauss.y = exp(-0.5f / (sigma * sigma));
        //gauss.z = gauss.y * gauss.y;

        //vec4 avg = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        //float coeffsum = 0.0f;

        //// Take the central sample first...
        //avg += texture2DRect( data, loc ) * gauss.x;
        //coeffsum += gauss.x;
        //gauss.xy *= gauss.yz;

        //// Go through the remaining 8 vertical samples (4 on each side of the center)
        //for (float i = 1.0f; i <= kernel; i++) 
        //{ 
          //avg += texture2DRect( data, loc - i * size * dir ) * gauss.x;
          //avg += texture2DRect( data, loc + i * size * dir ) * gauss.x;

          //coeffsum += 2 * gauss.x;
          //gauss.xy *= gauss.yz;
        //}

        //gl_FragColor = avg / coeffsum;
      //}

    //); //fragment code
  //};

}; //endof class
}; //endof namespace

