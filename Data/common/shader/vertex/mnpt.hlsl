//
// HLSL vertex shader.
// --------------------
//  m: mapping
//  n: normal
//  p: position
//  t: tangent
//

struct VS_IN
{
   float4 pos:  POSITION;
   float2 map:  TEXCOORD0;
   float3 norm: NORMAL;
   float4 tan : TANGENT;
};

struct VS_OUT
{
   float4 pos:  POSITION;
   float2 map:  TEXCOORD0;
   float4 wpos: TEXCOORD1;
   float3 norm: TEXCOORD2;
   float4 tan:  TEXCOORD3;
};

VS_OUT main( VS_IN IN )
{
   VS_OUT OUT;
   OUT.pos     = mul( gfxWorldViewProjectionMatrix, IN.pos );
   OUT.wpos    = mul( gfxWorldMatrix, IN.pos );
   OUT.map     = IN.map;
   OUT.norm    = mul( gfxWorldMatrix, IN.norm );
   OUT.tan.xyz = mul( gfxWorldMatrix, IN.tan );
   OUT.tan.w   = IN.tan.w;  //restore sign
   return OUT;
}
