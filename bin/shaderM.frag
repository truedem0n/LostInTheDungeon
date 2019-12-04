// Since it is so hard to debug shaders I had to 
// do this simple shader because I wasted a lot
// of time trying to come up with something cool.

uniform float time;
void main()
{
    gl_FragColor = vec4(abs(sin(time)),abs(cos(time)),abs(tan(time)),1.0);
}

