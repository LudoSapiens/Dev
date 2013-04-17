varying vec2 mapping;

// TexCoord1: Normal
// TexCoord2: Radius
void main(void)
{
   // In camera space.
   vec3 n = (gfxWorldViewMatrix * vec4( gl_MultiTexCoord1.xyz, 0.0 )).xyz;
   vec3 v = vec3( -n.y, n.x, 0.0 ); // cross( vec3(0,0,1), n );
   vec4 p = gfxWorldViewMatrix * gl_Vertex;
   p.xyz += v * gl_MultiTexCoord2.x;
   gl_Position = gfxProjectionMatrix * p;
   mapping.x = 0.0;
   mapping.y = sign( gl_MultiTexCoord2.x );
}
