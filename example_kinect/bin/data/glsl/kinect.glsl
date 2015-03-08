
#define k1 0.1236
#define k2 2842.5
#define k3 1.1863
#define k4 0.0370

#pragma include "math.glsl"

// mm to mt
/*uniform float near_mm;*/
/*uniform float far_mm;*/
/*uniform float width;*/
/*uniform float height;*/
/*uniform float depth_xoff;*/
/*uniform vec4 intrinsics; //cx, cy, fx, fy;*/

float near_mm = 500.;
float far_mm = 4000.;
float width = 640.;
float height = 480.;
/*float depth_xoff = -8.;*/

vec3 unproject( vec2 p2, float z_mm ) 
{
  /*return vec3(*/
  /*(p2.x + depth_xoff - cx) * z_mm / fx,*/
  /*(p2.y - cy) * z_mm / fy,*/
  /*z_mm*/
  /*);*/
  float XtoZ = 1.11146;
  float YtoZ = 0.83359;
  return vec3(
    ( p2.x / width - 0.5 ) * z_mm * XtoZ,
    ( p2.y / height - 0.5 ) * z_mm * YtoZ,
    z_mm
    );
}

float depth_mm_to_norm(float depth_mm) 
{
  return lerp2d( depth_mm, near_mm, far_mm, 1., 0. );
}

float depth_norm_to_mm(float depth_norm) 
{
  return lerp2d( depth_norm, 1., 0., near_mm, far_mm );
}

float depth_raw_to_mm(float depth_raw) 
{
  return ( k1 * tan( ( depth_raw / k2 ) + k3 ) - k4 ) * 1000.;
}

