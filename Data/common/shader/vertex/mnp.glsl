//
// Mapping, normal and position GLSL vertex shader.
//

varying vec2 mapping;
varying vec3 normal;
varying vec3 position;

void main(void)
{
   mapping     = gl_MultiTexCoord0.xy;
   normal      = (gfxWorldMatrix * vec4(gl_Normal,0)).xyz;
   position    = (gfxWorldMatrix * gl_Vertex).xyz;   
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
