//
// Color and mapping GLSL vertex shader for particles.
//

varying vec2 mapping;
varying vec3 screenPosition;

void main(void)
{
   vec4 pos    = gfxWorldViewMatrix * gl_Vertex;
   vec2 offset = (gl_MultiTexCoord0.xy - 0.5) * gl_MultiTexCoord0.z;
   pos.xy     += offset;
   vec4 fPos   = gfxProjectionMatrix * pos;
   pos.xy      = (fPos.xy/fPos.w)*0.5 + vec2(0.5);
   pos.z      /= 128.0;

   gl_FrontColor  = gl_Color;
   mapping        = gl_MultiTexCoord0.xy;
   screenPosition = pos.xyz;
   gl_Position    = fPos;
}
