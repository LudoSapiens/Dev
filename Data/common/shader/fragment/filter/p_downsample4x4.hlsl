sampler tex;
float2 texelSize;

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 Out;
   Out  = tex2D(tex, In.tex.xy + float2(-texelSize.x, -texelSize.y));
   Out += tex2D(tex, In.tex.xy + float2( texelSize.x, -texelSize.y));
   Out += tex2D(tex, In.tex.xy + float2(-texelSize.x,  texelSize.y));
   Out += tex2D(tex, In.tex.xy + float2( texelSize.x,  texelSize.y));
   Out *= (1.0 / 4.0);
   return Out;
}
