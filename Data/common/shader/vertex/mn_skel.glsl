//
// Mapping and normal GLSL vertex shader with skinning.
// 
uniform mat4 boneMatrices[33];
varying vec3 normal;
varying vec2 mapping;

void main(void)
{
   vec4  bones   = gl_MultiTexCoord2;
   vec4  weights = gl_MultiTexCoord1;
   float nbBones = gl_MultiTexCoord3.x;

   vec4 p = vec4( 0.0, 0.0, 0.0, 0.0 );
   vec4 n = vec4( 0.0, 0.0, 0.0, 0.0 );

#if DYN_LOOP_SLOW
   for( int i = 0; i < 4; ++i )
#else
   for( float i = 0.0; i < nbBones; i += 1.0 )
#endif
   {
      p = p + (boneMatrices[int(bones.x)] * gl_Vertex) * weights.x;
      n = n + (boneMatrices[int(bones.x)] * vec4(gl_Normal,0)) * weights.x;

      bones   = bones.yzwx;
      weights = weights.yzwx;
   }

   mapping     = gl_MultiTexCoord0.xy;
   normal      = (gfxWorldMatrix * n).xyz;
   gl_Position = gfxWorldViewProjectionMatrix * p;
}
