#version 120
#extension GL_ARB_texture_rectangle : enable

// data output for pingpong
uniform sampler2DRect pos;
// other data
uniform sampler2DRect vel;

uniform float elapsed;

void main()
{
    vec3 _pos = texture2DRect(pos, gl_TexCoord[0].st).xyz;
    vec3 _vel = texture2DRect(vel, gl_TexCoord[0].st).xyz;
    
    _pos += elapsed * _vel;
    
    gl_FragColor = vec4(_pos, 1.0);
    /*gl_FragData[0] = vec4(_pos, 1.0);*/
}

