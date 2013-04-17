//
// colorTex HLSL fragment shader.
//
float4 color;
sampler colorTex;
struct PS_INPUT
{
   float2 map: TEXCOORD0;
};

float4 main( PS_INPUT IN ) : COLOR
{
   float4 OUT;
   OUT  = color * tex2D( colorTex, IN.map.xy );
   return OUT;
}
