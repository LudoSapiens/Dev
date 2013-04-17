sampler tex;
float2 texelSize;

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 color1;
   float4 color2;
   color1  = tex2D(tex, In.tex.xy + float2( -texelSize.x, -texelSize.y )*1.5);
   color1 -= tex2D(tex, In.tex.xy + float2(  texelSize.x,  texelSize.y )*1.5);
   color2  = tex2D(tex, In.tex.xy + float2(  texelSize.x, -texelSize.y )*1.5);
   color2 -= tex2D(tex, In.tex.xy + float2( -texelSize.x,  texelSize.y )*1.5);
   float4 color = (abs(color1)+abs(color2))*0.5;
   return color * float4(0.2*4.0, 0.2*4.0, 2.2*4.0, 1.0);
}
