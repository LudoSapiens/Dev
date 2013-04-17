//
// Mapping GLSL vertex shader.
//

varying vec2 mapping;

void main(void)
{
   mapping     = gl_MultiTexCoord0.xy;
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
