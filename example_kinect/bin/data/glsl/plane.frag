#version 120

uniform sampler2DRect data;

uniform vec3 plane_vecs[3]; //use setUniform3fv() 
uniform float dist_range[2]; //min-max

void main( void ) 
{
  vec4 p4 = texture2DRect( data, gl_TexCoord[0].st );
  vec3 p3 = p4.rgb;
  float depth = p4.a;
  //vec4 plane = make_plane( plane_vecs );
  //float dist = plane_dist( plane, p3 );
  //if ( dist < dist_range[0] || dist > dist_range[1] )
  //p4 = vec4(0.,0.,0.,0.);
  gl_FragColor = p4 * gl_Color;
}
