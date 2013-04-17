//
// GLSL vertex shader.
// --------------------
//  m: mapping
//  p: position
//

varying vec2 mapping;
varying vec3 position;
//varying vec3 normal;

void main(void)
{
   mapping     = gl_MultiTexCoord0.xy;
   position    = (gfxWorldMatrix * gl_Vertex).xyz;
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
