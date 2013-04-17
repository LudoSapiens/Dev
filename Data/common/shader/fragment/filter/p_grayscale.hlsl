sampler tex;

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 Out;
   Out  = tex2D(tex, In.tex.rg);
   float lum = Out.r*0.30f + Out.g*0.59f + Out.b*0.11f;
   Out = float4(lum, lum, lum, 1.0f);
   return Out;
}
