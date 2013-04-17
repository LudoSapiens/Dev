//
// GLSL vertex shader with skinning.
// --------------------
//  m: mapping
//  n: normal
//  p: position
//

uniform mat4 boneMatrices[30];
varying vec2 mapping;
varying vec3 position;
varying vec3 normal;

void main(void)
{
   vec4  weights = gl_MultiTexCoord1;
   vec4  bones   = gl_MultiTexCoord2;

   vec4 p = vec4( 0.0, 0.0, 0.0, 0.0 );
   vec4 n = vec4( 0.0, 0.0, 0.0, 0.0 );

#if DYN_LOOP_SLOW
   p = p + (boneMatrices[int(bones.x)] * gl_Vertex) * weights.x;
   n = n + (boneMatrices[int(bones.x)] * vec4(gl_Normal,0)) * weights.x;
   p = p + (boneMatrices[int(bones.y)] * gl_Vertex) * weights.y;
   n = n + (boneMatrices[int(bones.y)] * vec4(gl_Normal,0)) * weights.y;
   p = p + (boneMatrices[int(bones.z)] * gl_Vertex) * weights.z;
   n = n + (boneMatrices[int(bones.z)] * vec4(gl_Normal,0)) * weights.z;
   p = p + (boneMatrices[int(bones.w)] * gl_Vertex) * weights.w;
   n = n + (boneMatrices[int(bones.w)] * vec4(gl_Normal,0)) * weights.w;
#else
   p = p + (boneMatrices[int(bones.x)] * gl_Vertex) * weights.x;
   n = n + (boneMatrices[int(bones.x)] * vec4(gl_Normal,0)) * weights.x;
   if( weights.y > 0.0 )
   {
      p = p + (boneMatrices[int(bones.y)] * gl_Vertex) * weights.y;
      n = n + (boneMatrices[int(bones.y)] * vec4(gl_Normal,0)) * weights.y;
      if( weights.z > 0.0 )
      {
         p = p + (boneMatrices[int(bones.z)] * gl_Vertex) * weights.z;
         n = n + (boneMatrices[int(bones.z)] * vec4(gl_Normal,0)) * weights.z;
         if( weights.w > 0.0 )
         {
            p = p + (boneMatrices[int(bones.w)] * gl_Vertex) * weights.w;
            n = n + (boneMatrices[int(bones.w)] * vec4(gl_Normal,0)) * weights.w;
         }
      }
   }
#endif

   mapping     = gl_MultiTexCoord0.xy;
   normal      = (gfxWorldMatrix * n).xyz;
   position    = (gfxWorldMatrix * p).xyz;
   gl_Position = gfxWorldViewProjectionMatrix * p;
}
