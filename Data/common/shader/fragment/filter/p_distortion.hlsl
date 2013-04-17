sampler tex;

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 Out;
   Out  = tex2D(tex, In.tex.rg);
   return Out;
}
