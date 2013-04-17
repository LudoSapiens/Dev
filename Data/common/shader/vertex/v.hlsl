//
// View position HLSL vertex shader.
//

struct VS_IN
{
   float4 pos:  POSITION;
};

struct VS_OUT
{
   float4 pos:  POSITION;
   float4 vpos: TEXCOORD0;
};

VS_OUT main( VS_IN IN )
{
   VS_OUT OUT;
   OUT.vpos    = mul( gfxWorldViewMatrix, IN.pos );
   OUT.pos     = mul( gfxWorldViewProjectionMatrix, IN.pos );
   return OUT;
}
