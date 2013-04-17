//
// Vertex color with constant color GLSL fragment shader.
//
uniform vec4 color;

void main( void )
{
   gl_FragColor = gl_Color*color;
}
