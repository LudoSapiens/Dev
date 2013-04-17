sampler tex0;

struct PS_IN
{
   float4 col: COLOR0;
   float2 map: TEXCOORD0;
};

struct PS_OUT
{
   float4 col: COLOR;
};

PS_OUT main( PS_IN In )
{
   PS_OUT Out;
   Out.col = In.col * tex2D(tex0, In.map.xy);
   return Out;
}
