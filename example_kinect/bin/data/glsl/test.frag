#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect data;

void main( void ) 
{
  vec2 p2 = gl_TexCoord[0].st;
  float depth = texture2DRect(data, p2).r;
  vec4 depth_out = vec4(depth);
  if (depth < 700) 
    depth_out = vec4(1.,1.,0.,1.);
  if (depth > 900) 
    depth_out = vec4(0.,1.,1.,1.);
  gl_FragColor = vec4( depth_out );
}

