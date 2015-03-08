#version 120

/*
 * normals calc
 * code by
 * Inigo Quilez
 * http://www.iquilezles.org/www/articles/normals/normals.htm
 *
 * in: data 
 *  3d points in metric space
 *  proccessed by depth_to_p3d.frag
 * out: 
 *  normals
 */

uniform sampler2DRect data;

const int kernel = 1;

void main( void ) 
{
  vec2 p2 = gl_TexCoord[0].st;
  vec4 p3 = texture2DRect( data, p2 );

  vec3 normal = vec3(0.);

  vec3 x = vec3( p3 ); 

  for ( int i = 1; i <= kernel; i++ )
  {
    /* south */
    vec3 s = vec3(texture2DRect(data,vec2( p2.x, p2.y + i )));

    /* north */
    vec3 n = vec3(texture2DRect(data,vec2( p2.x, p2.y - i )));

    /* east */
    vec3 e = vec3(texture2DRect(data,vec2( p2.x - i, p2.y )));

    /* west */
    vec3 w = vec3(texture2DRect(data,vec2( p2.x + i, p2.y ))); 

    /* edges */
    vec3 xe = x - e;
    vec3 xw = x - w;

    /* faces normals */
    vec3 xen = cross( xe, n - e );
    vec3 xws = cross( xw, s - w );
    vec3 xes = cross( xe, e - s );
    vec3 xwn = cross( xw, w - n );

    normal += ( xen + xws + xes + xwn );
  }

  normal = normalize( normal );

  gl_FragColor = vec4( normal, 1. );
}

