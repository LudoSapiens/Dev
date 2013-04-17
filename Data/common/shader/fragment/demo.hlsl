//
// HLSL fragment shader for testing.
//

float4 color;
float4 clipPlane;
sampler colorTex;

struct PS_INPUT
{
   float2 map:  TEXCOORD0;
   float4 pos:  TEXCOORD1;
   float3 norm: TEXCOORD2;
   float4 tan:  TEXCOORD3;
};

float4 main( PS_INPUT IN ) : COLOR
{
   clip( dot(clipPlane, IN.pos) );
   float ao = length( IN.norm );
   return color*float4(ao,ao,ao,1.0)*tex2D( colorTex, (IN.map*0.125).yx );
}

