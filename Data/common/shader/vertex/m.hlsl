//
// Mapping HLSL vertex shader.
//
struct VS_IN
{
   float4 pos: POSITION;
   float2 map: TEXCOORD0;
};
struct VS_OUT
{
   float4 pos: POSITION;
   float2 map: TEXCOORD0;
};

VS_OUT main( VS_IN IN )
{
   VS_OUT OUT;
   OUT.map = IN.map;
   OUT.pos = mul( gfxWorldViewProjectionMatrix, IN.pos );
   return OUT;
}
