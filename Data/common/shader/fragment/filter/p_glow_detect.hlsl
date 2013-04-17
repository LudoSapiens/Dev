sampler tex;

float4 remap( float4 color )
{
   return max( float4(0.0,0.0,0.0,0.0), color-float4(0.9, 0.9, 0.9, 0.0) )*1.3;
}

struct PS_INPUT
{
   float2 tex: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float4 Out;
   Out  = remap( tex2D(tex, In.tex.xy) );
   return Out;
}

