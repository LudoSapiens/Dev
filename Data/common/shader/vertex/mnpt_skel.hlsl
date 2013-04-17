//
// HLSL vertex shader with skinning.
// --------------------
//  m: mapping
//  n: normal
//  p: position
//  t: tangent
//

float4x4 boneMatrices[33]; // : WORLDMATRIXARRAY;

struct VS_IN
{
   float4 pos:      POSITION;
   float3 norm:     NORMAL;
   float4 tan:      TANGENT;
   float2 map:      TEXCOORD0;
   float4 bones:    TEXCOORD2;
   float4 weights:  TEXCOORD1;
   float  numBones: TEXCOORD3;
};

struct VS_OUT
{
   float4 pos:  POSITION;
   float2 map:  TEXCOORD0;
   float4 wpos: TEXCOORD1;
   float3 norm: TEXCOORD2;
   float4 tan:  TEXCOORD3;
};

VS_OUT main( VS_IN In )
{
   VS_OUT Out;

   float weights[4] = (float[4])In.weights;
   int   bones[4]   = (int[4])In.bones;
   int   numBones   = (int)In.numBones;

   float4 p = float4( 0.0, 0.0, 0.0, 0.0 );
   float3 n = float3( 0.0, 0.0, 0.0 );
   float3 t = float3( 0.0, 0.0, 0.0 );

   for( int i = 0; i < numBones; ++i )
   {
      p += mul(boneMatrices[bones[i]], In.pos) * weights[i];
      n += mul((float3x3)(boneMatrices[bones[i]]), In.norm) * weights[i];
      t += mul((float3x3)(boneMatrices[bones[i]]), In.tan.xyz) * weights[i];
   }

   Out.map     = In.map;
   Out.norm    = mul( (float3x3)(gfxWorldMatrix), n );
   Out.tan.xyz = mul( (float3x3)(gfxWorldMatrix), t );
   Out.tan.w   = In.tan.w; // Restore sign.
   Out.wpos    = mul( gfxWorldMatrix, p );
   Out.pos     = mul( gfxWorldViewProjectionMatrix, p );
   return Out;
};

