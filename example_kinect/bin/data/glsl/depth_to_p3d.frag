#version 120

/*
 * depth to 3d points
 *
 * in: data
 *  float depth texture
 * out: 
 *  3d points in metric space
 *
 * TODO 
 * in: tex1
 *  LUT raw2mm: z_raw_to_mm
 */

#pragma include "kinect.glsl"

uniform sampler2DRect data;

void main( void ) 
{
  vec2 p2 = gl_TexCoord[0].st;
  /*float depth = texture2DRect(data,p2).r;*/
  /*float depth_mm = depth_norm_to_mm(depth);*/
  float depth_mm = texture2DRect(data,p2).r;

  vec3 p3 = unproject( p2, depth_mm );
  gl_FragColor = vec4( p3, 1. );
}

