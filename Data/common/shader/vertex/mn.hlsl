//
// Mapping and normal HLSL vertex shader.
//
struct VS_IN
{
   float4 pos:  POSITION;
   float2 map:  TEXCOORD0;
   float3 norm: NORMAL;
};
struct VS_OUT
{
   float4 pos:  POSITION;
   float2 map:  TEXCOORD0;
   float3 norm: NORMAL;
};

VS_OUT main( VS_IN IN )
{
   VS_OUT OUT;
   OUT.map  = IN.map;
   OUT.norm = mul( gfxWorldMatrix, IN.norm );
   OUT.pos  = mul( gfxWorldViewProjectionMatrix, IN.pos );
   return OUT;
}
