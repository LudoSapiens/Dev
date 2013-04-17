//
// Depth passthrough HLSL pixel shader.
//

struct PS_INPUT
{
   float4 pos:  TEXCOORD0;
};

float4 main( PS_INPUT IN ) : COLOR
{
   float4 OUT;
   OUT = IN.pos.zzzz;
   return OUT;
}
