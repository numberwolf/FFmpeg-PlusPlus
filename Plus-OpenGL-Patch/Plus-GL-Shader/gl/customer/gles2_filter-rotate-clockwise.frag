//precision highp float;
varying vec2 TextureCoordsVarying;

const float inputHeight = 2560.0;
const float inputWidth = 1600.0;
uniform float playTime;

const float blurStep = 0.5;
const mat4 u_InvModel = mat4(
   1.0,    0.0,     0.0,    0.0,
   0.0,    1.0,      0.0,    0.0,
   0.0,    0.0,    1.0,    0.0,
   0.0,    0.0,    0.0,    1.0
);
const vec2 blurDirection = vec2(0.5, 0.5);

uniform sampler2D tex;

const float PI = 3.141592653589793;

/* random number between 0 and 1 */
float random(in vec3 scale, in float seed) {
    /* use the fragment position for randomness */
    return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);
}

vec4 crossFade(in vec2 uv, in float dissolve) {
    return texture2D(tex, uv).rgba;
}

vec4 directionBlur(sampler2D tex, vec2 resolution, vec2 uv, vec2 directionOfBlur, float intensity)
{
    vec2 pixelStep = 1.0/resolution * intensity;
    float dircLength = length(directionOfBlur);
	pixelStep.x = directionOfBlur.x * 1.0 / dircLength * pixelStep.x;
	pixelStep.y = directionOfBlur.y * 1.0 / dircLength * pixelStep.y;

    float minVal = min(100.0, playTime * 50.0);
    int num = 100 - int(minVal);
	vec4 color = vec4(0);
	for(int i = -num; i <= num; i++)
	{
       vec2 blurCoord = uv + pixelStep * float(i);
	   vec2 uvT = vec2(1.0 - abs(abs(blurCoord.x) - 1.0), 1.0 - abs(abs(blurCoord.y) - 1.0));
	   color += texture2D(tex, uvT);
	}
	color /= float(2 * num + 1);	
	return color;
}

void main() {

    float ratio = inputWidth / inputHeight;

    vec2 uv = (u_InvModel * vec4((TextureCoordsVarying.x * 2.0 - 1.0) * ratio, TextureCoordsVarying.y * 2.0 - 1.0, 0.0, 1.0)).xy;

    uv.x = (uv.x / ratio + 1.0) / 2.0;
    uv.y = (uv.y + 1.0) / 2.0;

	vec2 resolution = vec2(inputWidth,inputHeight);
	//vec2 resolution = vec2(720.0,1280.0);
	vec4 resultColor = directionBlur(tex,resolution,uv,blurDirection, blurStep);
	gl_FragColor = vec4(resultColor.rgb, resultColor.a) * step(uv.x, 2.0) * step(uv.y, 2.0) * step(-1.0, uv.x) * step(-1.0, uv.y);

}
