//
// VSM HLSL pixel shader.
//

struct PS_INPUT
{
   float4 pos:  TEXCOORD0;
};

float4 main( PS_INPUT IN ) : COLOR
{
#if 0
   float4 OUT;
   float depth    = length( IN.pos )*0.01;
   float2 moments = float2( depth, depth*depth );
   OUT.zw         = frac( moments * 32.0 );
   OUT.xy         = moments-(OUT.zw/32.0);
   return OUT;
#endif   
   float4 OUT;
   float depth   = length( IN.pos )*0.01;
   OUT           = float4( depth, depth*depth, 0.0, 0.0 );
   return OUT;
}
