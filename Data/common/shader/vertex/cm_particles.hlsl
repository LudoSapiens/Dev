//
// Color and mapping HLSL vertex shader for particles.
//

struct VS_IN
{
   float4 pos: POSITION;
   float4 col: COLOR;
   float3 map: TEXCOORD0;
};

struct VS_OUT
{
   float4 pos: POSITION;
   float4 col: COLOR;
   float2 map: TEXCOORD0;
};

VS_OUT main( VS_IN In )
{
   VS_OUT Out;
   float4 pos    = mul( gfxWorldViewMatrix, In.pos );
   float2 offset = (In.map.xy - 0.5) * In.map.z;
   pos.xy       += offset;

   Out.col = In.col;
   Out.map = In.map.xy;
   Out.pos = mul( gfxProjectionMatrix, pos );

   return Out;
}
