//
// Minimal GLSL vertex shader.
//

void main( void )
{
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
