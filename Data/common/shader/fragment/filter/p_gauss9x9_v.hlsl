sampler tex;
float2 disp;

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 Out;
   Out  = 0.074876 * tex2D(tex, In.tex.xy + float2(0.0, -disp.y));
   Out += 0.313037 * tex2D(tex, In.tex.xy + float2(0.0, -disp.x));
   Out += 0.224174 * tex2D(tex, In.tex.xy);
   Out += 0.313037 * tex2D(tex, In.tex.xy + float2(0.0, disp.x));
   Out += 0.074876 * tex2D(tex, In.tex.xy + float2(0.0, disp.y));
   return Out;
}
