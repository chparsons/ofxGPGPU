#pragma once

#include "ofMain.h"
#include "shader.h"

namespace gpgpu
{ 

  class Process
  {

    public: 

      Process(){};
      ~Process(){};

      void init(
          gpgpu::Shader* shader, 
          int width, int height );

      void init(
          string frag_file, 
          int width, int height );

      void init(
          string frag_file, 
          int width, int height, 
          vector<string> backbuffers );

      void init(
          string frag_file, 
          int width, int height, 
          string backbuffer );

      void update( int passes = 1 );

      void set( string id, ofTexture& data );
      void set( string id, vector<float>& data );
      void set( string id, float* data );

      ofTexture& get( string id = "" );

      //try to avoid this one
      //though useful
      //it is a performance bottleneck
      //TODO impl https://github.com/satoruhiga/ofxFastFboReader
      //TODO cache fpix by id
      //float* get_data( string id = "" );
      vector<float> get_data( string id = "" );
      ofVec4f get_data( int x, int y, string id = "" );

      int size();

      void log( int x, int y, string id = "" );
      void log( string id = "" );
      void log_config();

      ofEvent<ofShader> on_init;
      ofEvent<ofShader> on_update;

    private:

      vector<string> backbuffers;

      int width, height;
      int channels;
      int curfbo;
      int _size;
      string _name;

      ofFbo fbos[2];
      map<string,ofTexture> input_texs;
      //map<string,ofFbo> input_texs;
      //void init_input_tex( string id );

      ofShader of_shader; 

      ofFloatPixels fpix;
      void read_to_fpix( string id = "");

      gpgpu::Shader* shader; 

      void _init( 
          int width, int height, 
          vector<string> backbuffers );

      void set_bbuf_data( string id, vector<float>& data );
      void set_data_tex( ofTexture& tex, vector<float>& data, string id );

      void init_bbuf( string id );
      bool check_bbuf( int i, string id );

      int get_bbuf_idx( string id );
      int get_pix_idx( int x, int y );
      //int get_tex_idx( string id );

      bool is_input_tex( string id );
      bool is_backbuffer( string id );

      void quad( float x, float y, float width, float height, float s, float t );

      void log_datum( int i, int x, int y, float r, float g, float b, float a );

  };

};

