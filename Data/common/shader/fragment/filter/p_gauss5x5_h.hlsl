sampler tex;
float disp;

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 Out;
   Out  = (5.0/16.0) * tex2D(tex, In.tex.xy + float2(-disp, 0.0));
   Out += (6.0/16.0) * tex2D(tex, In.tex.xy);
   Out += (5.0/16.0) * tex2D(tex, In.tex.xy + float2( disp, 0.0));

   return Out;
}