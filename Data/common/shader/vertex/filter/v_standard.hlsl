struct VS_IN
{
   float4 pos: POSITION;
   float2 tex: TEXCOORD0;
};

struct VS_OUT
{
   float4 pos: POSITION;
   float2 tex: TEXCOORD0;
};

VS_OUT main( VS_IN In )
{
   VS_OUT Out;
   Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos );
   Out.tex = In.pos * 0.5 + 0.5;
   return Out;
}
