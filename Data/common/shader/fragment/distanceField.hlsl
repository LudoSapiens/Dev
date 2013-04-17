//
// Distance field HLSL fragment shader.
//

sampler colorTex;

float4 color;

struct PS_INPUT
{
   float2 map: TEXCOORD0;
};

float4 main( PS_INPUT In ) : COLOR
{
   float alpha = tex2D( colorTex, In.map.xy ).x;
   //if( alpha <= 0.35 || 0.50 <= alpha )
   //{
   //   clip(-1.0);
   //}
   clip(0.075 - abs(alpha-0.425));
   return color * smoothstep( 0.25, 0.30, alpha ) * 2.0;
}

