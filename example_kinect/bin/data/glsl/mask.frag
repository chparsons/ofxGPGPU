#version 120

uniform sampler2DRect data;
uniform sampler2DRect mask;

void main( void ) 
{
  vec4 _depth = texture2DRect( data, gl_TexCoord[0].st );
  vec4 _mask = texture2DRect( mask, gl_TexCoord[0].st );

  _depth = _mask.r < 0.5 ? vec4(0.,0.,0.,1.) : _depth;
  gl_FragColor = _depth * gl_Color;
}
