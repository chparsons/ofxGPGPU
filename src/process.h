#pragma once

#include "ofMain.h"
#include "shader.h"
#include <openssl/md5.h>

namespace gpgpu
{  

  class Process
  {

    public:

      Process()
      {
        shader = NULL;
        file_path = "";
        _debug = NULL;
      };

      ~Process()
      {
        delete _debug;
      };

      Process& init(
          gpgpu::Shader* shader, 
          int _width, int _height ); 

      Process& init(
          string frag_file, 
          int _width, int _height );

      Process& update( int passes = 1 );

      Process& set( string id, ofTexture& data );
      Process& set( string id, vector<float>& data );
      Process& set( string id, float* data );

      ofTexture& get( string id = "" );
      ofTexture get_scaled( float scale, string id = "" );
      ofTexture get_scaled( int width, int height, string id = "" );

      //always add or set backbuffers before calling init
      Process& add_backbuffer( string id );
      Process& add_backbuffers( vector<string> bbuffs );

      Process& update_debug( string id = "debug_input" );
      void draw_debug( float x, float y, float w, float h );
      void set_debug( string frag_file_d );

      //try to avoid this one
      //though useful
      //it is a performance bottleneck
      //TODO impl https://github.com/satoruhiga/ofxFastFboReader
      //TODO cache fpix by id
      float* get_data( string id = "" );
      //ofFloatPixels& get_data( string id = "" );
      //vector<float>& get_data( string id = "" );
      ofVec4f get_data( int x, int y, string id = "" ); 

      /*
       * set watch for all processes
       *  "all"
       *  "none"
       *  "local" (let it watch per process)
       */
      static void watch( string w )
      {  g_watch = w; };

      int size();

      int width()
      {
        return _width;
      };

      int height()
      {
        return _height;
      };

      void log( int x, int y, string id = "" );
      void log( string id = "" );
      void log_config();

      ofEvent<ofShader> on_init;
      ofEvent<ofShader> on_update;

    private:

      vector<string> backbuffers;

      int _width, _height;
      int _size;
      int channels;
      int curfbo;
      string _name;

      ofFbo fbos[2];
      ofFbo::Settings fbo_settings;
      map<string,ofTexture> inputs;
      map<string,ofTexture> inputs_backbuffers;
      //map<string,ofFbo> inputs;
      //void init_input_tex( string id );

      ofShader of_shader;

      ofFloatPixels fpix;
      void read_to_fpix( string id = "");

      gpgpu::Shader* shader; 

      Process& _init( int _width, int _height );

      void set_bbuf_data( string id, vector<float>& data );
      void set_tex_data( ofTexture& tex, vector<float>& data, string id );

      void init_bbuf( string id );
      bool check_bbuf( int i, string id );
      bool check_input_bbuf(ofTexture& input_data);

      int bbuf_idx( string id );
      int pix_idx( int x, int y );
      //int get_tex_idx( string id );

      bool is_input( string id );
      bool is_input_backbuffer( string id );
      bool is_backbuffer( string id );

      void quad( float x, float y, float _width, float _height, float s, float t );

      void log_datum( int i, int x, int y, float r, float g, float b, float a ); 

      //  check file update
      static string g_watch;
      bool _watch;
      void watch( bool w );
      void update_watch();
      string file_path;
      string file_current_hash;
      string file_hash();

      Process* _debug;
      Process& get_debug();
      void _debug_init_from_code();
  };

};

