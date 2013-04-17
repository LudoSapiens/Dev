//
// Skeletal skinning GLSL vertex shader.
//
uniform mat4 boneMatrices[30];

mat3 toMat3( mat4 m )
{
   //return mat3(m)
   return mat3( vec3(m[0]), vec3(m[1]), vec3(m[2]) );
}

void main(void)
{
   vec4  bones   = gl_MultiTexCoord2;
   vec4  weights = gl_MultiTexCoord1;

   const float b = 1.0;
   float w = 0.0;
   if( bones.x == b ) w = weights.x;
   if( bones.y == b ) w = weights.y;
   if( bones.z == b ) w = weights.z;
   if( bones.w == b ) w = weights.w;

   const float b2 = 6.0;
   float w2 = 0.0;
   if( bones.x == b2 ) w2 = weights.x;
   if( bones.y == b2 ) w2 = weights.y;
   if( bones.z == b2 ) w2 = weights.z;
   if( bones.w == b2 ) w2 = weights.w;
   w = max( w, w2 );

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

   gl_FrontColor = mix( vec4(0,0,1,1), vec4(1,0,0,1), w );
   //gl_FrontColor = vec4( boneMatrices[0][2].xyz, 1.0 );
   //gl_FrontColor = vec4( normalize((p.xyz)-vec3(0.0,8.0,0.0)), 1.0 );
   //gl_FrontColor = vec4( normalize(gl_Vertex.xyz), 1.0 );
   gl_Position   = gfxWorldViewProjectionMatrix * p;
   //gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
}
