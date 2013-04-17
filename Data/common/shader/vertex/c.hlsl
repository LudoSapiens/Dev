//
// Color HLSL vertex shader.
//

struct VS_IN
{
   float4 pos: POSITION;
   float4 col: COLOR;
};

struct VS_OUT
{
   float4 pos: POSITION;
   float4 col: COLOR;
};

VS_OUT main( VS_IN In )
{
   VS_OUT Out;
   Out.col = In.col;
   Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos);
   return Out;
};
