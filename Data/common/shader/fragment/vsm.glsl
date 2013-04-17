//
// VSM GLSL pixel shader.
//

varying vec3 position;

void main( void )
{
   // VSM
#if 1
   float depth  = length( position )*0.01;
   //vec2 moments = vec2( depth, depth*depth );
   //vec2 out2    = fract( moments * 32.0 );
   //vec2 out1    = moments-(out2/32.0);
   //gl_FragColor = vec4( out1, out2 );
   gl_FragColor = vec4( depth, depth*depth, 1.0, 1.0 );
#endif

   // VSM - JHoule compression.
#if 0
   float z  = length( position )*0.01;
   float zz = z*z;
   
   float f12 = z*4095;
   float b1  = floor( f12/256.0 );
   float f8  = f12-b1*256.0;
   
   float zzf12 = zz*4095;
   float zzb1  = floor( zzf12/256.0 );
   float zzf8  = zzf12-zzb1*256.0;
   
   gl_FragColor = vec4( b1*16.0/255.0, f8/255.0, zzb1*16.0/255.0, zzf8/255.0 );
#endif

   // VSM - Compression.
#if 0
   float z  = length( position )*0.01;
   float zz = z*z;
   vec4 decomp = floor( vec4(z*(65535.0/256.0), z*(65535.0), zz*(65535.0/256.0), zz*(65535.0) ) ) / 255.0;
   gl_FragColor = vec4( decomp.x, decomp.y-decomp.x*256.0, decomp.z, decomp.w-decomp.z*256.0 );
#endif   

   // ESM.
#if 0
   float depth = length( position )*0.01;
   float esm   = exp2( depth*10.0 );
   //float esm_e = floor( log2( esm ) );
   //float esm_m = esm/esm_e;
   //gl_FragColor = vec4( esm_e, esm_m, esm, log2( esm_e ) );
   gl_FragColor = vec4( esm, 0.0, 0.0, 0.0 );
   //gl_FragColor = vec4( depth, 0.0, 0.0, 0.0 );
#endif
   
}
