//precision highp float;

attribute vec4 position;
//attribute vec2 texcoord0;
varying vec2 TextureCoordsVarying;

uniform float playTime;

void main() 
{ 
    mat4 u_VP = mat4(
       1.0,    0.0,    0.0,    0.0,
       0.0,    1.0,    0.0,    -1.0 + min(1.0, playTime),
       0.0,    0.0,    1.0,    0.0,
       0.0,    0.0,    0.0,    1.0
    );
    gl_Position = position * u_VP;
    // gl_Position = sign(u_VP * vec4(position.xy, 0.0, 1.0));
    TextureCoordsVarying.x = position.x * 0.5 + 0.5;
    TextureCoordsVarying.y = position.y * 0.5 + 0.5;
    //TextureCoordsVarying.y = 1.0 - TextureCoordsVarying.y;
}
