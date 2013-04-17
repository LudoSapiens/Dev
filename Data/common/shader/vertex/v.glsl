//
// View position GLSL vertex shader.
//

varying vec3 position;

void main( void )
{
   position    = (gfxWorldViewMatrix * gl_Vertex).xyz;
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
