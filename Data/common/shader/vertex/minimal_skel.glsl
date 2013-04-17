//
// Minimal GLSL vertex shader with skinning.
// 
uniform mat4 boneMatrices[30];

void main(void)
{
   vec4  bones   = gl_MultiTexCoord2;
   vec4  weights = gl_MultiTexCoord1;

   vec4 p = vec4( 0.0, 0.0, 0.0, 0.0 );

#if DYN_LOOP_SLOW
   p = p + (boneMatrices[int(bones.x)] * gl_Vertex) * weights.x;
   p = p + (boneMatrices[int(bones.y)] * gl_Vertex) * weights.y;
   p = p + (boneMatrices[int(bones.z)] * gl_Vertex) * weights.z;
   p = p + (boneMatrices[int(bones.w)] * gl_Vertex) * weights.w;
#else
   p = p + (boneMatrices[int(bones.x)] * gl_Vertex) * weights.x;
   if( weights.y > 0.0 )
   {
      p = p + (boneMatrices[int(bones.y)] * gl_Vertex) * weights.y;
      if( weights.z > 0.0 )
      {
         p = p + (boneMatrices[int(bones.z)] * gl_Vertex) * weights.z;
         if( weights.w > 0.0 )
         {
            p = p + (boneMatrices[int(bones.w)] * gl_Vertex) * weights.w;
         }
      }
   }
#endif

   gl_Position = gfxWorldViewProjectionMatrix * p;
}
