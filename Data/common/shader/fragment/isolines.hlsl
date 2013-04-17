//
// isolines HLSL fragment shader.
//
sampler colorTex;
float4 frontColor;
float4 backColor;

struct PS_INPUT
{
   float2 map: TEXCOORD0;
   float3 norm: NORMAL;
};

float4 main( PS_INPUT IN ) : COLOR
{
   // Compute lighting.
   float3 lightPos = { 1.0, 5.0, 20.0 };
   float  nl       = max( dot( normalize( IN.norm ), normalize( lightPos ) ), 0.0 );
   float4 lighting = lerp( backColor, frontColor, nl );
   
   // Default values.
   float2 pos;
   float2 uv      = IN.map;
   float4 color   = { 1.0, 1.0, 1.0, 1.0 };
   float4 linecol = { 0.0, 0.0, 0.0, 1.0 };
   
   // Derivative.
   float2 d = max( abs( ddx( IN.map ) ), abs( ddy( IN.map ) ) ) * 0.5;
   
   // Lines color.
   for( int i = 0; i < 3; i++ )
   {
      pos   = float2( 0.5, 0.5 ) - abs( float2( 0.5, 0.5 ) - frac( uv ) );
      pos   = smoothstep( d, 4.0*d, pos );
      color = min( color, lerp( linecol, float4( 1.0, 1.0, 1.0, 1.0 ), pos.x*pos.y ) );
      
      d  = d * 1.8;
      uv = uv * 2.0;
      linecol = lerp( linecol, float4( 1.0, 1.0, 1.0, 1.0 ), 0.8 );
   }
   
   // Return final color.
   float4 OUT;
   OUT  = color * lighting * tex2D( colorTex, IN.map.xy/2.0 );
   return OUT;
}
