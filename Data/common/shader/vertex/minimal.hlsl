//
// Minimal HLSL vertex shader.
//

struct VS_IN
{
   float4 pos: POSITION;
};

struct VS_OUT
{
   float4 pos: POSITION;
};

VS_OUT main( VS_IN In )
{
   VS_OUT Out;
   Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos);
   return Out;
};
