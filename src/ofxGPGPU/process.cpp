#include "ofxGPGPU/process.h"

string gpgpu::Process::g_watch = "local";

void gpgpu::Process::dispose()
{
  fpix.clear();
  if (_inited)
    for ( int i = 0; i < 2; i++ )
      delete fbos[i];
  for ( map<string,ofTexture>::iterator it = inputs.begin(); it != inputs.end(); it++ )
    it->second.clear();
  inputs.clear();
  backbuffers.clear();
  dispose_render();
  shader = nullptr;
  _inited = false;
};

gpgpu::Process& gpgpu::Process::init(
    gpgpu::Shader* shader, 
    int _width, int _height, 
    int _channels )
{
  this->shader = shader;
  _name = shader->name();

  if ( ! of_shader.setupShaderFromSource( GL_FRAGMENT_SHADER, shader->fragment() ) )
    ofLogError("gpgpu::Process") 
      << "[" << _name << "]: "
      << "shader failed to compile";

  if ( ! of_shader.linkProgram() )
    ofLogError("gpgpu::Process") 
      << "[" << _name << "]: "
      << "shader failed to link";

  add_backbuffers( shader->backbuffers() );
  return _init( _width, _height, _channels );
};

gpgpu::Process& gpgpu::Process::init(
    string frag_file, 
    int _width, int _height, 
    int _channels )
{
  _name = frag_file;
  file_path = frag_file;
  if ( ! of_shader.load( "", frag_file ) )
    ofLogError("gpgpu::Process") 
      << "[" << _name << "]: "
      << "shader failed to load";
  return _init( _width, _height, _channels );
};

gpgpu::Process& gpgpu::Process::_init(
    int _width, int _height, 
    int _channels )
{

  this->_width = _width;
  this->_height = _height;
  this->_channels = _channels;

  curfbo = 0;

  _size = _width * _height * _channels;
  fpix.allocate(_width, _height, _channels);
  int nbbufs = backbuffers.size();

  // init ping pong fbos

  ofFbo::Settings& s = fbo_settings;
  s.internalformat = get_internal_format();
  s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
  s.minFilter = GL_NEAREST;
  s.maxFilter = GL_NEAREST;
  s.wrapModeHorizontal = GL_CLAMP;
  s.wrapModeVertical = GL_CLAMP;
  s.width = _width;
  s.height = _height;
  //0.8.0 bug: on ofFbo.cpp this line should be commented out
  ////settings.numColorbuffers = settings.colorFormats.size();
  s.numColorbuffers = nbbufs > 0 ? nbbufs : 1;

  for ( int i = 0; i < 2; i++ )
  {
    fbos[i] = new ofFbo();
    init_fbo( *(fbos[i]) );
  }

  // init data
  for (int i = 0; i < nbbufs; i++)
    init_bbuf( backbuffers[i] );

  ofNotifyEvent( on_init, of_shader, this );
  _inited = true;

  return *this;
};

gpgpu::Process& gpgpu::Process::update( int passes )
{
  int pass = 0;
  while ( pass < passes )
  {
    ofFbo* read = fbos[curfbo];
    ofFbo* write = fbos[1-curfbo];

    write->begin();
    ofClear(0,255);

    //fbos[write].begin(false);
    //glPushAttrib(GL_ENABLE_BIT);
    //glViewport(0,0,_width,_height);
    //glDisable(GL_BLEND);
    //ofSetColor(255);

    write->activateAllDrawBuffers();

    of_shader.begin();

    // pass data from gpgpu::Shader
    if ( shader != nullptr )
      shader->update( of_shader, pass );

    // let users override gpgpu::Shader data
    ofNotifyEvent( on_update, of_shader, this );

    // pass some default uniforms
    of_shader.setUniform2f( "size", _width, _height );
    of_shader.setUniform1i( "pass", pass );

    // set data textures
    int tex_i = 0;
    int bbuf_i = 0;

    // backbuffers
    int nbbufs = backbuffers.size(); 
    for ( int i = 0; i < nbbufs; i++ )
    {
      of_shader.setUniformTexture( 
        backbuffers[i], 
        read->getTexture( bbuf_i++ ),
        tex_i++ );
    }

    // input textures
    for ( map<string,ofTexture>::iterator it = inputs.begin(); it != inputs.end(); it++ )
    //for ( map<string,ofFbo>::iterator it = inputs.begin(); it != inputs.end(); it++ )
    {
      of_shader.setUniformTexture( 
        it->first, 
        it->second,
        //it->second.getTexture(),
        tex_i++ );
    }

    quad(0, 0, _width, _height);

    of_shader.end();

    //glPopAttrib();

    write->end();

    //ping pong
    curfbo = 1-curfbo;
    pass++;
  }

  update_watch(); 
  return *this;
};

gpgpu::Process& gpgpu::Process::set( string id, ofTexture& data )
{  
  int bbi = bbuf_idx( id );
  if ( bbi > -1 ) 
  {
    set_fbo( data, *(fbos[curfbo]) );
    fbos[curfbo]->getTexture( bbi ); //triggers updateTexture(attachmentPoint)
  }

  else
  {
    inputs[id] = data; // copy input tex
    //init_fbo( inputs[id] );
    //set_fbo( data, inputs[id] );
  }

  return *this;
};

gpgpu::Process& gpgpu::Process::set( string id, float* data )
{
  vector<float> data_;
  data_.assign( data, data + _size );
  return set( id, data_ );
};

gpgpu::Process& gpgpu::Process::set( string id, vector<float>& data )
{

  if ( is_backbuffer( id ) ) 
  {
    set_bbuf_data( id, data );
  }

  else
  {
    set_tex_data( inputs[id], data, id ); 
    //set_tex_data( inputs[id].getTexture(), data, id );
  } 

  return *this;
};

void gpgpu::Process::set_bbuf_data( string id, vector<float>& data )
{
  int i = bbuf_idx( id );
  if ( !check_bbuf(i,id) ) 
    return;
  set_tex_data( fbos[curfbo]->getTexture( i ), data, id );
};

void gpgpu::Process::set_tex_data( ofTexture& tex, vector<float>& data, string id )
{ 

  if ( data.size() != _size )
  {
    ofLogError("gpgpu::Process") 
      << "[" << _name << "]: "
      << "set data texture "
      << "[" << id << "] "
      << "data size is (" << data.size() << ") "
      << "and (" << _size << ") is expected";
    return;
  }

  if ( !tex.isAllocated() )
    tex.allocate( _width, _height, get_internal_format(), get_format(), GL_FLOAT );

  tex.loadData( &data[0], _width, _height, get_format() );

  //tex.bind();
  //glTexSubImage2D( 
      //GL_TEXTURE_RECTANGLE_ARB, 0, 
      //0, 0, _width, _height, 
      //get_format(), GL_FLOAT, 
      //data );
  //tex.unbind();
};

ofTexture& gpgpu::Process::get( string id )
{

  // get process result
  if ( id.empty() )
  {
    return fbos[curfbo]->getTexture( 0 );
  }

  // an input texture
  //else if ( is_input(id) )
  //{
    //return inputs[id];
  //}

  // or a backbuffer
  else 
  {
    int i = bbuf_idx( id );
    if ( check_bbuf(i,id) )
    {
      return fbos[curfbo]->getTexture( i );
    }
    else
    {
      static ofTexture err;
      return err;
    }
  }
};

void gpgpu::Process::init_fbo( ofFbo& fbo )
{
  return init_fbo( fbo, fbo_settings );
};

void gpgpu::Process::init_fbo( ofFbo& fbo, ofFbo::Settings& s )
{
  if ( fbo.isAllocated() )
    return;
  fbo.allocate( s );
  fbo.begin();
  ofClear(0,255);
  fbo.end();
};

void gpgpu::Process::set_fbo( ofTexture& src, ofFbo& dst )
{
  set_fbo( src, dst, dst.getWidth(), dst.getHeight() );
};

void gpgpu::Process::set_fbo( ofTexture& src, ofFbo& dst, int w, int h )
{
  dst.begin();
  ofClear(0,255);
  src.draw( 0, 0, w, h );
  dst.end();
};

float* gpgpu::Process::get_data( string id )
{
  read_to_fpix( id );
  return fpix.getData();
};

ofFloatPixels& gpgpu::Process::get_data_pix( string id )
{
  read_to_fpix( id );
  return fpix;
};

ofVec4f gpgpu::Process::get_data( int x, int y, string id )
{
  read_to_fpix( id );
  ofFloatColor d = fpix.getColor(x,y);
  return ofVec4f(d.r, d.g, d.b, d.a);
};

void gpgpu::Process::read_to_fpix( string id)
{

  if ( id.empty() )
  {
    fbos[curfbo]->readToPixels( fpix, 0 );
    return;
  }

  // an input texture
  //else if ( is_input(id) )
  //{
    //inputs[id].readToPixels(fpix);
    //return;
  //}

  // or a backbuffer
  else 
  {
    int i = bbuf_idx( id );
    if ( check_bbuf(i,id) ) 
    {
      fbos[curfbo]
        ->readToPixels( fpix, i );
        //.getTexture( i )
        //.readToPixels( fpix );
    }
  }
};

ofTexture gpgpu::Process::get_scaled( int w, int h, string id )
{
  return get_scaled( (float)w/_width, id );
};

ofTexture gpgpu::Process::get_scaled( float scale, string id )
{
  return get_scaled_tex( get(id), scale );
};

ofTexture gpgpu::Process::get_scaled_tex( ofTexture& src, int w, int h )
{
  return get_scaled_tex( src, (float)w/_width );
};

ofTexture gpgpu::Process::get_scaled_tex( ofTexture& src, float scale )
{
  if ( scale == 1.0 )
    return src; //a copy

  ofFbo::Settings s = fbo_settings; //copy
  s.width = src.getWidth() * scale;
  s.height = src.getHeight() * scale;

  ofFbo dst;
  init_fbo( dst, s );
  set_fbo( src, dst, s.width, s.height );

  return dst.getTexture(); //a copy
};

gpgpu::Process& gpgpu::Process::add_backbuffer( string id )
{
  if ( _inited )
    ofLogError("gpgpu::Process") 
      << "[" << _name << "]:"
      << " add_backbuffer [" << id << "]"
      << " called after initialization";

  if ( is_backbuffer( id ) )
  {
    ofLogWarning("gpgpu::Process") 
      << "[" << _name << "]:"
      << " add backbuffer [" << id << "]" 
      << " is already a backbuffer";
  }

  else
  {
    backbuffers.push_back( id );
  }

  return *this;
};

gpgpu::Process& gpgpu::Process::add_backbuffers( vector<string> bbuffs )
{
  for ( int i = 0; i < bbuffs.size(); i++ )
    add_backbuffer( bbuffs[i] );
  return *this; 
};

void gpgpu::Process::init_bbuf( string id )
{
  // initial state to 0

  int i = bbuf_idx( id );
  if ( !check_bbuf(i,id) ) 
    return;

  vector<float> zeros( _size, 0.0f );
  set_bbuf_data( backbuffers[i], zeros );

  //float* zeros = new float[ _size ];
  //memset( zeros, 0, sizeof(float) * _size );
  //set_bbuf_data( backbuffers[i], zeros );
  //delete[] zeros;
};

bool gpgpu::Process::check_bbuf( int i, string id )
{
  if ( i == -1 ) 
  {
    ofLogWarning("gpgpu::Process") 
      << "[" << _name << "] "
      << "check_bbuf backbuffer id: "
      << id << " not found";
    return false;
  }
  return true;
};

int gpgpu::Process::bbuf_idx( string id )
{
  vector<string>::iterator it = std::find( backbuffers.begin(), backbuffers.end(), id );
  int i = it - backbuffers.begin();
  if ( it == backbuffers.end() || i >= fbos[curfbo]->getNumTextures() )
    return -1;
  return i;
};

int gpgpu::Process::pix_idx( int x, int y )
{
  return ( x + y * _width ) * _channels; 
};

int gpgpu::Process::size()
{
  return _size;
};

void gpgpu::Process::log( int x, int y, string id )
{
  ofLog() 
    << "[" << _name << "] "
    << "log data"
    << ", id=" << id
    << ", xy=(" << x << "," << y << ")"
    << ", curfbo=" << curfbo;

  int i = pix_idx(x,y);

  //float* data = get_data( id );
  //float r = data[i];
  //float g = data[i+1];
  //float b = data[i+2];
  //float a = data[i+3];
  //log_datum(i,x,y,r,g,b,a);

  read_to_fpix( id );
  ofFloatColor d = fpix.getColor(x,y);
  log_datum(i,x,y,d.r,d.g,d.b,d.a);
}

void gpgpu::Process::log( string id )
{
  ofLog() 
    << "[" << _name << "] "
    << "log data"
    << ", id=" << id
    << ", curfbo=" << curfbo;

  int w = this->_width;
  int h = this->_height;

  //float* data = get_data( id );
  read_to_fpix( id );

  for (int y = 0; y < h; y++)
  for (int x = 0; x < w; x++)
  {
    int i = pix_idx(x,y);

    //float r = data[i];
    //float g = data[i+1];
    //float b = data[i+2];
    //float a = data[i+3];
    //log_datum(i,x,y,r,g,b,a);

    ofFloatColor d = fpix.getColor(x,y);
    log_datum(i,x,y,d.r,d.g,d.b,d.a);
  }
};

void gpgpu::Process::log_datum( int i, int x, int y, float r, float g, float b, float a )
{
  ofLog() 
    << "[" << _name << "] "
    << "\t" 
    << "idx: "
    << ofToString(i) 
    << "\t xy: "
    << ofToString(x) 
    << ","
    << ofToString(y) 
    << "\t data: "
    << ofToString(r)
    << ","
    << ofToString(g)
    << ","
    << ofToString(b)
    << ","
    << ofToString(a)
    << "";
};

//  check file update

string gpgpu::Process::file_hash()
{
  string ret = "";
  string aboluste_path = ofFilePath::getAbsolutePath(file_path,true);

  unsigned char c[MD5_DIGEST_LENGTH];
  int i;
  FILE *inFile = fopen (aboluste_path.c_str(), "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];

  if ( inFile != nullptr )
  {
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
      MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    for( i = 0; i < MD5_DIGEST_LENGTH; i++ )
    {
      ret += ofToHex(c[i]);
    }
    fclose (inFile);
  }

  inFile = nullptr;
  return ret;
}

//

void gpgpu::Process::log_config()
{
  ofLog() 
    << "[" << _name << "] "
    << "log process config";

  int nbbufs = backbuffers.size(); 
  ofLog() << "\t backbuffers size: " << nbbufs;

  for (int i = 0; i < nbbufs; i++)
    ofLog() << "\t\t" << backbuffers[i];

  ofLog() << "\t input textures:";
  for ( map<string,ofTexture>::iterator it = inputs.begin(); it != inputs.end(); it++ )
  //for ( map<string,ofFbo>::iterator it = inputs.begin(); it != inputs.end(); it++ )
  {
    ofTextureData& texd = it->second.getTextureData();
    //ofTextureData& texd = it->second.getTexture().getTextureData();
    ofLog() << "\t\t" 
      << "id: "
      << it->first
      << ", "
      << "width: " << texd.tex_w
      << ", "
      << "height: " << texd.tex_h
      << ", "
      << "allocated: " << texd.bAllocated
      << ", "
      << "internal format: " << ofGetGlInternalFormatName(texd.glInternalFormat);
  }

};

bool gpgpu::Process::is_input( string id )
{
  return inputs.find( id ) != inputs.end();
};

bool gpgpu::Process::is_backbuffer( string id )
{
  return bbuf_idx( id ) > -1;
};

void gpgpu::Process::watch( bool w )
{ 
  _watch = w; 
  file_current_hash = file_hash();
};

void gpgpu::Process::update_watch()
{
  if ( g_watch == "none" 
    || (g_watch == "local" && !_watch) 
    || file_path.empty() )
    return;

  //watch!
  if ( ofGetSeconds() % 4 == 0.0 )
  {
    string hash = file_hash();
    if ( file_current_hash != hash )
    {
      file_current_hash = hash;
      of_shader.setupShaderFromFile(GL_FRAGMENT_SHADER, file_path);
      of_shader.linkProgram();
    }
  }

  if ( !of_shader.isLoaded() )
  {
    ofLogNotice() << " ofxGPGPU : Cant compile shader.\r\n";
  }
}

void gpgpu::Process::quad( float x, float y, float _width, float _height)
{

  //TODO set st [0,1] to avoid doing this in frags:
  //vec2 st = gl_TexCoord[0].xy / process_size * vec2(textureSize2DRect(data,0))
  float s = _width;
  float t = _height;

  glBegin(GL_TRIANGLES);

  //t1
  //nw
  glTexCoord2f(0, 0);
  glVertex3f(x, y, 0);
  //ne
  glTexCoord2f(s, 0);
  glVertex3f(x + _width, y, 0);
  //sw
  glTexCoord2f(0, t);
  glVertex3f(x, y + _height, 0);

  //t2
  //ne
  glTexCoord2f(s, 0);
  glVertex3f(x + _width, y, 0);
  //se
  glTexCoord2f(s, t);
  glVertex3f(x + _width, y + _height, 0);
  //sw
  glTexCoord2f(0, t);
  glVertex3f(x, y + _height, 0);

  glEnd();

};

int gpgpu::Process::get_format()
{
  switch(_channels) 
  {
    case 1:
      return GL_LUMINANCE;
    case 3:
      return GL_RGB;
    default:
      return GL_RGBA;
  };
};

int gpgpu::Process::get_internal_format()
{
  switch(_channels) 
  {
    case 1:
      return GL_LUMINANCE32F_ARB;
    case 3:
      return GL_RGB32F_ARB;
    default:
      return GL_RGBA32F_ARB;
  };
};

gpgpu::Process& gpgpu::Process::update_render( string id )
{
  Process& render = get_render();
  render.set( id, get() );
  return render.update();
};

void gpgpu::Process::update_render( bool run, string id )
{
  if (run) update_render( id );
  else dispose_render();
};

void gpgpu::Process::dispose_render()
{
  delete _render;
  _render = nullptr;
};

void gpgpu::Process::render( float x, float y, float w, float h )
{
  Process& render = get_render();
  ofTexture& tex = render.get();
  tex.draw(x, y, w, h);
};

gpgpu::Process& gpgpu::Process::get_render()
{
  _render_init_from_code();
  return *_render;
};

void gpgpu::Process::set_render( string frag_file_d )
{
  if ( _render != nullptr )
    return;
  _render = new Process();
  _render->init( frag_file_d, _width, _height, _channels );
};

void gpgpu::Process::_render_init_from_code()
{
  if ( _render != nullptr )
    return;
  _render = new Process();

  string filename = file_path;
  GLenum type = GL_FRAGMENT_SHADER;

  if ( filename.empty() )
  {
    ofLogError("gpgpu::Process") 
      << "[" << _name << "]: "
      << "_render_init_from_code(): "
      << "couldn't init code from empty fragment filename";
    return;
  }

  //see ofShader::setupShaderFromFile(GLenum type, string filename)
  ofBuffer buffer = ofBufferFromFile(filename);
  // we need to make absolutely sure to have an absolute path here, so that any #includes
  // within the shader files have a root directory to traverse from.
  string absoluteFilePath = ofFilePath::getAbsolutePath(filename, true);
  string sourceDirectoryPath = ofFilePath::getEnclosingDirectory(absoluteFilePath,false);
  if ( !buffer.size() ) 
  {
    ofLogError("gpgpu::Process") 
      << "_render_init_from_code(): "
      << "couldn't load shader from \""<<filename<<"\"";
    return;
  } 

  _render->_name = _name+"_render";

  string frag_code = buffer.getText();
  bool render_enabled = ofIsStringInString(frag_code, "void __render__");
  if ( !render_enabled)
  {
    ofLogError("gpgpu::Process") 
      << "[" << _render->_name << "]: "
      << "_render_init_from_code(): "
      << "fragment code does not contain a \"void __render__()\" function to parse";
    return;
  }

  ofStringReplace(frag_code, "void main", "void __main__");
  ofStringReplace(frag_code, "void __render__", "void main");
  //ofLogNotice("gpgpu::Process") << "init render process" 
    //<< " frag_code: \n\n" 
    //<< " xxxxxxxxxxxxxxxxxxxxxxxxx\n\n"
    //<< frag_code
    //<< " xxxxxxxxxxxxxxxxxxxxxxxxx\n\n";

  if ( ! _render->of_shader.setupShaderFromSource( GL_FRAGMENT_SHADER, frag_code, sourceDirectoryPath ) )
    ofLogError("gpgpu::Process") 
      << "[" << _render->_name << "]: "
      << "_render_init_from_code(): "
      << "shader failed to compile";

  if ( ! _render->of_shader.linkProgram() )
    ofLogError("gpgpu::Process") 
      << "[" << _render->_name << "]: "
      << "_render_init_from_code(): "
      << "shader failed to link";

  //ofLogNotice("gpgpu::Process") << "init render process ["
    //<< _name << "] frag_code: \n\n" 
    //<< " xxxxxxxxxxxxxxxxxxxxxxxxx\n\n"
    //<< _render->of_shader.getShaderSource( GL_FRAGMENT_SHADER )
    //<< " xxxxxxxxxxxxxxxxxxxxxxxxx\n\n";

  _render->_init( _width, _height, _channels );
};

