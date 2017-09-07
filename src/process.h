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
        _inited = false;
      };

      virtual ~Process()
      {
        dispose();
      };

      Process& init(
          gpgpu::Shader* shader, 
          int _width, int _height ); 

      Process& init(
          string frag_file, 
          int _width, int _height );

      Process& update( int passes = 1 );

      void dispose();

      Process& set( string id, ofTexture& data );
      Process& set( string id, vector<float>& data );
      Process& set( string id, float* data );

      ofTexture& get( string id = "" );
      ofTexture get_scaled( float scale, string id = "" );
      ofTexture get_scaled( int width, int height, string id = "" );

      Process& add_backbuffer( string id );
      Process& add_backbuffers( vector<string> bbuffs );

      Process& get_debug();
      Process& update_debug( string id = "debug_input" );
      Process& update_debug( bool run, string id = "debug_input" );
      void dispose_debug();
      void render_debug( float x, float y, float w, float h );
      void set_debug( string frag_file_d );

      //try to avoid this one: perf bottleneck
      //TODO impl https://github.com/satoruhiga/ofxFastFboReader
      //TODO cache fpix by id
      float* get_data( string id = "" );
      ofFloatPixels& get_data_pix( string id = "" );
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

      bool inited()
      {
        return _inited;
      };

      void log( int x, int y, string id = "" );
      void log( string id = "" );
      void log_config();

      template <class ListenerClass>
      Process& on( string type, ListenerClass* listener, void (ListenerClass::*method)(ofShader&) )
      {
        ofAddListener( event(type), listener, method );
        return *this;
      }; 

      template <class ListenerClass>
      Process& off( string type, ListenerClass* listener, void (ListenerClass::*method)(ofShader&) )
      {
        ofRemoveListener( event(type), listener, method );
        return *this;
      }; 

    private:

      vector<string> backbuffers;

      int _width, _height;
      int _size;
      bool _inited;
      int channels;
      int curfbo;
      string _name;

      ofFbo::Settings fbo_settings;
      ofFbo* fbos[2];
      void init_fbo( ofFbo& fbo );
      void init_fbo( ofFbo& fbo, ofFbo::Settings& s );
      void set_fbo( ofTexture& src, ofFbo& dst );
      void set_fbo( ofTexture& src, ofFbo& dst, int w, int h );
      map<string,ofTexture> inputs;
      //map<string,ofFbo> inputs;

      ofShader of_shader;

      ofFloatPixels fpix;
      void read_to_fpix( string id = "");

      gpgpu::Shader* shader; 

      Process& _init( int _width, int _height );

      void set_bbuf_data( string id, vector<float>& data );
      void set_tex_data( ofTexture& tex, vector<float>& data, string id );

      void init_bbuf( string id );
      bool check_bbuf( int i, string id );
      int bbuf_idx( string id );

      int pix_idx( int x, int y );
      //int get_tex_idx( string id );

      bool is_input( string id );
      bool is_backbuffer( string id );

      void quad( float x, float y, float _width, float _height, float s, float t );

      ofTexture get_scaled_tex( ofTexture& src, float scale );
      ofTexture get_scaled_tex( ofTexture& src, int width, int height );

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
      void _debug_init_from_code();

      ofEvent<ofShader> on_init;
      ofEvent<ofShader> on_update;
      ofEvent<ofShader>& event( string type )
      {
        if ( type == "update" ) 
          return on_update;
        else if ( type == "init" ) 
          return on_init;
      };
  };

};

