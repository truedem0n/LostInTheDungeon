// source
// https://www.shadertoy.com/view/lds3RH
// modified to work for this assignment

#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform sampler2D currentTexture;

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	
	float w = (0.5 - (uv.x));
    float h = 0.5 - uv.y;
	float distanceFromCenter = sqrt(w * w + h * h);
	
	float sinArg = distanceFromCenter * 10.0 - time * 10.0;
	float slope = cos(sinArg) ;
	vec4 color = texture2D(currentTexture, uv + normalize(vec2(w, h)) * slope * 0.05);
	
	gl_FragColor = color;
}