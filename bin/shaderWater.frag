// source
// https://gitlab.com/philippe.lucidarme/sfml_tutorial/blob/master/part_006/example_001.frag
#version 120
uniform sampler2D currentTexture;
uniform float time;

void main()
{
    vec2 coord = gl_TexCoord[0].xy;

    float factorx=0.05*( 1 + cos(2.*time));
    float factory=0.05*( 1 - sin(2.*time));
    coord.x = (coord.x+factorx)/(1.+2.*factorx);
    coord.y = (coord.y+factory)/(1.+2.*factory);

    vec4 pixel_color = texture2D(currentTexture, coord);

    gl_FragColor = pixel_color;
}