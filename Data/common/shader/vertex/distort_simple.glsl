//
// GLSL vertex shader used for simple distortion.
//

varying vec3 position;
varying vec3 normal;
varying vec3 screenPosition;

void main(void)
{
   vec4 pos = gfxWorldViewProjectionMatrix * gl_Vertex;

   position    = (gfxWorldViewMatrix * gl_Vertex).xyz;
   normal      = (gfxWorldViewMatrix * vec4(gl_Normal,0)).xyz;
   screenPosition.xy = pos.xy * 0.5 + vec2( 0.5 * pos.w );
   screenPosition.z  = pos.w;
   gl_Position = pos;
}
