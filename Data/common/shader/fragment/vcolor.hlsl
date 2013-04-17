//
// Vertex color HLSL fragment shader.
//

struct PS_IN
{
   float4 col: COLOR0;
};

struct PS_OUT
{
   float4 color: COLOR;
};

PS_OUT main( PS_IN In )
{
   PS_OUT Out;
   Out.color = In.col;
   return Out;
}
