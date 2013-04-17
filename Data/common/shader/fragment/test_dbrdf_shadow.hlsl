//
// Distribution-based BRDF
//
// Notes:
//  * All single-character vectors are normalized
//

float4 color;
sampler colorTex;

struct PS_INPUT
{
   float2 map : TEXCOORD0;
   float4 pos:  TEXCOORD1;
   float3 norm: TEXCOORD2;
   float3 tan : TEXCOORD3;
};

float3 velvet( float theta )
{
   float c = 0.25;
   float cotan_theta = cos(theta)/sin(theta);
   float v = c * (1.0 + 4.0*exp(-(cotan_theta*cotan_theta)));
   return float3(v, v, v);
}

float3 anisoPhong( float theta, float phi )
{
   float2 rough   = float2( 10.0, 100.0 );
   float cosPhi = cos( phi );
   float sinPhi = sin( phi );
   float ep     = rough.x * cosPhi*cosPhi + rough.y * sinPhi*sinPhi;
   float v = pow( cos(theta), ep ) + 0.2;
   return float3(v, v, v);
}

float4 main( PS_INPUT IN ) : COLOR
{
   float3  lightPos = float3( 10.0, 5.0, 5.0 );
   //lightPos = gfxCameraPosition;

   float3 i = normalize(lightPos-IN.pos); //aka k1
   float3 o = normalize(gfxCameraPosition-IN.pos); //aka k2

   float3 n = normalize(IN.norm);
   float3 t = normalize(IN.tan.xyz);
   float3 h = normalize(i + o);


   float n_dot_h = dot(n, h);
   float i_dot_n = max( 0.0, dot(i, n) );
   float o_dot_n = dot(o, n);
   
   float kh = min( 1.0, dot(i, h) );  //or dot(o, h)

   float3 h_tangent = normalize(h - n*(n_dot_h));

   float theta = acos(n_dot_h);
   float phi = acos(dot(t, h_tangent));

   //Convert theta in [0, Pi] and phi in [0, 2Pi] into
   //[0, 1] texture normalized texture coordinates
   //float2 brdfCoord = float2(theta, phi)*float2(ooPi, oo2Pi);


   //Actual dBRDF calculation
   
   // Velvet
   float3 ph = velvet(theta);
   
   // Lambert
   //float3 ph = float3(1.0);
   
   // Blinn
   float b = pow(cos(theta), 100.0) + 0.3;
   //float3 ph = { b, b, b };
   
   // Anisotropic
   //float3 ph = anisoPhong(theta, phi);
   
   //Schlick's polynomial approximation
   float r0   = 1.0;
   float fkh  = r0 + (1.0-r0)*pow((1.0-kh), 5.0);
   float att  = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n)); //denominator
   float4 dbrdf = float4( ph*fkh*att, 1.0 );

   float4 OUT = dbrdf * color;//*tex2D( colorTex, IN.map );
   return OUT;
}
