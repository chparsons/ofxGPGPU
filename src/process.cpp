
#include "process.h"

void gpgpu::Process::init(
    gpgpu::Shader* shader, 
    int width, int height )
{
  this->shader = shader;
  _name = typeid(shader).name();

  of_shader.setupShaderFromSource( GL_FRAGMENT_SHADER, shader->fragment() );
  of_shader.linkProgram();

  _init( width, height, shader->backbuffers() );
};

void gpgpu::Process::init(
    string frag_file, 
    int width, int height )
{
  vector<string> backbuffers;
  init( frag_file, width, height, backbuffers );
};

void gpgpu::Process::init(
    string frag_file, 
    int width, int height, 
    string backbuffer )
{
  vector<string> backbuffers;
  backbuffers.push_back( backbuffer );
  init( frag_file, width, height, backbuffers );
};

void gpgpu::Process::init(
    string frag_file, 
    int width, int height, 
    vector<string> backbuffers )
{
  _name = frag_file;
  of_shader.load( "", frag_file );
  _init( width, height, backbuffers );
};

void gpgpu::Process::_init(
    int width, int height, 
    vector<string> backbuffers )
{

  this->width = width;
  this->height = height;
  this->backbuffers = backbuffers;

  curfbo = 0;
  channels = 4; //rgba

  _size = width * height * channels;

  //_data = new float[_size];
  //memset(_data, 0, _size);

  fpix.allocate(width, height, channels);

  int nbbufs = backbuffers.size();

  // init ping pong fbos

  ofFbo::Settings s;
  s.internalformat = GL_RGBA32F_ARB;
  s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
  s.minFilter = GL_NEAREST;
  s.maxFilter = GL_NEAREST;
  s.wrapModeHorizontal = GL_CLAMP;
  s.wrapModeVertical = GL_CLAMP;
  s.width = width;
  s.height = height;
  //0.8.0 bug: on ofFbo.cpp this line should be commented out
  ////settings.numColorbuffers = settings.colorFormats.size();
  s.numColorbuffers = nbbufs > 0 ? nbbufs : 1;

  for ( int i = 0; i < 2; i++ )
  {
    fbos[i].allocate(s);
    fbos[i].begin();
    ofClear(0,255);
    fbos[i].end();
  }

  // init data
  for (int i = 0; i < nbbufs; i++)
    init_bbuf( backbuffers[i] );

  ofNotifyEvent( on_init,of_shader,this );

};

void gpgpu::Process::update( int passes )
{

  while ( passes-- > 0 )
  {
    ofFbo* read = &(fbos[curfbo]);
    ofFbo* write = &(fbos[1-curfbo]);

    write->begin();
    ofClear(0,255);

    //fbos[write].begin(false);
    //glPushAttrib(GL_ENABLE_BIT);
    //glViewport(0,0,width,height);
    //glDisable(GL_BLEND);
    //ofSetColor(255);

    write->activateAllDrawBuffers();

    of_shader.begin();

    ofNotifyEvent( on_update, of_shader, this );

    if ( shader != NULL )
      shader->update( of_shader );

    // backbuffers
    int nbbufs = backbuffers.size(); 
    for (int i = 0; i < nbbufs; i++)
    {
      of_shader.setUniformTexture( 
        backbuffers[i], 
        read->getTextureReference(i),
        i );
    }

    // input textures
    int itex = 0;
    for ( map<string,ofTexture>::iterator it = input_texs.begin(); it != input_texs.end(); it++ )
    //for ( map<string,ofFbo>::iterator it = input_texs.begin(); it != input_texs.end(); it++ )
    {
      of_shader.setUniformTexture( 
        it->first, 
        it->second,
        //it->second.getTextureReference(),
        nbbufs + itex );
      itex++;
    }

    //quad( -1, -1, 2, 2, width, height );
    quad(0,0,width,height,width,height);

    of_shader.end();

    //glPopAttrib();

    write->end();

    //ping pong
    curfbo = 1-curfbo;
  }

}; 

void gpgpu::Process::set( string id, ofTexture& data )
{  

  if ( is_backbuffer( id ) ) 
  {
    ofLogWarning() 
      << "[" << _name << "] "
      << "gpgpu::Process::set "
      << "data texture id "
      << "[" << id << "] "
      << "is a backbuffer";
    return;
  }

  else if ( ! is_input_tex( id ) )
  {
    ofLogWarning() 
      << "[" << _name << "] "
      << "gpgpu::Process::set "
      << "data texture id "
      << "[" << id << "] "
      << "not found (initializing...?)";
  }

  // copy input tex
  input_texs[id] = data;

  // input texs as fbos
  //init_input_tex(id);
  //input_texs[id].begin();
  //ofClear(0,255);
  //ofSetColor(255);
  //data.draw(0,0);
  //input_texs[id].end();
};

void gpgpu::Process::set( string id, float* data )
{
  vector<float> data_;
  data_.assign( data, data + _size );
  set( id, data_ );
};

void gpgpu::Process::set( string id, vector<float>& data )
{

  if ( is_backbuffer( id ) ) 
  {
    set_bbuf_data( id, data );
  }

  else if ( is_input_tex( id ) )
  {
    set_data_tex( input_texs[id], data, id ); 
    //set_data_tex( input_texs[id].getTextureReference(), data, id ); 
  }

  else
  {
    ofLogWarning() 
      << "[" << _name << "] "
      << "gpgpu::Process"
      << "set data: "
      << "[" << id << "] "
      << "not found";
  }
};

void gpgpu::Process::set_bbuf_data( string id, vector<float>& data )
{
  int i = get_bbuf_idx( id );
  if ( !check_bbuf(i,id) ) return;
  set_data_tex( fbos[curfbo].getTextureReference( i ), data, id );
};

void gpgpu::Process::set_data_tex( ofTexture& tex, vector<float>& data, string id )
{ 

  if ( data.size() != _size )
  {
    ofLogError() 
      << "[" << _name << "] "
      << "gpgpu::Process "
      << "set data texture "
      << "[" << id << "] "
      << "data size is (" << data.size() << ") "
      << "and (" << _size << ") is expected";
    return;
  }

  if ( !tex.isAllocated() )
    tex.allocate( width, height, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT );

  tex.loadData( &data[0], width, height, GL_RGBA );

  //tex.bind();
  //glTexSubImage2D( 
      //GL_TEXTURE_RECTANGLE_ARB, 0, 
      //0, 0, width, height, 
      //GL_RGBA, GL_FLOAT, 
      //data );
  //tex.unbind();
};

ofTexture& gpgpu::Process::get( string id )
{

  // get process result
  if ( id.empty() )
  {
    return fbos[curfbo]
      .getTextureReference( 0 );
  }

  // or an input texture
  if ( is_input_tex(id) )
  {
    return input_texs[id];
    //return input_texs[id].getTextureReference();
  }

  // or a backbuffer
  int i = get_bbuf_idx( id );

  return fbos[curfbo]
    .getTextureReference( i );
};

float* gpgpu::Process::get_data( string id )
//vector<float>& gpgpu::Process::get_data( string id )
{
  read_to_fpix( id );
  return fpix.getPixels();
  //memcpy( _data, fpix.getPixels(), _size );
  //return _data;
  //float* data_ = fpix.getPixels();
  //_data.assign( data_, data_ + _size );
  //return _data;
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
    fbos[curfbo].readToPixels( fpix, 0 );
    return;
  }

  if ( is_input_tex(id) )
  {
    input_texs[id].readToPixels(fpix);
    //input_texs[id].getTextureReference().readToPixels(fpix);
    return;
  }

  // or a backbuffer
  int i = get_bbuf_idx( id );
  if ( !check_bbuf(i,id) ) return NULL;

  fbos[curfbo]
    .readToPixels( fpix, i );
    //.getTextureReference( i )
    //.readToPixels( fpix );
};

void gpgpu::Process::init_bbuf( string id )
{
  // initial state to 0

  int i = get_bbuf_idx( id );
  if ( !check_bbuf(i,id) ) return;

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
    ofLogWarning() 
      << "[" << _name << "] "
      << "gpgpu::Process::check_bbuf backbuffer id: "
      << id
      << " not found";
    return false;
  }
  return true;
};

// input tex as fbo
//void gpgpu::Process::init_input_tex( string id )
//{
  //ofFbo& fbo = input_texs[id];

  //if ( fbo.isAllocated() )
    //return;

  //ofFbo::Settings s;
  //s.internalformat = GL_RGBA32F_ARB;
  //s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
  //s.minFilter = GL_NEAREST;
  //s.maxFilter = GL_NEAREST;
  //s.wrapModeHorizontal = GL_CLAMP;
  //s.wrapModeVertical = GL_CLAMP;
  //s.width = width;
  //s.height = height;
  //s.numColorbuffers = 1;

  //fbo.allocate(s);
  //fbo.begin();
  //ofClear(0,255);
  //fbo.end();
//};

int gpgpu::Process::get_bbuf_idx( string id )
{
  vector<string>::iterator it = std::find(backbuffers.begin(), backbuffers.end(), id);
  int i = it - backbuffers.begin();
  if ( it == backbuffers.end() || i >= fbos[curfbo].getNumTextures() )
    return -1;
  return i;
};


int gpgpu::Process::get_pix_idx( int x, int y )
{
  return ( x + y * width ) * channels; 
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

  int i = get_pix_idx(x,y);

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

  int w = this->width;
  int h = this->height;

  //float* data = get_data( id );
  read_to_fpix( id );

  for (int y = 0; y < h; y++)
  for (int x = 0; x < w; x++)
  {
    int i = get_pix_idx(x,y);

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
  ofLog() << "\t" 
    << "[" << _name << "] "
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
  for ( map<string,ofTexture>::iterator it = input_texs.begin(); it != input_texs.end(); it++ )
  //for ( map<string,ofFbo>::iterator it = input_texs.begin(); it != input_texs.end(); it++ )
  {
    ofTextureData& texd = it->second.getTextureData();
    //ofTextureData& texd = it->second.getTextureReference().getTextureData();
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
      << "internal format: " << ofGetGlInternalFormatName(texd.glTypeInternal);
  }

};

bool gpgpu::Process::is_input_tex( string id )
{
  return input_texs.find( id ) != input_texs.end();
}

bool gpgpu::Process::is_backbuffer( string id )
{
  return get_bbuf_idx( id ) > -1;
}

void gpgpu::Process::quad( float x, float y, float width, float height, float s, float t )
{

  glBegin(GL_TRIANGLES);

  //t1
  //nw
  glTexCoord2f(0, 0);
  glVertex3f(x, y, 0);
  //ne
  glTexCoord2f(s, 0);
  glVertex3f(x + width, y, 0);
  //sw
  glTexCoord2f(0, t);
  glVertex3f(x, y + height, 0);

  //t2
  //ne
  glTexCoord2f(s, 0);
  glVertex3f(x + width, y, 0);
  //se
  glTexCoord2f(s, t);
  glVertex3f(x + width, y + height, 0);
  //sw
  glTexCoord2f(0, t);
  glVertex3f(x, y + height, 0);

  glEnd();

};

