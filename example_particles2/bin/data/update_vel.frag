#version 120
#extension GL_ARB_texture_rectangle : enable

// data output for pingpong
uniform sampler2DRect vel;
// other data
uniform sampler2DRect pos;

uniform vec3 mouse;
uniform float radiusSquared;
uniform float elapsed;

void main()
{
    vec3 _pos = texture2DRect(pos, gl_TexCoord[0].st).xyz;
    vec3 _vel = texture2DRect(vel, gl_TexCoord[0].st).xyz;
    
    // mouse attraction
    vec3 direction = mouse - _pos.xyz;
    float distSquared = dot(direction, direction);
    float magnitude = 500.0 * (1.0 - distSquared / radiusSquared);
    vec3 force = step(distSquared, radiusSquared) * magnitude * normalize(direction);
    
    // gravity
    force += vec3(0.0, -0.5, 0.0);
    
    // accelerate
    _vel += elapsed * force;
    
    // bounce off the sides
    _vel.x *= step(abs(_pos.x), 512.0) * 2.0 - 1.0;
    _vel.y *= step(abs(_pos.y), 384.0) * 2.0 - 1.0;
    
    // damping
    _vel *= 0.995;
    
    gl_FragColor = vec4(_vel, 0.0);
    /*gl_FragData[0] = vec4(_vel, 0.0);*/
}

