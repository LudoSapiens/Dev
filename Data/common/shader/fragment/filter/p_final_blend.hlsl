sampler baseTex;
sampler bloomTex1;
sampler bloomTex2;



struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   const float bloomFactor1 = 2.0;
   const float bloomFactor2 = 2.0;

   float4 Out = float4(0.0, 0.0, 0.0, 0.0);
   Out += tex2D(baseTex, In.tex.xy);
   Out += bloomFactor1 * tex2D(bloomTex1, In.tex.xy);
   Out += bloomFactor2 * tex2D(bloomTex2, In.tex.xy);
   // Tone mapping?
   return Out;
}
