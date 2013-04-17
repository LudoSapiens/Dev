//
// Color and mapping HLSL vertex shader.
//

struct VS_IN
{
   float4 pos: POSITION;
   float4 col: COLOR;
   float2 map: TEXCOORD0;
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
   Out.col = In.col;
   Out.map = In.map;
   Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos );
   return Out;
}
