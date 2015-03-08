#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect postex;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(texture2DRect(postex, gl_TexCoord[0].st).xyz, 1.0);
}
