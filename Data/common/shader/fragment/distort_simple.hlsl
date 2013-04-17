

sampler tex;
float2  texelSize;
float   eta;

struct PS_INPUT
{
   float4 cpos: TEXCOORD0;
   float3 norm: TEXCOORD1;
   float3 spos: TEXCOORD2;
};

float4 main( PS_INPUT IN ) : COLOR
{
   float3 i = normalize( gfxCameraPosition - IN.cpos );
   float3 n = normalize( IN.norm );
   float3 r = refract( i, n, eta );

   float2 d = r.xy * 0.01;

   float2 coord = (IN.spos.xy/IN.spos.z) + d;

   return tex2D(tex, coord);
}
