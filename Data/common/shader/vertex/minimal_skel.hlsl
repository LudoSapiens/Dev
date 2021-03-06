//
// Minimal HLSL vertex shader with skinning.
//
float4x4 boneMatrices[33]; // : WORLDMATRIXARRAY;

struct VS_IN
{
   float4 pos:      POSITION;
   float4 bones:    TEXCOORD2;
   float4 weights:  TEXCOORD1;
   float  numBones: TEXCOORD3;
};

struct VS_OUT
{
   float4 pos:  POSITION;
};

VS_OUT main( VS_IN In )
{
   VS_OUT Out;

   float weights[4] = (float[4])In.weights;
   int   bones[4]   = (int[4])In.bones;
   int   numBones   = (int)In.numBones;

   float4 p = float4( 0.0, 0.0, 0.0, 0.0 );

   for( int i = 0; i < numBones; ++i )
   {
      p += mul(boneMatrices[bones[i]], In.pos) * weights[i];
   }

   Out.pos = mul( gfxWorldViewProjectionMatrix, p );
   return Out;
};

