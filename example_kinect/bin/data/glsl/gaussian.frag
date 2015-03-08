/*
 * gaussian filter
 * code from GPU Gems 3
 * Chapter 40. Incremental Computation of the Gaussian
 * http://callumhay.blogspot.com.ar/2010/09/gaussian-blur-shader-glsl.html
 */

uniform float sigma;     
// gaussian sigma: higher value means more blur
// a good value for 9x9 is around 3 to 5
// a good value for 7x7 is around 2.5 to 4
// a good value for 5x5 is around 2 to 3.5

uniform float size;  
// This should usually be equal to
// 1.0f / texture_pixel_width for a horizontal blur, and
// 1.0f / texture_pixel_height for a vertical blur.

//TODO uniforms
const float kernel = 4.0f;
const vec2  dir = vec2( 1.0f, 0.0f );

uniform sampler2D data;  
// Texture that will be blurred by this shader

const float PI = 3.14159265f;

void main() 
{
  vec2 loc = gl_TexCoord[0].xy; // center pixel cooordinate

  // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
  vec3 gauss;
  gauss.x = 1.0f / (sqrt(2.0f * PI) * sigma);
  gauss.y = exp(-0.5f / (sigma * sigma));
  gauss.z = gauss.y * gauss.y;

  vec4 avg = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float coeffsum = 0.0f;

  // Take the central sample first...
  avg += texture2D( data, loc ) * gauss.x;
  coeffsum += gauss.x;
  gauss.xy *= gauss.yz;

  // Go through the remaining 8 vertical samples (4 on each side of the center)
  for (float i = 1.0f; i <= kernel; i++) 
  { 
    avg += texture2D( data, loc - i * size * dir ) * gauss.x;
    avg += texture2D( data, loc + i * size * dir ) * gauss.x;

    coeffsum += 2 * gauss.x;
    gauss.xy *= gauss.yz;
  }

  gl_FragColor = avg / coeffsum;
}

/*
 * gaussian filter
 * code from GPU Gems 3
 * Chapter 40. Incremental Computation of the Gaussian
 */

/*uniform sampler2D data; // source image*/
/*uniform vec2 dir;       // horiz=(1.0, 0.0), vert=(0.0, 1.0)*/

/*const float sigma;*/
/*const int kernel = 3;*/
/*const float PI = 3.14159265f;*/

/*void main()*/
/*{*/
/*float norm = 1 / ( sqrt ( 2 * PI ) * sigma );*/

/*vec2 loc = gl_FragCoord.xy; // center pixel cooordinate*/
/*vec2 loc = gl_TexCoord[0].xy; // center pixel cooordinate*/
/*vec4 acc; // accumulator*/
/*acc = texture2D( data, loc ); // accumulate center pixel*/

/*for ( int i = 1; i <= kernel; i++ ) */
/*{*/
/*float coeff = exp(-0.5 * float(i) * float(i) / (sigma * sigma));*/
/*acc += (texture2D(data, loc - float(i) * dir)) * coeff;//L*/
/*acc += (texture2D(data, loc + float(i) * dir)) * coeff;//R*/
/*}*/
/*acc *= norm; // normalize for unity gain*/
/*gl_fragColor = acc;*/
/*}*/

