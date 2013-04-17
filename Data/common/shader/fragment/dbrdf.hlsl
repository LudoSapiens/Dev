//
// Distribution-based BRDF
//
// Notes:
//  * All single-character vectors are normalized
//

float   dbrdfId;
float4  color;
sampler dbrdfTex;

// Light information.
float    numLights;
sampler  light0_depth;
float3   light0_color;
float3   light0_position;
float4x4 light0_matrix;
sampler  light1_depth;
float3   light1_color;
float3   light1_position;
float4x4 light1_matrix;
sampler  light2_depth;
float3   light2_color;
float3   light2_position;
float4x4 light2_matrix;
sampler  light3_depth;
float3   light3_color;
float3   light3_position;
float4x4 light3_matrix;

struct PS_INPUT
{
   float2 map : TEXCOORD0;
   float4 pos:  TEXCOORD1;
   float3 norm: TEXCOORD2;
   float3 tan : TEXCOORD3;
};

float vsm( float2 moments, float z )
{
   float variance = max( moments.y - (moments.x*moments.x), 0.00004 );
   float dl       = max( 0.0, z - moments.x );
   float sh       = variance / (variance + dl*dl);
   return smoothstep( 0.5, 1.0, sh );
}

float vsm_shadow( float3 position, float3 posToLight, float4x4 light_matrix, sampler light_depth )
{
   const float dsf  = 0.01; // Some scale factor used for the 't' term.
   float t     = length( posToLight ) * dsf;
   float4 lpos = mul( light_matrix, float4( position, 1.0 ) );
   float2 t1   = frac( 512.0 * (lpos.xy/lpos.w) );
   float2 t0   = float2( 1.0, 1.0 ) - t1;
   float d     = (1.0/512.0) * lpos.w;

   float4 m = float4( 0.0, 0.0, 0.0, 0.0 );
   m += tex2Dproj( light0_depth, lpos + float4( 0.0, 0.0, 0.0, 0.0 ) ) * (t0.x * t0.y);
   m += tex2Dproj( light0_depth, lpos + float4(   d, 0.0, 0.0, 0.0 ) ) * (t1.x * t0.y);
   m += tex2Dproj( light0_depth, lpos + float4( 0.0,   d, 0.0, 0.0 ) ) * (t0.x * t1.y);
   m += tex2Dproj( light0_depth, lpos + float4(   d,   d, 0.0, 0.0 ) ) * (t1.x * t1.y);
   //m.xy += m.zw/32.0;

   return vsm( m.xy, t );
}

float spot( float3 i, float4x4 light_matrix )
{
   float3 spotDir = light_matrix[2].xyz;
   float  spotAtt = dot( normalize(spotDir), -i );
   return smoothstep( 0.5, 0.6, spotAtt );
}

float4 main( PS_INPUT IN ) : COLOR
{
   // Common terms.
   float3 o      = normalize( gfxCameraPosition-IN.pos ); //aka k2
   float3 n      = normalize( IN.norm );
   float o_dot_n = dot( o, n );
   
   float4 allLights; // Accumulating all of the lights energy.
   
   // CameraLight.
   {
      float ph   = tex2D( dbrdfTex, float2(o_dot_n, dbrdfId) );
      float dist = distance( gfxCameraPosition, IN.pos );
      float att  = (1.0 - smoothstep( 3.5, 6.0, dist ))/(2.0-o_dot_n);
      allLights  = float4( float3(0.5,0.5,0.5)*ph*att, 1.0 );
   }
   
   //if( numLights >= 1.0 )
   {
      float3 l      = light0_position - IN.pos;
      float3 i      = normalize( l ); //aka k1
      float3 h      = normalize( i + o );
      float n_dot_h = dot(n, h);
      float i_dot_n = max( 0.0, dot(i, n) );

      float sh      = vsm_shadow( IN.pos, l, light0_matrix, light0_depth );
      sh           *= spot( i, light0_matrix );
   
      float att     = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n));
      att           = min( 1.0, att );
      float ph      = tex2D( dbrdfTex, float2(n_dot_h, dbrdfId) );
      
      allLights    += float4( light0_color*ph*att*sh, 1.0 );
   }
   
   allLights += float4( 0.1, 0.1, 0.1, 0.0 ) * o_dot_n;

   return allLights * color + color*color.a;
}
