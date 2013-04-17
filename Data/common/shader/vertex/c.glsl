//
// Color GLSL vertex shader.
//

void main( void )
{
   gl_FrontColor = gl_Color;
   gl_Position   = gfxWorldViewProjectionMatrix * gl_Vertex;
}
