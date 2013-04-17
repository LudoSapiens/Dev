//
// Mapping and normal GLSL vertex shader.
//

varying vec2 mapping;
varying vec3 normal;

void main(void)
{
   mapping     = gl_MultiTexCoord0.xy;
   normal      = (gfxWorldMatrix * vec4(gl_Normal,0)).xyz;
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
