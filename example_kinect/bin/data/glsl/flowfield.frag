#version 120

#pragma include "math.glsl"

/*
 * flowfield
 *
 * in: data
 *  float depth texture
 * out: 
 *  2d vector field, rgb = xyz
 *
 */

uniform sampler2DRect data;

const int kernel = 6;

void main( void ) 
{
  vec2 loc = gl_TexCoord[0].st;
  /*float depth = texture2DRect(data,loc).r;*/

  float ndepth;
  vec2 nloc = vec2(0.);
  vec2 force = vec2(0.);

  int n = 0;
  int ini = -kernel / 2;
  int end = -ini;

  for ( int i = ini; i <= end; i++ )
    for ( int j = ini; j <= end; j++ )
    {
      if ( i == 0 && j == 0 )
        continue;
      nloc = loc + vec2( i,j );
      ndepth = texture2DRect( data, nloc ).r;
      //*ndepth: drives far away from the plane
      //*-ndepth: drives towards the plane
      force += vec2( i,j ) * ndepth;
      n++;
    }

  force /= n; 
  force = normalize( force );

  vec2 force_vis = vec2(
      lerp2d( force.x, -1., 1., 0., 1. ),
      lerp2d( force.y, -1., 1., 0., 1. )
      );

  gl_FragColor = vec4( force_vis, 0., 1. );
  /*gl_FragColor = vec4( force, 0., 1. );*/
  /*gl_FragColor = vec4( depth, 0., 0., 1. );*/
}
