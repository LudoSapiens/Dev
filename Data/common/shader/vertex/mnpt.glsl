//
// GLSL vertex shader.
// --------------------
//  m: mapping
//  n: normal
//  p: position
//  t: tangent
//

varying vec2 mapping;
varying vec3 position;
varying vec3 normal;
varying vec4 tangent;

void main(void)
{
   mapping     = gl_MultiTexCoord0.xy;
   position    = (gfxWorldMatrix * gl_Vertex).xyz;
   normal      = (gfxWorldMatrix * vec4(gl_Normal,0)).xyz;
   tangent.xyz = (gfxWorldMatrix * vec4(gfxTangent.xyz,0)).xyz;
   tangent.w   = gfxTangent.w; //restore sign

   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
