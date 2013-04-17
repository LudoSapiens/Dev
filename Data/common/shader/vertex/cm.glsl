//
// Color and mapping GLSL vertex shader.
//

varying vec2 mapping;

void main( void )
{
   gl_FrontColor = gl_Color;
   mapping       = gl_MultiTexCoord0.xy;
   gl_Position   = gfxWorldViewProjectionMatrix * gl_Vertex;
}
