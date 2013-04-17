//
// Distribution-based BRDF
//
// Notes:
//  * All single-character vectors are normalized
//
uniform vec4 color;
uniform vec3 gfxCameraPosition;
uniform sampler2D colorTex;
//uniform sampler2DShadow light_depth;
uniform sampler2D light_depth;
uniform sampler2D depthTex;
uniform vec3 light_position;
uniform vec3 light_color;
uniform mat4 light_matrix;

varying vec2 mapping;
varying vec3 position;
varying vec3 normal;
varying vec4 tangent;

vec3 velvet( float theta )
{
   float c = 0.25;
   float cotan_theta = cos(theta)/sin(theta);
   float v = c * (1.0 + 4.0*exp(-(cotan_theta*cotan_theta)));
   return vec3(v);
}

vec3 anisoPhong( float theta, float phi )
{
   vec2 rough   = vec2( 10.0, 100.0 );
   float cosPhi = cos( phi );
   float sinPhi = sin( phi );
   float ep     = rough.x * cosPhi*cosPhi + rough.y * sinPhi*sinPhi;
   return vec3( pow( cos(theta), ep ) + 0.2 );
}

void main(void)
{
   vec3 i = normalize(light_position-position); //aka k1
   vec3 o = normalize(gfxCameraPosition-position); //aka k2

   vec3 n = normalize(normal);
   //vec3 t = normalize(tangent.xyz);
   vec3 h = normalize(i + o);


   float n_dot_h = dot(n, h);
   float i_dot_n = max( 0.0, dot(i, n) );
   float o_dot_n = dot(o, n);
   
   //float kh = min( 1.0, dot(i, h) );  //or dot(o, h)

   //vec3 h_tangent = normalize(h - n*(n_dot_h));

   //float theta = acos(n_dot_h);
   //float phi = acos(dot(t, h_tangent));

   //Convert theta in [0, Pi] and phi in [0, 2Pi] into
   //[0, 1] texture normalized texture coordinates
   //vec2 brdfCoord = vec2(theta, phi)*vec2(ooPi, oo2Pi);


   //Actual dBRDF calculation
   
   // Velvet
   //vec3 ph = velvet(theta);
   
   // Lambert
   vec3 ph = vec3(0.7);
   
   // Blinn
   //vec3 ph = vec3(pow(cos(theta), 100.0) + 0.3);
   
   // Anisotropic
   //vec3 ph = anisoPhong(theta, phi);
   
   
   // Shadow
#if 0   
   vec4 lpos = light_matrix * vec4( position, 1.0 );
   lpos.z   -= 0.001;
   float sh  = shadow2DProj( light_depth, lpos ).r;
   //float sh  = texture2DProj( light_depth, lpos ).r;
   //sh = sh > (lpos.z/lpos.w)/*-0.00001*/ ? 1.0 : 0.0;
#endif
#if 0  
   vec4 lpos = light_matrix * vec4( position, 1.0 );
   lpos.z   -= 0.001;
   const float SHADOW_SIZE = 1024.0;
   float sh = 0.0;
   for( int y = -2; y <= 2; y++ )
      for( int x = -2; x <= 2; x++ )
      {
         vec4 coord = lpos;
         coord.xy += (vec2(x,y)/SHADOW_SIZE)*lpos.w;
         sh += shadow2DProj( light_depth, coord ).r;
      }
   sh = smoothstep( 1, 23, sh );
#endif
#if 0
   vec4 lpos = light_matrix * vec4( position, 1.0 );
   lpos.z   -= 0.001;
   const float SHADOW_SIZE = 1024.0;

   const float r = 2.0;
   vec2 t0 = vec2(1.0,1.0)-fract(SHADOW_SIZE*(lpos.xy/lpos.w)-vec2(r,r));
   vec2 t1 = fract(SHADOW_SIZE*(lpos.xy/lpos.w)+vec2(r,r));
   
   float d  = ( 1.0 / SHADOW_SIZE ) * lpos.w;
   float d2 = ( 2.0 / SHADOW_SIZE ) * lpos.w;
   
   float sh = 0.0;
   
   sh += shadow2DProj( light_depth, lpos + vec4( -d2, -d2, 0.0, 0.0 ) ).r * t0.x * t0.y;
   sh += shadow2DProj( light_depth, lpos + vec4(  -d, -d2, 0.0, 0.0 ) ).r * 1.0  * t0.y;
   sh += shadow2DProj( light_depth, lpos + vec4( 0.0, -d2, 0.0, 0.0 ) ).r * 1.0  * t0.y;
   sh += shadow2DProj( light_depth, lpos + vec4(   d, -d2, 0.0, 0.0 ) ).r * 1.0  * t0.y;
   sh += shadow2DProj( light_depth, lpos + vec4(  d2, -d2, 0.0, 0.0 ) ).r * t1.x * t0.y;

   sh += shadow2DProj( light_depth, lpos + vec4( -d2, -d, 0.0, 0.0 ) ).r * t0.x * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(  -d, -d, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4( 0.0, -d, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(   d, -d, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(  d2, -d, 0.0, 0.0 ) ).r * t1.x * 1.0;
   
   sh += shadow2DProj( light_depth, lpos + vec4( -d2, 0.0, 0.0, 0.0 ) ).r * t0.x * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(  -d, 0.0, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4( 0.0, 0.0, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(   d, 0.0, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(  d2, 0.0, 0.0, 0.0 ) ).r * t1.x * 1.0;
   
   sh += shadow2DProj( light_depth, lpos + vec4( -d2, d, 0.0, 0.0 ) ).r * t0.x * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(  -d, d, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4( 0.0, d, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(   d, d, 0.0, 0.0 ) ).r * 1.0  * 1.0;
   sh += shadow2DProj( light_depth, lpos + vec4(  d2, d, 0.0, 0.0 ) ).r * t1.x * 1.0;
   
   sh += shadow2DProj( light_depth, lpos + vec4( -d2, d2, 0.0, 0.0 ) ).r * t0.x * t1.y;
   sh += shadow2DProj( light_depth, lpos + vec4(  -d, d2, 0.0, 0.0 ) ).r * 1.0  * t1.y;
   sh += shadow2DProj( light_depth, lpos + vec4( 0.0, d2, 0.0, 0.0 ) ).r * 1.0  * t1.y;
   sh += shadow2DProj( light_depth, lpos + vec4(   d, d2, 0.0, 0.0 ) ).r * 1.0  * t1.y;
   sh += shadow2DProj( light_depth, lpos + vec4(  d2, d2, 0.0, 0.0 ) ).r * t1.x * t1.y;
   
   sh /= r*r*4.0;
#endif
#if 0
   const float SHADOW_SIZE = 1024.0;
   const vec2 sample_array[20] = {
        
      vec2(  1.0,  0.0 ) / SHADOW_SIZE,
      vec2(  0.0,  1.0 ) / SHADOW_SIZE,
      vec2(  0.0, -1.0 ) / SHADOW_SIZE,
      vec2( -1.0,  0.0 ) / SHADOW_SIZE,

      vec2( -1.0, -1.0 ) / SHADOW_SIZE,
      vec2(  1.0, -1.0 ) / SHADOW_SIZE,
      vec2( -1.0,  1.0 ) / SHADOW_SIZE,
      vec2(  1.0,  1.0 ) / SHADOW_SIZE,       

      vec2(  0.0, -2.0 ) / SHADOW_SIZE,
      vec2( -2.0,  0.0 ) / SHADOW_SIZE,
      vec2(  0.0,  2.0 ) / SHADOW_SIZE,
      vec2(  2.0,  0.0 ) / SHADOW_SIZE,
        
      vec2( -1.0, -2.0 ) / SHADOW_SIZE,
      vec2(  1.0, -2.0 ) / SHADOW_SIZE,
      vec2( -2.0, -1.0 ) / SHADOW_SIZE,
      vec2( -2.0,  1.0 ) / SHADOW_SIZE,
        
      vec2( -1.0,  2.0 ) / SHADOW_SIZE,
      vec2(  1.0,  2.0 ) / SHADOW_SIZE,
      vec2(  2.0, -1.0 ) / SHADOW_SIZE,
      vec2(  2.0,  1.0 ) / SHADOW_SIZE,
   };
  
   vec4 lpos = light_matrix * vec4( position, 1.0 );
   lpos.z   -= 0.00005*lpos.w;

   float sh = shadow2DProj( light_depth, lpos ).r;
   vec4 offset  = vec4(0.0);
   for ( int i=0; i<19; i++ )
   {
      offset.x = dot(sample_array[i].xy, vec2(0.793353,-0.608761));
      offset.y = dot(sample_array[i].xy, vec2(0.608761, 0.793353));
      sh += shadow2DProj( light_depth, lpos + offset*lpos.w).r;
   }
   sh = smoothstep(1.0, 19.0, sh);
#endif
// HERE
#if 0
   // VSM
   // TODO: mip mapping LOD choose with distance from occludder!!!

   float t    = length( light_position-position )*0.01;
   vec4 lpos  = light_matrix * vec4( position, 1.0 );
   vec4 m     = vec4(0.0, 0.0, 0.0, 0.0);

#if 0
   m = texture2DProj( light_depth, lpos );
#endif

#if 0
   const float SHADOW_SIZE = 1024.0;
   float d  = ( 1.0 / SHADOW_SIZE ) * lpos.w;
   m += texture2DProj( light_depth, lpos + vec4(-d,-d,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4( 0,-d,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4( d,-d,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4(-d, 0.0,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4( 0, 0.0,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4( d, 0.0,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4(-d, d,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4( 0, d,0.0,0.0) );
   m += texture2DProj( light_depth, lpos + vec4( d, d,0.0,0.0) );
   m /= 9.0;
#endif
   
   
 #if 0
   const float SHADOW_SIZE = 512.0;
   vec2 t1  = fract(SHADOW_SIZE*(lpos.xy/lpos.w));
   vec2 t0  = vec2(1.0, 1.0) - t1;
   float d  = ( 1.0 / SHADOW_SIZE ) * lpos.w;
   m += texture2DProj( light_depth, lpos + vec4( -d,  -d, 0.0, 0.0 ) ) * t0.x * t0.y;
   m += texture2DProj( light_depth, lpos + vec4( 0.0, -d, 0.0, 0.0 ) ) * 1.0  * t0.y;
   m += texture2DProj( light_depth, lpos + vec4(  d,  -d, 0.0, 0.0 ) ) * t1.x * t0.y;
   m += texture2DProj( light_depth, lpos + vec4( -d,  0.0, 0.0, 0.0 ) ) * t0.x * 1.0;
   m += texture2DProj( light_depth, lpos + vec4( 0.0, 0.0, 0.0, 0.0 ) ) * 1.0  * 1.0;
   m += texture2DProj( light_depth, lpos + vec4(  d,  0.0, 0.0, 0.0 ) ) * t1.x * 1.0;
   m += texture2DProj( light_depth, lpos + vec4( -d,  d, 0.0, 0.0 ) ) * t0.x * t1.y;
   m += texture2DProj( light_depth, lpos + vec4( 0.0, d, 0.0, 0.0 ) ) * 1.0  * t1.y;
   m += texture2DProj( light_depth, lpos + vec4(  d,  d, 0.0, 0.0 ) ) * t1.x * t1.y;
   
   m /= 4.0;
#endif
#if 1
   const float SHADOW_SIZE = 512.0;
   vec2 t1  = fract(SHADOW_SIZE*(lpos.xy/lpos.w));
   vec2 t0  = vec2(1.0,1.0)-t1;
   float d  = ( 1.0 / SHADOW_SIZE ) * lpos.w;
   m += texture2DProj( light_depth, lpos + vec4( 0.0, 0.0, 0.0, 0.0 ) ) * t0.x * t0.y;
   m += texture2DProj( light_depth, lpos + vec4( d,   0.0, 0.0, 0.0 ) ) * t1.x * t0.y;
   m += texture2DProj( light_depth, lpos + vec4( 0.0, d, 0.0, 0.0 ) ) * t0.x * t1.y;
   m += texture2DProj( light_depth, lpos + vec4( d,   d, 0.0, 0.0 ) ) * t1.x * t1.y;
#endif

   m.xy += m.zw/32.0;

   float p        = (t <= m.x) ? 1.0 : 0.0;
   float variance = max( m.y - (m.x*m.x), 0.00004 );
   float dl       = t - m.x;
   float p_max    = variance / (variance + dl*dl);
   float sh       = max( p, p_max );
   sh             = smoothstep( 0.5, 1.0, sh );
#else
   // No shadow.
   float sh = 1.0;
#endif

#if 1
   // SSAO.
   const vec2 SCREEN_SIZE_INV = vec2( 1.0/1280.0, 1.0/720.0 );
   vec2 screenCoord = gl_FragCoord.xy * SCREEN_SIZE_INV;

   float z = texture2D( depthTex, screenCoord ).x;
   float minZ = z - 1.0;

   // Hardcoded sample kernel.
   float zAvg = 0.0;
   float numZ = 0.0;
   const int kernelHalfSize = 3;

#if 1
   for( int y = -kernelHalfSize; y <= kernelHalfSize; ++y )
   {
      for( int x = -kernelHalfSize; x <= kernelHalfSize; ++x )
      {
         vec2 coord = screenCoord + vec2(x,y)*SCREEN_SIZE_INV;
         float newZ = texture2D( depthTex, coord ).x;
         //if( minZ < newZ  )
         {
            zAvg += newZ;
            ++numZ;
         }
      }
   }
#else
   for( int y = -kernelHalfSize; y <= -1; ++y )
   {
      for( int x = -kernelHalfSize; x <= kernelHalfSize; ++x )
      {
         vec2 coord = screenCoord + vec2(x,y)*SCREEN_SIZE_INV;
         zAvg += texture2D( depthTex, coord ).x;
         ++numZ;
      }
      //for( int x = 1; x <= kernelHalfSize; ++x )
      //{
      //   vec2 coord = screenCoord + vec2(x,y)*SCREEN_SIZE_INV;
      //   zAvg += texture2D( depthTex, coord ).x;
      //   ++numZ;
      //}
   }
   for( int x = -kernelHalfSize; x <= -1; ++x )
   {
      vec2 coord = screenCoord + vec2(x,0)*SCREEN_SIZE_INV;
      zAvg += texture2D( depthTex, coord ).x;
      ++numZ;
   }
   zAvg += z;
   ++n;
   for( int x = 1; x <= kernelHalfSize; ++x )
   {
      vec2 coord = screenCoord + vec2(x,0)*SCREEN_SIZE_INV;
      zAvg += texture2D( depthTex, coord ).x;
      ++numZ;
   }
   for( int y = 1; y <= kernelHalfSize; ++y )
   {
      for( int x = -kernelHalfSize; x <= kernelHalfSize; ++x )
      {
         vec2 coord = screenCoord + vec2(x,y)*SCREEN_SIZE_INV;
         zAvg += texture2D( depthTex, coord ).x;
         ++numZ;
      }
      //for( int x = 1; x <= kernelHalfSize; ++x )
      //{
      //   vec2 coord = screenCoord + vec2(x,y)*SCREEN_SIZE_INV;
      //   zAvg += texture2D( depthTex, coord ).x;
      //   ++numZ;
      //}
   }
#endif // zAvg accumulation

   zAvg /= numZ;
   //zAvg /= 7.0*7.0;

   float zDif = (z - zAvg); // negative means tip of convex region
   //const float sphereSize = 1.0;
   //zDif /= sphereSize;
   float ao = 1.0 - smoothstep(0.0, 1.0, zDif);
   ao = pow(ao, 100.0);
#endif


   //Schlick's polynomial approximation
   float r0   = 1.0;
   //float fkh  = r0 + (1.0-r0)*pow((1.0-kh), 5.0);
   float fkh  = 1.0;
   float att  = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n)); //denominator
   vec4 dbrdf = vec4( light_color*ph*fkh*att*sh*ao, 1.0 );

   gl_FragColor = dbrdf * color;//*texture2D( colorTex, mapping );
#if 0
   gl_FragColor = vec4(zAvg*0.05);
   if( screenCoord.x < screenCoord.y )
   {
      // Top left.
      gl_FragColor = vec4(ao);
      //gl_FragColor = vec4(z * 0.02);
   }
   else
   {
      // Bottom right.
      gl_FragColor = vec4(ao);
      //gl_FragColor = vec4(zAvg * 0.02);
   }
#endif
}
