//
// HLSL vertex shader used for simple distortion.
//


struct VS_IN
{
   float4 pos:  POSITION;
   float3 norm: NORMAL;
};

struct VS_OUT
{
   float4 pos:  POSITION;
   float4 cpos: TEXCOORD0;
   float3 norm: TEXCOORD1;
   float3 spos: TEXCOORD2;
};

VS_OUT main( VS_IN IN )
{
   VS_OUT OUT;

   OUT.pos     = mul( gfxWorldViewProjectionMatrix, IN.pos );
   OUT.cpos    = mul( gfxWorldViewMatrix, IN.pos );
   OUT.norm    = mul( gfxWorldViewMatrix, IN.norm );
   OUT.spos.xy = OUT.pos.xy * 0.5 + 0.5 * OUT.pos.w;
   OUT.spos.z  = OUT.pos.w;
   return OUT;
}

