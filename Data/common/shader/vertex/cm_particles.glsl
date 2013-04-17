//
// Color and mapping GLSL vertex shader for particles.
//

varying vec2 mapping;

void main(void)
{
   vec4 pos    = gfxWorldViewMatrix * gl_Vertex;
   vec2 offset = (gl_MultiTexCoord0.xy - 0.5) * gl_MultiTexCoord0.z;
   pos.xy     += offset;
   vec4 fPos   = gfxProjectionMatrix * pos;

   gl_FrontColor  = gl_Color;
   mapping        = gl_MultiTexCoord0.xy;
   gl_Position    = fPos;
}
