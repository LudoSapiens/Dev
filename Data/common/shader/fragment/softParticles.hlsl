//
// Particle fragment shader
//

sampler tex0;
sampler depthTex;

struct PS_IN
{
   float4 col:  COLOR0;
   float2 map:  TEXCOORD0;
   float3 spos: TEXCOORD1;
};

struct PS_OUT
{
   float4 col: COLOR;
};

PS_OUT main( PS_IN In )
{
   float4 particleColor = In.col * tex2D( tex0, In.map );
   float sceneDepth     = tex2D( depthTex, In.spos.xy ).x;
   float depthDiff      = sceneDepth + In.spos.z;
   depthDiff            = min( 1.0, depthDiff*32.0 );

   PS_OUT Out;
   Out.col = particleColor*depthDiff;
   return Out;
}
