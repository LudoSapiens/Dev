//
// View position GLSL vertex shader with skinning.
//

uniform mat4 boneMatrices[33];

varying vec3 position;

mat3 toMat3( mat4 m )
{
   //return mat3(m)
   return mat3( vec3(m[0]), vec3(m[1]), vec3(m[2]) );
}

void main(void)
{
   vec4  bones   = gl_MultiTexCoord2;
   vec4  weights = gl_MultiTexCoord1;
   float nbBones = gl_MultiTexCoord3.x;

   vec4 p = vec4( 0.0, 0.0, 0.0, 0.0 );

#if DYN_LOOP_SLOW
   for( int i = 0; i < 4; ++i )
#else
   for( float i = 0.0; i < nbBones; i += 1.0 )
#endif
   {
      p       = p + (boneMatrices[int(bones.x)] * gl_Vertex) * weights.x;
      bones   = bones.yzwx;
      weights = weights.yzwx;
   }

   position    = (gfxWorldViewMatrix * p).xyz;
   gl_Position = gfxWorldViewProjectionMatrix * p;
}
