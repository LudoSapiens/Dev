//
// Texture HLSL fragment shader.
//

sampler colorTex;

struct PS_INPUT
{
   float2 map: TEXCOORD0;
};

struct PS_OUT
{
   float4 col: COLOR;
};

PS_OUT main( PS_INPUT In )
{
   PS_OUT Out;
   Out.col = tex2D( colorTex, In.map.xy );
   return Out;
}
