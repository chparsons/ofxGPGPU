#version 120

#pragma include "math.glsl"

/*
 * normals visualization
 * in: data 
 *  normalized normals
 *  proccessed by normals.frag
 * out: 
 *  normals within a visible range
 */

uniform sampler2DRect data;

void main( void ) 
{
    vec2 p2 = gl_TexCoord[0].st;
    vec4 normal = texture2DRect(data, p2);

    //in: normalized normals vectors
    //[-1,1] for each dimension
    vec3 normal_vis = vec3(
      lerp2d(normal.x, -1.,1., 1.,0.),
      lerp2d(normal.y, -1.,1., 1.,0.),
      lerp2d(normal.z, -1.,1., 1.,0.)
    );

    gl_FragColor = vec4( normal_vis, 1. );
}

