/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/ADT/Vector.h>
#include <Base/MT/Thread.h>

#include <Gfx/Prog/Constants.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Mgr/Context.h>
#include <Gfx/Mgr/Manager.h>
#include <Gfx/Pass/Pass.h>
#include <Gfx/FB/RenderState.h>
#include <Gfx/Tex/Sampler.h>
#include <Gfx/Tex/Texture.h>

//Include window-specific routines for tests
#include <Gfx/src/main.h>

#include <cassert>

USING_NAMESPACE


#define checkIf( test, pass_count, fail_count ) \
   if( test ) \
   { \
      ++pass_count; \
   } \
   else \
   { \
      printf("FAILED: '" #test "' on line %d\n", __LINE__); \
      ++fail_count; \
   }


/*==============================================================================
   TEXTURES
==============================================================================*/
#define T_RED  (uchar)0xFF, (uchar)0x00, (uchar)0x00, (uchar)0xFF
#define T_GRN  (uchar)0x00, (uchar)0xFF, (uchar)0x00, (uchar)0xFF
#define T_BLU  (uchar)0x00, (uchar)0x00, (uchar)0xFF, (uchar)0xFF
#define T_CYA  (uchar)0x00, (uchar)0xFF, (uchar)0xFF, (uchar)0xFF
#define T_MAG  (uchar)0xFF, (uchar)0x00, (uchar)0xFF, (uchar)0xFF
#define T_YEL  (uchar)0xFF, (uchar)0xFF, (uchar)0x00, (uchar)0xFF
#define T_BLK  (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0xFF
#define T_DGR  (uchar)0x7F, (uchar)0x7F, (uchar)0x7F, (uchar)0xFF
#define T_GRY  (uchar)0x80, (uchar)0x80, (uchar)0x80, (uchar)0xFF
#define T_LGR  (uchar)0x80, (uchar)0x80, (uchar)0x80, (uchar)0xFF
#define T_WHT  (uchar)0xFF, (uchar)0xFF, (uchar)0xFF, (uchar)0xFF

uchar tex_rgba_4x4[] =
{
	T_RED, T_MAG, T_GRN, T_GRN,
	T_RED, T_DGR, T_LGR, T_GRN,
	T_BLU, T_DGR, T_LGR, T_WHT,
	T_BLU, T_BLU, T_WHT, T_WHT,
};

/*==============================================================================
   SHADER PROGRAMS
==============================================================================*/

//Global shader strings
String vs_str_XYZ_RGBA;
String ps_str_XYZ_RGBA;
String ps_str_XYZ_RGBA_scale;
String ps_str_XYZ_RGBA_grayscale;
String vs_str_XYZ_ST;
String vs_str_XYZ_ST2;
String ps_str_XYZ_ST;
String ps_str_XYZ_ST_scale;
String ps_str_XYZ_ST_grayscale;
String vs_str_cubemap;
String ps_str_cubemap;


void initGLSLShaderStrings()
{
//GLSL
vs_str_XYZ_RGBA =
"   void main( void )\n"
"   {\n"
"      gl_FrontColor = gl_Color;\n"
"      gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;\n"
"   }\n";

ps_str_XYZ_RGBA =
"   void main( void )\n"
"   {\n"
"      gl_FragColor = gl_Color;\n"
"   }\n";

ps_str_XYZ_RGBA_scale =
"   uniform float scaleFactor;\n"
"   void main( void )\n"
"   {\n"
"      gl_FragColor = vec4(gl_Color.xyz * scaleFactor, gl_Color.w);\n"
"   }\n";

ps_str_XYZ_RGBA_grayscale =
"   const vec3 grayScaleWeights = vec3(0.30, 0.59, 0.11);\n"
"   \n"
"   void main( void )\n"
"   {\n"
"      //Convert incoming color as grayscale using R*0.30 + G*0.59 + B*0.11\n"
"      float luminance = dot(gl_Color.xyz, grayScaleWeights);\n"
"      gl_FragColor = vec4(luminance, luminance, luminance, 1.0);\n"
"   }\n";

vs_str_XYZ_ST =
"   void main( void )\n"
"   {\n"
"      gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"      gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;\n"
"   }\n";

vs_str_XYZ_ST2 =
"   void main( void )\n"
"   {\n"
"      gl_TexCoord[0] = gl_Vertex*0.5 + 0.5;\n"
"      gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;\n"
"   }\n";


ps_str_XYZ_ST =
"   uniform sampler2D tex;\n"
"   void main( void )\n"
"   {\n"
"      vec4 color = texture2D(tex, gl_TexCoord[0].xy);\n"
"      gl_FragColor = color;\n"
"   }\n";

ps_str_XYZ_ST_scale =
"   uniform sampler2D tex;\n"
"   uniform float scaleFactor;\n"
"   void main( void )\n"
"   {\n"
"      vec4 color = texture2D(tex, gl_TexCoord[0].xy);\n"
"      gl_FragColor = vec4(color.xyz*scaleFactor, color.w);\n"
"   }\n";

ps_str_XYZ_ST_grayscale =
"   uniform sampler2D tex;\n"
"   const vec3 grayScaleWeights = vec3(0.30, 0.59, 0.11);\n"
"   \n"
"   void main( void )\n"
"   {\n"
"      vec4 color = texture2D(tex, gl_TexCoord[0].xy);\n"
"      //Convert incoming color as grayscale using R*0.30 + G*0.59 + B*0.11\n"
"      float luminance = dot(color.xyz, grayScaleWeights);\n"
"      gl_FragColor = vec4(luminance, luminance, luminance, 1.0);\n"
"   }\n";

vs_str_cubemap =
"   void main( void )\n"
"   {\n"
"      gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"      gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;\n"
"   }\n";

ps_str_cubemap =
"   uniform samplerCube tex;\n"
"   void main( void )\n"
"   {\n"
"      vec4 color = texture(tex, gl_TexCoord[0].xyz);\n"
"      gl_FragColor = color;\n"
"   }\n";
}

void initHLSLShaderStrings()
{
//HLSL shaders

vs_str_XYZ_RGBA =
"   struct VS_IN\n"
"   {\n"
"      float4 pos: POSITION;\n"
"      float4 col: COLOR;\n"
"   };\n"
"   struct VS_OUT\n"
"   {\n"
"      float4 pos: POSITION;\n"
"      float4 col: COLOR;\n"
"   };\n"
"   VS_OUT main( VS_IN In )\n"
"   {\n"
"      VS_OUT Out;\n"
"      Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos);\n"
"      Out.col = In.col;\n"
"      return Out;\n"
"   }\n";

ps_str_XYZ_RGBA =
"   struct PS_INPUT\n"
"   {\n"
"      float4 color: COLOR0;\n"
"   };\n"
"   float4 main( PS_INPUT In ) : COLOR\n"
"   {\n"
"      float4 Out;\n"
"      Out  = In.color;\n"
"      return Out;\n"
"   }\n";

ps_str_XYZ_RGBA_scale =
"   float scaleFactor;\n"
"   struct PS_INPUT\n"
"   {\n"
"      float4 color: COLOR0;\n"
"   };\n"
"   float4 main( PS_INPUT In ) : COLOR\n"
"   {\n"
"      float4 Out;\n"
"      Out.xyz = In.color.xyz * scaleFactor;\n"
"      Out.w = In.color.w;\n"
"      return Out;\n"
"   }\n";

ps_str_XYZ_RGBA_grayscale =
"   struct PS_INPUT\n"
"   {\n"
"      float4 color: COLOR0;\n"
"   };\n"
"   float4 main( PS_INPUT In ) : COLOR\n"
"   {\n"
"      float4 Out;\n"
"      float lum = In.color.r*0.30f + In.color.g*0.59f + In.color.b*0.11f;\n"
"      Out  = float4(lum, lum, lum, 1.0f);\n"
"      return Out;\n"
"   }\n";

vs_str_XYZ_ST =
"   struct VS_IN\n"
"   {\n"
"      float4 pos: POSITION;\n"
"      float2 tex: TEXCOORD0;\n"
"   };\n"
"   struct VS_OUT\n"
"   {\n"
"      float4 pos: POSITION;\n"
"      float2 tex: TEXCOORD0;\n"
"   };\n"
"   VS_OUT main( VS_IN In )\n"
"   {\n"
"      VS_OUT Out;\n"
"      Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos);\n"
"      Out.tex = In.tex;\n"
"      return Out;\n"
"   }\n";

vs_str_XYZ_ST2 =
"   struct VS_IN\n"
"   {\n"
"      float4 pos: POSITION;\n"
"   };\n"
"   struct VS_OUT\n"
"   {\n"
"      float4 pos: POSITION;\n"
"      float2 tex: TEXCOORD0;\n"
"   };\n"
"   VS_OUT main( VS_IN In )\n"
"   {\n"
"      VS_OUT Out;\n"
"      Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos);\n"
"      Out.tex = In.pos*0.5 + 0.5;\n"
"      return Out;\n"
"   }\n";


//Note: sampler_state filter settings don't seem to work
ps_str_XYZ_ST =
"   sampler tex;\n"
"   struct PS_INPUT\n"
"   {\n"
"      float2 tex: TEXCOORD0;\n"
"   };\n"
"   float4 main( PS_INPUT In ) : COLOR\n"
"   {\n"
"      float4 Out;\n"
"      Out  = tex2D(tex, In.tex.rg);\n"
"      return Out;\n"
"   }\n";

ps_str_XYZ_ST_scale =
"   float scaleFactor;\n"
"   sampler tex;\n"
"   struct PS_INPUT\n"
"   {\n"
"      float2 tex: TEXCOORD0;\n"
"   };\n"
"   float4 main( PS_INPUT In ) : COLOR\n"
"   {\n"
"      float4 Out;\n"
"      Out = tex2D(tex, In.tex.rg);\n"
"      Out.xyz = Out.xyz*scaleFactor;\n"
"      return Out;\n"
"   }\n";

//Note: sampler_state filter settings don't seem to work
ps_str_XYZ_ST_grayscale =
"   sampler tex;\n"
"   struct PS_INPUT\n"
"   {\n"
"      float2 tex: TEXCOORD0;\n"
"   };\n"
"   float4 main( PS_INPUT In ) : COLOR\n"
"   {\n"
"      float4 Out;\n"
"      Out = tex2D(tex, In.tex.rg);\n"
"      float lum = Out.r*0.30f + Out.g*0.59f + Out.b*0.11f;\n"
"      Out = float4(lum, lum, lum, 1.0f);\n"
"      return Out;\n"
"   }\n";

}

void initShaderStrings( const String& api_str )
{
   printf("API: %s\n", api_str.cstr());
   if( api_str.empty() )
   {
#if API_TEST != 4
      initGLSLShaderStrings();
#else
      initHLSLShaderStrings();
#endif
   }
   else if( tolower(api_str[0]) == 'o' )
   {
      initGLSLShaderStrings();
   }
   else if( tolower(api_str[0]) == 'd' )
   {
      initHLSLShaderStrings();
   }
   else
   {
      //Unknown API
      assert(0);
   }
}

/*==============================================================================
   GLOBAL FUNCTIONS
==============================================================================*/


/*==============================================================================
   Scene creation routines
==============================================================================*/

//------------------------------------------------------------------------------
//! Creates 2 intersecting triangles, with interpolated colors
void  createTwoTriangles( RCP<Gfx::Manager>& mgr, RCP<Gfx::Geometry>& dst )
{
   //Geometry
   Vector<float> vertices;
   Vector<uint>  indices;

   //First triangle
   vertices.pushBack(-1.0f); //X
   vertices.pushBack(-1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 1.0f); //R
   vertices.pushBack( 0.0f); //G
   vertices.pushBack( 0.0f); //B
   vertices.pushBack( 1.0f); //A

   vertices.pushBack( 1.0f); //X
   vertices.pushBack(-1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 0.0f); //R
   vertices.pushBack( 1.0f); //G
   vertices.pushBack( 0.0f); //B
   vertices.pushBack( 1.0f); //A

   vertices.pushBack(-1.0f); //X
   vertices.pushBack( 1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 0.0f); //R
   vertices.pushBack( 0.0f); //G
   vertices.pushBack( 1.0f); //B
   vertices.pushBack( 1.0f); //A

   //vertices.clear();

   //Second triangle
   vertices.pushBack(-1.0f); //X
   vertices.pushBack(-1.0f); //Y
   vertices.pushBack( 0.4f); //Z
   vertices.pushBack( 1.0f); //R
   vertices.pushBack( 1.0f); //G
   vertices.pushBack( 0.0f); //B
   vertices.pushBack( 1.0f); //A

   vertices.pushBack( 1.0f); //X
   vertices.pushBack(-1.0f); //Y
   vertices.pushBack( 0.6f); //Z
   vertices.pushBack( 0.0f); //R
   vertices.pushBack( 1.0f); //G
   vertices.pushBack( 1.0f); //B
   vertices.pushBack( 1.0f); //A

   vertices.pushBack( 1.0f); //X
   vertices.pushBack( 1.0f); //Y
   vertices.pushBack( 0.6f); //Z
   vertices.pushBack( 1.0f); //R
   vertices.pushBack( 0.0f); //G
   vertices.pushBack( 1.0f); //B
   vertices.pushBack( 1.0f); //A

   indices.pushBack(0);
   indices.pushBack(1);
   indices.pushBack(2);
   indices.pushBack(3);
   indices.pushBack(4);
   indices.pushBack(5);

   RCP<Gfx::Geometry> geom( mgr->createGeometry( Gfx::PRIM_TRIANGLES ) );

   //Index buffer
   RCP<Gfx::IndexBuffer> indexBuffer(
      mgr->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, indices.dataSize(), indices.data() )
   );
   geom->indexBuffer( indexBuffer );

   //Vertex buffer
#if 1
   RCP<Gfx::VertexBuffer> vertexBuffer(
      mgr->createBuffer( Gfx::BUFFER_FLAGS_NONE, vertices.dataSize(), vertices.data() )
   );
#else
   RCP<Gfx::VertexBuffer> vertexBuffer(
      mgr->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, NULL )
   );
   mgr->setData(vertexBuffer, vertices.dataSize(), vertices.data());
#endif
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, vertexBuffer->strideInBytes() );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR, Gfx::ATTRIB_FMT_32F_32F_32F_32F, vertexBuffer->strideInBytes() );
   geom->addBuffer( vertexBuffer );

   dst = geom;
}

//------------------------------------------------------------------------------
//! Creates a simple texture quad covering almost all of the screen
void  createTexturedQuad( RCP<Gfx::Manager>& mgr, RCP<Gfx::Geometry>& dst )
{
   //Geometry
   Vector<float> vertices;
   Vector<uint>  indices;

   //Triangle fan, CCW
	//  D--C
	//  | /|
	//  |/ |
	//  A--B
   vertices.pushBack(-0.8f); //X
   vertices.pushBack(-0.8f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack(-1.0f); //S
   vertices.pushBack(-1.0f); //T

   vertices.pushBack( 0.8f); //X
   vertices.pushBack(-0.8f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 2.0f); //S
   vertices.pushBack(-1.0f); //T

   vertices.pushBack( 0.8f); //X
   vertices.pushBack( 0.8f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 2.0f); //S
   vertices.pushBack( 2.0f); //T

   vertices.pushBack(-0.8f); //X
   vertices.pushBack( 0.8f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack(-1.0f); //S
   vertices.pushBack( 2.0f); //T

   indices.pushBack(0);
   indices.pushBack(1);
   indices.pushBack(2);
   indices.pushBack(3);

   RCP<Gfx::Geometry> geom( mgr->createGeometry( Gfx::PRIM_TRIANGLE_FAN ) );

   //Index buffer
   RCP<Gfx::IndexBuffer> indexBuffer(
      mgr->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, indices.dataSize(), indices.data() )
   );
   geom->indexBuffer( indexBuffer );

   //Vertex buffer
   RCP<Gfx::VertexBuffer> vertexBuffer(
      mgr->createBuffer( Gfx::BUFFER_FLAGS_NONE, vertices.dataSize(), vertices.data() )
   );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, vertexBuffer->strideInBytes() );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, vertexBuffer->strideInBytes() );
   geom->addBuffer( vertexBuffer );

   dst = geom;
}

//------------------------------------------------------------------------------
//! Creates a simple texture quad of one
void  createTexturedUnitQuad( RCP<Gfx::Manager>& mgr, RCP<Gfx::Geometry>& dst )
{
   //Geometry
   Vector<float> vertices;
   Vector<uint>  indices;

   //Triangle fan, CCW
	//  D--C
	//  | /|
	//  |/ |
	//  A--B
   vertices.pushBack( 0.0f); //X
   vertices.pushBack( 0.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 0.0f); //S
   vertices.pushBack( 0.0f); //T

   vertices.pushBack( 1.0f); //X
   vertices.pushBack( 0.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 1.0f); //S
   vertices.pushBack( 0.0f); //T

   vertices.pushBack( 1.0f); //X
   vertices.pushBack( 1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 1.0f); //S
   vertices.pushBack( 1.0f); //T

   vertices.pushBack( 0.0f); //X
   vertices.pushBack( 1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 0.0f); //S
   vertices.pushBack( 1.0f); //T

   indices.pushBack(0);
   indices.pushBack(1);
   indices.pushBack(2);
   indices.pushBack(3);

   RCP<Gfx::Geometry> geom( mgr->createGeometry( Gfx::PRIM_TRIANGLE_FAN ) );

   //Index buffer
   RCP<Gfx::IndexBuffer> indexBuffer(
      mgr->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, indices.dataSize(), indices.data() )
   );
   geom->indexBuffer( indexBuffer );

   //Vertex buffer
   RCP<Gfx::VertexBuffer> vertexBuffer(
      mgr->createBuffer( Gfx::BUFFER_FLAGS_NONE, vertices.dataSize(), vertices.data() )
   );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, vertexBuffer->strideInBytes() );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, vertexBuffer->strideInBytes() );
   geom->addBuffer( vertexBuffer );

   dst = geom;
}

//------------------------------------------------------------------------------
//! Creates a simple texture quad, rotated a little
void  createTexturedQuadRot( RCP<Gfx::Manager>& mgr, RCP<Gfx::Geometry>& dst )
{
   //Geometry
   Vector<float> vertices;
   Vector<uint>  indices;

   //Triangle fan, CCW
   //  +-C-...+
   //  |/     B
   //  /      /
   //  D...  /|
   //  +----A-+
   vertices.pushBack( 0.8f); //X
   vertices.pushBack(-1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 0.0f); //S
   vertices.pushBack( 0.0f); //T

   vertices.pushBack( 1.0f); //X
   vertices.pushBack( 0.8f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 1.0f); //S
   vertices.pushBack( 0.0f); //T

   vertices.pushBack(-0.8f); //X
   vertices.pushBack( 1.0f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 1.0f); //S
   vertices.pushBack( 1.0f); //T

   vertices.pushBack(-1.0f); //X
   vertices.pushBack(-0.8f); //Y
   vertices.pushBack( 0.5f); //Z
   vertices.pushBack( 0.0f); //S
   vertices.pushBack( 1.0f); //T

   indices.pushBack(0);
   indices.pushBack(1);
   indices.pushBack(2);
   indices.pushBack(3);

   RCP<Gfx::Geometry> geom( mgr->createGeometry( Gfx::PRIM_TRIANGLE_FAN ) );

   //Index buffer
   RCP<Gfx::IndexBuffer> indexBuffer(
      mgr->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, indices.dataSize(), indices.data() )
   );
   geom->indexBuffer( indexBuffer );

   //Vertex buffer
   RCP<Gfx::VertexBuffer> vertexBuffer(
      mgr->createBuffer( Gfx::BUFFER_FLAGS_NONE, vertices.dataSize(), vertices.data() )
   );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, vertexBuffer->strideInBytes() );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, vertexBuffer->strideInBytes() );
   geom->addBuffer( vertexBuffer );

   dst = geom;
}

//------------------------------------------------------------------------------
//! A cross to show all of the faces of a cubemap.
void  createCubemapQuads( RCP<Gfx::Manager>& mgr, RCP<Gfx::Geometry>& dst )
{
   //
   //     +--+     -- 0.80
   //     |-z|
   //     +--+     -- 0.65
   //     |+y|
   //  +--+--+--+  -- 0.50
   //  |-x|+z|+x|
   //  +--+--+--+  -- 0.35
   //     |-y|
   //     +--+     -- 0.20
   //
   //  |  |  |  |
   // 0.2 | 0.6 |
   //    0.4   0.8
   //
   // Drawing order: -X +X -Y +Y -Z +Z.
   // Each quad is a 2-triangle strip.

   float vertices[] = {
      // X      Y     Z      U     V     W
      0.2f, 0.35f, 0.5f, -1.0f,-1.0f,-1.0f,
      0.4f, 0.35f, 0.5f, -1.0f,-1.0f, 1.0f,
      0.2f, 0.50f, 0.5f, -1.0f, 1.0f,-1.0f,
      0.4f, 0.50f, 0.5f, -1.0f, 1.0f, 1.0f,

      0.6f, 0.35f, 0.5f,  1.0f,-1.0f, 1.0f,
      0.8f, 0.35f, 0.5f,  1.0f,-1.0f,-1.0f,
      0.6f, 0.50f, 0.5f,  1.0f, 1.0f, 1.0f,
      0.8f, 0.50f, 0.5f,  1.0f, 1.0f,-1.0f,

      0.4f, 0.20f, 0.5f, -1.0f,-1.0f,-1.0f,
      0.6f, 0.20f, 0.5f,  1.0f,-1.0f,-1.0f,
      0.4f, 0.35f, 0.5f, -1.0f,-1.0f, 1.0f,
      0.6f, 0.35f, 0.5f,  1.0f,-1.0f, 1.0f,

      0.4f, 0.50f, 0.5f, -1.0f, 1.0f, 1.0f,
      0.6f, 0.50f, 0.5f,  1.0f, 1.0f, 1.0f,
      0.4f, 0.65f, 0.5f, -1.0f, 1.0f,-1.0f,
      0.6f, 0.65f, 0.5f,  1.0f, 1.0f,-1.0f,

      0.4f, 0.65f, 0.5f, -1.0f, 1.0f,-1.0f,
      0.6f, 0.65f, 0.5f,  1.0f, 1.0f,-1.0f,
      0.4f, 0.80f, 0.5f, -1.0f,-1.0f,-1.0f,
      0.6f, 0.80f, 0.5f,  1.0f,-1.0f,-1.0f,

      0.4f, 0.35f, 0.5f, -1.0f,-1.0f, 1.0f,
      0.6f, 0.35f, 0.5f,  1.0f,-1.0f, 1.0f,
      0.4f, 0.50f, 0.5f, -1.0f, 1.0f, 1.0f,
      0.6f, 0.50f, 0.5f,  1.0f, 1.0f, 1.0f,
   };

   // Set XY range from [0, 1] to [-1, 1].
   for( uint i = 0; i < 6*4; ++i )
   {
      float* v = vertices + 6*i;
      v[0] = v[0]*2.0f - 1.0f;
      v[1] = v[1]*2.0f - 1.0f;
      v[2] = v[2]*2.0f - 1.0f;
   }

   uint indices[6*6]; // 6 faces, 2 triangles each.
   for( uint i = 0; i < 6; ++i )
   {
      uint baseI = i*6;
      uint baseV = i*4;
      indices[baseI+0] = baseV+0;
      indices[baseI+1] = baseV+1;
      indices[baseI+2] = baseV+3;
      indices[baseI+3] = baseV+0;
      indices[baseI+4] = baseV+3;
      indices[baseI+5] = baseV+2;
   }

   RCP<Gfx::Geometry> geom( mgr->createGeometry( Gfx::PRIM_TRIANGLES ) );

   //Index buffer
   RCP<Gfx::IndexBuffer> indexBuffer(
      mgr->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, sizeof(indices), indices )
   );
   geom->indexBuffer( indexBuffer );

   //Vertex buffer
   RCP<Gfx::VertexBuffer> vertexBuffer(
      mgr->createBuffer( Gfx::BUFFER_FLAGS_NONE, sizeof(vertices), vertices )
   );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, vertexBuffer->strideInBytes() );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F_32F, vertexBuffer->strideInBytes() );
   geom->addBuffer( vertexBuffer );

   dst = geom;
}

/*==============================================================================
   Texture routines
==============================================================================*/
void  setTexture_RGBA_4x4( RCP<Gfx::Manager>& mgr, RCP<Gfx::Texture>& tex )
{
   tex = mgr->create2DTexture(
      4, 4,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_NONE
   );
   mgr->setData(tex, 0, tex_rgba_4x4);
}

void  setTexture_Lines( RCP<Gfx::Manager>& mgr, RCP<Gfx::Texture>& tex, uint width, uint height )
{
   tex = mgr->create2DTexture(
      width, height,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_MIPMAPPED
   );
   uchar* data = new uchar[ width * height * 4 ];
   uchar* cur = data;
   for( uint y = 0; y < height; ++y )
   {
      uchar val = (y * 0xFF) / height;
      for( uint x = 0; x < width; ++x )
      {
         cur[0] = ((x%3)==0)?(val):(0x00);
         cur[1] = cur[0];
         cur[2] = cur[0]; //x * 0xFF / width;
         cur[3] = 0xFF;
         cur += 4;
      }
   }
   mgr->setData(tex, 0, data);
   delete[] data;
}
void  setTexture_cubemap( RCP<Gfx::Manager>& mgr, RCP<Gfx::Texture>& tex )
{
   tex = mgr->createCubeTexture(
      4,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_NONE
   );

   uchar tex_neg_x[] =
   {
      T_RED, T_RED, T_RED, T_RED,
      T_RED, T_RED, T_WHT, T_RED,
      T_RED, T_RED, T_RED, T_RED,
      T_RED, T_RED, T_RED, T_RED,
   };
   uchar tex_neg_y[] =
   {
      T_GRN, T_GRN, T_GRN, T_GRN,
      T_GRN, T_GRN, T_WHT, T_GRN,
      T_GRN, T_GRN, T_GRN, T_GRN,
      T_GRN, T_GRN, T_GRN, T_GRN,
   };
   uchar tex_neg_z[] =
   {
      T_BLU, T_BLU, T_BLU, T_BLU,
      T_BLU, T_BLU, T_WHT, T_BLU,
      T_BLU, T_BLU, T_BLU, T_BLU,
      T_BLU, T_BLU, T_BLU, T_BLU,
   };
   uchar tex_pos_x[] =
   {
      T_CYA, T_CYA, T_CYA, T_CYA,
      T_CYA, T_CYA, T_WHT, T_CYA,
      T_CYA, T_CYA, T_CYA, T_CYA,
      T_CYA, T_CYA, T_CYA, T_CYA,
   };
   uchar tex_pos_y[] =
   {
      T_MAG, T_MAG, T_MAG, T_MAG,
      T_MAG, T_MAG, T_WHT, T_MAG,
      T_MAG, T_MAG, T_MAG, T_MAG,
      T_MAG, T_MAG, T_MAG, T_MAG,
   };
   uchar tex_pos_z[] =
   {
      T_YEL, T_YEL, T_YEL, T_YEL,
      T_YEL, T_YEL, T_WHT, T_YEL,
      T_YEL, T_YEL, T_YEL, T_YEL,
      T_YEL, T_YEL, T_YEL, T_YEL,
   };
   mgr->setData(tex, 0, Gfx::TEX_SLICE_NEG_X, tex_neg_x);
   mgr->setData(tex, 0, Gfx::TEX_SLICE_POS_X, tex_pos_x);
   mgr->setData(tex, 0, Gfx::TEX_SLICE_NEG_Y, tex_neg_y);
   mgr->setData(tex, 0, Gfx::TEX_SLICE_POS_Y, tex_pos_y);
   mgr->setData(tex, 0, Gfx::TEX_SLICE_NEG_Z, tex_neg_z);
   mgr->setData(tex, 0, Gfx::TEX_SLICE_POS_Z, tex_pos_z);
}

void  setTextureState_Defaults( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.setDefaults();
}

void  setTextureState_ClampToLast( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.clampX(Gfx::TEX_CLAMP_LAST);
   texState.clampY(Gfx::TEX_CLAMP_LAST);
   //texState.clampZ(Gfx::TEX_CLAMP_LAST);
}

void  setTextureState_ClampToBorder( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.clampX(Gfx::TEX_CLAMP_BORDER);
   texState.clampY(Gfx::TEX_CLAMP_BORDER);
   //texState.clampZ(Gfx::TEX_CLAMP_BORDER);
}

void  setTextureState_Point( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.magFilter(Gfx::TEX_FILTER_POINT);
   texState.minFilter(Gfx::TEX_FILTER_POINT);
   texState.mipFilter(Gfx::TEX_FILTER_NONE);
}

void  setTextureState_Bilinear( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.magFilter(Gfx::TEX_FILTER_LINEAR);
   texState.minFilter(Gfx::TEX_FILTER_LINEAR);
   texState.mipFilter(Gfx::TEX_FILTER_NONE);
}

void  setTextureState_Trilinear( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.magFilter(Gfx::TEX_FILTER_LINEAR);
   texState.minFilter(Gfx::TEX_FILTER_LINEAR);
   texState.mipFilter(Gfx::TEX_FILTER_LINEAR);
}

void  setTextureState_Special1( RCP<Gfx::Manager>& /*mgr*/, Gfx::TextureState& texState )
{
   texState.magFilter(Gfx::TEX_FILTER_LINEAR);
   texState.clampX(Gfx::TEX_CLAMP_MIRRORONCE_LAST);
   texState.clampY(Gfx::TEX_CLAMP_LAST);
}

/*==============================================================================
   Test routines
==============================================================================*/

//------------------------------------------------------------------------------
//!
void simple( RCP<Gfx::Manager>& mgr )
{
   Gfx::Pass pass;
   pass.setClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

   pass.setViewport( 0, 0, 200, 200 );

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA_grayscale ) );

   pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );

   pass.execGeometry( geom );

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void simple_texture( RCP<Gfx::Manager>& mgr )
{
   Gfx::Pass pass;
   pass.setClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

   //Texture
   RCP<Gfx::Texture> texture;
   setTexture_RGBA_4x4(mgr, texture);
   Gfx::TextureState texState;
   setTextureState_Bilinear(mgr, texState);
   RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
   sl->addSampler( "tex", texture, texState );
   pass.setSamplers(sl);

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST ) );
   pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTexturedQuad(mgr, geom);

   pass.execGeometry( geom );

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void simple_mipmap( RCP<Gfx::Manager>& mgr )
{
   Gfx::Pass pass;
   pass.setClearColor( 0.0f, 0.0f, 0.5f, 1.0f );

   //Texture
   RCP<Gfx::Texture> texture;
   uint numLevels = 5;
   --numLevels;
   setTexture_Lines(mgr, texture, 1 << numLevels, 1 << numLevels);
   mgr->generateMipmaps(texture);

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST ) );
   pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTexturedUnitQuad(mgr, geom);

   pass.setGeometry( geom );


   float mat[] = {
      0.5f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
   };

   for( uint y = 0; y < 3; ++y )
   {
      // Start top-left
      mat[13] = 0.5f - 0.1f - 0.66f*y;
      for( uint x = 0; x < 3; ++x )
      {
         mat[12] = -1.0f + 0.1f + 0.66f*x;
         uint curLevel = y*3 + x;
         if( curLevel > numLevels )
         {
            break;
         }
         Gfx::TextureState tState;
         tState.setPointSampling();
         tState.LODBias(-10.0f);  // force LOD to be 0
         tState.baseLevel(curLevel);
         RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
         sl->addSampler( "tex", texture, tState );
         pass.setSamplers(sl);
         pass.setViewMatrix(mat);
         pass.exec();
      }
   }

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void simple_cubemap( RCP<Gfx::Manager>& mgr )
{
   Gfx::Pass pass;
   pass.setClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

   //Texture
   RCP<Gfx::Texture> texture;
   setTexture_cubemap(mgr, texture);
   Gfx::TextureState texState;
   setTextureState_Point(mgr, texState);
   RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
   sl->addSampler( "tex", texture, texState );
   pass.setSamplers(sl);

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_cubemap ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_cubemap ) );
   pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createCubemapQuads(mgr, geom);

   pass.execGeometry( geom );

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void simple_texture_rot( RCP<Gfx::Manager>& mgr )
{
   Gfx::Pass pass;
   pass.setClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

   //Texture
   RCP<Gfx::Texture> texture;
   setTexture_RGBA_4x4(mgr, texture);
   Gfx::TextureState texState;
   setTextureState_Point(mgr, texState);
   RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
   sl->addSampler( "tex", texture, texState );
   pass.setSamplers(sl);

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   //program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST ) );
   pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTexturedQuadRot(mgr, geom);

   pass.execGeometry( geom );

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void render_to_texture( RCP<Gfx::Manager>& mgr )
{
   //=================
   //RENDER TO TEXTURE
   //=================
   Gfx::Pass rtt_pass;

   RCP<Gfx::Texture> rtex = mgr->create2DTexture(
      16, 16,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_RENDERABLE
   );
   RCP<Gfx::Texture> rdtex = mgr->create2DTexture(
      rtex->width(), rtex->height(),
      Gfx::TEX_FMT_24_8,
      Gfx::TEX_CHANS_ZS,
      Gfx::TEX_FLAGS_RENDERABLE
   );

   RCP<Gfx::Framebuffer> fb = mgr->createFramebuffer();
   fb->setColorBuffer(rtex);
   fb->setDepthBuffer(rdtex);
   rtt_pass.setFramebuffer(fb);

   rtt_pass.setClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
   rtt_pass.setViewport(0, 0, rtex->width(), rtex->height());

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA_grayscale ) );
   rtt_pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );

   rtt_pass.execGeometry( geom );

   mgr->render(rtt_pass);

   //mgr->display(); Thread::sleep( 2.0 );

   //=======================
   //USE RESULT AS A TEXTURE
   //=======================
   Gfx::Pass pass;
   pass.setClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
   pass.setViewport(0, 0, 320, 240);

   //Texture
   Gfx::TextureState rtexState;
   setTextureState_Bilinear(mgr, rtexState);
   setTextureState_ClampToLast(mgr, rtexState);
   RCP<Gfx::SamplerList> rsl( new Gfx::SamplerList() );
   rsl->addSampler( "tex", rtex, rtexState );
   pass.setSamplers(rsl);

   //Program
   RCP<Gfx::Program> rprogram( mgr->createProgram() );
   rprogram->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST ) );
   rprogram->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST ) );
   pass.setProgram( rprogram );

   //Geometry
   RCP<Gfx::Geometry> rgeom;
   createTexturedQuadRot(mgr, rgeom);

   pass.execGeometry( rgeom );

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void render_to_texture_mipmap( RCP<Gfx::Manager>& mgr )
{
   //=================
   //RENDER TO TEXTURE
   //=================
   Gfx::Pass rtt_pass;

   RCP<Gfx::Texture> cb = mgr->create2DTexture(
      16, 16,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_RENDERABLE|Gfx::TEX_FLAGS_MIPMAPPED
   );
   RCP<Gfx::Texture> db = mgr->create2DTexture(
      cb->width(), cb->height(),
      Gfx::TEX_FMT_24_8,
      Gfx::TEX_CHANS_ZS,
      Gfx::TEX_FLAGS_RENDERABLE
   );

   RCP<Gfx::Framebuffer> fb = mgr->createFramebuffer();
   fb->setColorBuffer(cb);
   fb->setDepthBuffer(db);
   rtt_pass.setFramebuffer(fb);
   rtt_pass.mipmaps( true );

   rtt_pass.setClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
   rtt_pass.setViewport(0, 0, cb->width(), cb->height());

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA_grayscale ) );
   rtt_pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );

   rtt_pass.execGeometry( geom );

   mgr->render(rtt_pass);


   Gfx::Pass pass;
   pass.setClearColor( 0.0f, 0.0f, 0.5f, 1.0f );

   //Texture
   //Use the color buffe above
   RCP<Gfx::Texture> texture = cb;
   uint numLevels = 5; //it's 16x16
   --numLevels;  //keeps same break condition as in simple_mipmap

   //Program
   program = mgr->createProgram();
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST ) );
   pass.setProgram( program );

   //Geometry
   geom = NULL;
   createTexturedUnitQuad(mgr, geom);

   pass.setGeometry( geom );


   float mat[] = {
      0.5f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
   };

   for( uint y = 0; y < 3; ++y )
   {
      // Start top-left
      mat[13] = 0.5f - 0.1f - 0.66f*y;
      for( uint x = 0; x < 3; ++x )
      {
         mat[12] = -1.0f + 0.1f + 0.66f*x;
         uint curLevel = y*3 + x;
         if( curLevel > numLevels )
         {
            break;
         }
         Gfx::TextureState tState;
         tState.setPointSampling();
         tState.LODBias(-10.0f);  // force LOD to be 0
         tState.baseLevel(curLevel);
         RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
         sl->addSampler( "tex", texture, tState );
         pass.setSamplers(sl);
         pass.setViewMatrix(mat);
         pass.exec();
      }
   }

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void render_to_texture_mipmap_manual( RCP<Gfx::Manager>& mgr )
{
   //=================
   //RENDER TO TEXTURE
   //=================
   Gfx::Pass rtt_pass;

   RCP<Gfx::Texture> cb = mgr->create2DTexture(
      16, 16,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_RENDERABLE|Gfx::TEX_FLAGS_MIPMAPPED
   );
   uint numLevels = cb->getNumLevelsFromSize();
   RCP<Gfx::Texture> db = mgr->create2DTexture(
      cb->width(), cb->height(),
      Gfx::TEX_FMT_24_8,
      Gfx::TEX_CHANS_ZS,
      Gfx::TEX_FLAGS_RENDERABLE
   );

   RCP<Gfx::Framebuffer> fb = mgr->createFramebuffer();
   fb->setColorBuffer(cb);
   fb->setDepthBuffer(db);
   rtt_pass.setFramebuffer(fb);

   rtt_pass.setClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
   rtt_pass.setViewport(0, 0, fb->width(), fb->height());

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA_grayscale ) );
   rtt_pass.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );

   rtt_pass.execGeometry( geom );

   mgr->render(rtt_pass);

   //mgr->generateMipmaps(cb);

   Gfx::TextureState tState;
   tState.setPointSampling();
   //OSX fallbacks in software renderer if unsupported clamp policy is used
   tState.clampX(Gfx::TEX_CLAMP_LAST);
   tState.clampY(Gfx::TEX_CLAMP_LAST);
   tState.mipFilter(Gfx::TEX_FILTER_NONE);
   RCP<Gfx::SamplerList> sampler( new Gfx::SamplerList() );
   sampler->addSampler("tex", cb, tState);
   //Program
   program = mgr->createProgram();
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST2 ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST_grayscale ) );
   for( uint level = 1; level < numLevels; ++level )
   {
      tState.baseLevel(level-1);  //Level to be read
      RCP<Gfx::Pass> pass = mgr->createFilterPass(sampler, program, cb, level);
      mgr->render(*pass);
   }

   Gfx::Pass pass;
   pass.setClearColor( 0.0f, 0.0f, 0.5f, 1.0f );

   //Texture
   //Use the color buffer above
   RCP<Gfx::Texture> texture = cb;
   --numLevels;  //keeps same break condition as in simple_mipmap

   //Program
   program = mgr->createProgram();
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST ) );
   pass.setProgram( program );

   //Geometry
   geom = NULL;
   createTexturedUnitQuad(mgr, geom);

   pass.setGeometry( geom );


   float mat[] = {
      0.5f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
   };

   for( uint y = 0; y < 3; ++y )
   {
      // Start top-left
      mat[13] = 0.5f - 0.1f - 0.66f*y;
      for( uint x = 0; x < 3; ++x )
      {
         mat[12] = -1.0f + 0.1f + 0.66f*x;
         uint curLevel = y*3 + x;
         if( curLevel > numLevels )
         {
            break;
         }
         Gfx::TextureState tState;
         tState.setPointSampling();
         tState.LODBias(-10.0f);  // force LOD to be 0
         tState.baseLevel(curLevel);
         RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
         sl->addSampler( "tex", texture, tState );
         pass.setSamplers(sl);
         pass.setViewMatrix(mat);
         pass.exec();
      }
   }

   mgr->render(pass);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void simple_node( RCP<Gfx::Manager>& mgr )
{
   /**
    * Some explanations.
    *
    * This example shows how to build a render tree using RenderNodes.
    * The rendering is as follows:
    *                    nodeClear
    *                        |
    *               +--------+--------+
    *               |                 |
    *               |             nodeRightA
    *            nodeLeft             |
    *               |             nodeRightB
    *               |                 |
    *               +--------+--------+
    *                        |
    *                     nodeEnd
    *
    * where:
    * - nodeClear clears the whole framebuffer
    * - nodeLeft renders both triangles using 2 separate passes (left triangle, then right)
    * - nodeRightA renders the first triangle (the one on the right)
    * - nodeRightB renders the second triangle (the one on the left, overwriting A)
    * - nodeEnd only serves as a sync point for Left and Right paths
    *
    * The intented end result is to show a different rendering order on the left and on the right
    */
   //Program
   RCP<Gfx::Program> programColor( mgr->createProgram() );
   programColor->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
   programColor->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA ) );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );

   //DepthState
   RCP<Gfx::DepthState> dState( new Gfx::DepthState() );
   dState->depthTesting(false);

   RCP<Gfx::Pass> pass;  //reused for all passes
   uint geomTri1[] = { 0, 3 };
   uint geomTri2[] = { 3, 3 };

   ///////
   // Init
   RCP<Gfx::RenderNode> nodeClear( new Gfx::RenderNode() );
   pass = new Gfx::Pass();
   pass->setClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
   nodeClear->addPass( pass );

   ///////
   // Left
   RCP<Gfx::RenderNode> nodeLeft( new Gfx::RenderNode() );
   nodeLeft->addRequirement(nodeClear);
   pass = new Gfx::Pass();
   pass->setClear( Gfx::CLEAR_NONE );
   pass->setViewport( 0, 0, 160, 200 );
   pass->setProgram( programColor );
   pass->setDepthState(dState);
   pass->execRangeGeometry( geom, geomTri1 );
   nodeLeft->addPass( pass );
   pass = new Gfx::Pass();
   pass->setClear( Gfx::CLEAR_NONE );
   pass->setViewport( 0, 0, 160, 200 );
   pass->setProgram( programColor );
   pass->execRangeGeometry( geom, geomTri2 );
   nodeLeft->addPass( pass );

   ////////
   // Right
   RCP<Gfx::RenderNode> nodeRightA( new Gfx::RenderNode() );
   RCP<Gfx::RenderNode> nodeRightB( new Gfx::RenderNode() );

   // Render the first triangle (this will be done AFTER nodeRightA)
   nodeRightB->addRequirement(nodeRightA);
   pass = new Gfx::Pass();
   pass->setClear( Gfx::CLEAR_NONE );
   pass->setViewport( 160, 0, 160, 200 );
   pass->setProgram( programColor );
   pass->setDepthState(dState);
   pass->execRangeGeometry( geom, geomTri1 );
   nodeRightB->addPass( pass );

   // Render the second triangle (this will be done before nodeRightB)
   nodeRightA->addRequirement(nodeClear);
   pass = new Gfx::Pass();
   pass->setClear( Gfx::CLEAR_NONE );
   pass->setViewport( 160, 0, 160, 200 );
   pass->setProgram( programColor );
   pass->setDepthState(dState);
   pass->execRangeGeometry( geom, geomTri2 );
   nodeRightA->addPass( pass );

   //////
   // End
   RCP<Gfx::RenderNode> nodeEnd( new Gfx::RenderNode() );
   nodeEnd->addRequirement(nodeLeft);
   nodeEnd->addRequirement(nodeRightB);

   mgr->render(nodeEnd);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void simple_filter( RCP<Gfx::Manager>& mgr )
{
   uint w, h;
   mgr->getSize(w, h);

   printf("%dx%d\n", w, h);
   //w = 128;
   //h = 128;

   //=================
   //RENDER TO TEXTURE
   //=================
   Gfx::Pass scene;

   RCP<Gfx::Texture> cb = mgr->create2DTexture(
      w, h,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_RENDERABLE
   );
   RCP<Gfx::Texture> db = mgr->create2DTexture(
      cb->width(), cb->height(),
      Gfx::TEX_FMT_24_8,
      Gfx::TEX_CHANS_ZS,
      Gfx::TEX_FLAGS_RENDERABLE
   );

   RCP<Gfx::Framebuffer> fb = mgr->createFramebuffer();
   fb->setColorBuffer(cb);
   fb->setDepthBuffer(db);
   scene.setFramebuffer(fb);

   scene.setClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
   scene.setViewport(0, 0, cb->width(), cb->height());

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA ) );
   scene.setProgram( program );

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );
   scene.execGeometry( geom );

   mgr->render(scene);

   //mgr->display(); Thread::sleep( 2.0 );

   // Apply filter to scene
   RCP<Gfx::Program> filterPgm( mgr->createProgram() );
   filterPgm->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST2 ) );
   filterPgm->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST_grayscale ) );
   RCP<Gfx::Pass> filterPass = mgr->createFilterPass(cb, filterPgm, NULL, 0, true);

   mgr->render( *filterPass );

   mgr->display();
}

//------------------------------------------------------------------------------
//!
void hdr( RCP<Gfx::Manager>& mgr )
{
   uint w, h;
   mgr->getSize(w, h);

#define FIRST_PASS_8888 0
#define USE_DEPTH_TEXTURE 0

   float sf = 4.0f;

   printf("%dx%d\n", w, h);
   //w = 128;
   //h = 128;

   //=================
   //RENDER TO TEXTURE
   //=================
   RCP<Gfx::Pass> scene( new Gfx::Pass() );

   RCP<Gfx::Texture> cbHdr = mgr->create2DTexture(
      w, h,
      Gfx::TEX_FMT_16F_16F_16F_16F,
      //Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_RENDERABLE
   );
   RCP<Gfx::Texture> cb = mgr->create2DTexture(
      w, h,
      Gfx::TEX_FMT_8_8_8_8,
      Gfx::TEX_CHANS_RGBA,
      Gfx::TEX_FLAGS_RENDERABLE
   );
#if USE_DEPTH_TEXTURE
   RCP<Gfx::Texture> db = mgr->create2DTexture(
      cb->width(), cb->height(),
      Gfx::TEX_FMT_24_8,
      Gfx::TEX_CHANS_ZS,
      Gfx::TEX_FLAGS_RENDERABLE
   );
#endif

   RCP<Gfx::Framebuffer> fb = mgr->createFramebuffer();
#if FIRST_PASS_8888
   fb->setColorBuffer(cb);
#else
   fb->setColorBuffer(cbHdr);
#endif
#if USE_DEPTH_TEXTURE
   fb->setDepthBuffer(db);
#endif
   scene->setFramebuffer(fb);

#if FIRST_PASS_8888
   scene->setClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
#else
   scene->setClearColor( 0.5f*sf, 0.0f*sf, 0.0f*sf, 1.0f*sf );
#endif
   scene->setViewport(0, 0, cb->width(), cb->height());

   //Program
   RCP<Gfx::Program> program( mgr->createProgram() );
   program->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_RGBA ) );
#if FIRST_PASS_8888
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA ) );
#else
   program->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_RGBA_scale ) );
#endif
   scene->setProgram( program );

#if !FIRST_PASS_8888
   //Constants
   RCP<Gfx::ConstantBuffer> constants = mgr->createConstants( program );
   constants->setConstant("scaleFactor", &sf);
   scene->setConstants( Gfx::ConstantList::create(constants) );
#endif

   //Geometry
   RCP<Gfx::Geometry> geom;
   createTwoTriangles( mgr, geom );
   scene->execGeometry( geom );

   RCP<Gfx::RenderNode> rn( new Gfx::RenderNode() );
   rn->addPass( scene );


   // Scale the color up, and store in HDR buffer, then scale it down, and display.
   RCP<Gfx::Program> filterPgm( mgr->createProgram() );
   filterPgm->addShader( mgr->createShader( Gfx::VERTEX_SHADER, vs_str_XYZ_ST2 ) );
   filterPgm->addShader( mgr->createShader( Gfx::FRAGMENT_SHADER, ps_str_XYZ_ST_scale ) );

   RCP<Gfx::ConstantBuffer> cUp = mgr->createConstants( filterPgm );
   cUp->setConstant("scaleFactor", &sf);
   RCP<Gfx::Pass> scaleUpPass = mgr->createFilterPass( cb, filterPgm, Gfx::ConstantList::create(cUp), cbHdr );
#if FIRST_PASS_8888
   rn->addPass( scaleUpPass );
#endif

   RCP<Gfx::ConstantBuffer> cDown = mgr->createConstants( filterPgm );
   sf = 1.0f/sf;  //if the buffer above is indeed float, the result should be the original image
   cDown->setConstant("scaleFactor", &sf);
   RCP<Gfx::Pass> scaleDownPass = mgr->createFilterPass( cbHdr, filterPgm, Gfx::ConstantList::create(cDown), mgr->backBuffer() );
   rn->addPass( scaleDownPass );

   mgr->render(rn);
   mgr->display();
}

//------------------------------------------------------------------------------
//!
void  flicker( RCP<Gfx::Manager>& mgr )
{
   for(uint i = 0; i < 100; ++i )
   {
      simple(mgr);
      simple_texture(mgr);
      simple_texture_rot(mgr);
   }
}

//------------------------------------------------------------------------------
//!
void  constants( RCP<Gfx::Manager>& /*mgr*/ )
{
   Gfx::ConstantBuffer::Container constants;
   constants.pushBack( Gfx::ConstantBuffer::Constant("a", Gfx::CONST_FLOAT,  Gfx::ConstantBuffer::getSize(constants)) );
   constants.pushBack( Gfx::ConstantBuffer::Constant("b", Gfx::CONST_FLOAT,  Gfx::ConstantBuffer::getSize(constants)) );
   constants.pushBack( Gfx::ConstantBuffer::Constant("b", Gfx::CONST_FLOAT4, Gfx::ConstantBuffer::getSize(constants)) );
   printf("Required size is: %d bytes\n", (int)Gfx::ConstantBuffer::getSize(constants));
   if( Gfx::ConstantBuffer::getSize(constants) != 24 )
   {
      printf("PROBLEM - ConstantBuffer should have a size of 24 but instead get %d\n", (int)Gfx::ConstantBuffer::getSize(constants));
   }
}

//------------------------------------------------------------------------------
//!
bool  state_compare( RCP<Gfx::Manager>& /*mgr*/ )
{
   uint pass_count = 0;
   uint fail_count = 0;

   Gfx::AlphaState as1, as2;
   checkIf( as1.compare(as2) == 0, pass_count, fail_count );
   as2.alphaBlending( !as2.alphaBlending() );
   checkIf( as1.compare(as2) == 1, pass_count, fail_count );
   as2.alphaBlendDst( Gfx::ALPHA_BLEND_BLEND_FACTOR );
   checkIf( as1.compare(as2) == 3, pass_count, fail_count );
   as1.alphaBlendDst( Gfx::ALPHA_BLEND_BLEND_FACTOR );
   checkIf( as1.compare(as2) == 1, pass_count, fail_count );
   as1.alphaBlendDst( Gfx::ALPHA_BLEND_BLEND_FACTOR );
   checkIf( as1.compare(as2) == 1, pass_count, fail_count );
   as2.alphaTesting( !as2.alphaTesting() );
   checkIf( as1.compare(as2) == 5, pass_count, fail_count );
   as2.alphaBlendSrc( Gfx::ALPHA_BLEND_BLEND_FACTOR );
   checkIf( as1.compare(as2) == 7, pass_count, fail_count );
   as2.alphaTestFunc( Gfx::COMPARE_FUNC_NOTEQUAL );
   checkIf( as1.compare(as2) == 15, pass_count, fail_count );
   as2.alphaTestRef( 0.5f );
   checkIf( as1.compare(as2) == 31, pass_count, fail_count );
   as2 = as1;
   checkIf( as1.compare(as2) == 0, pass_count, fail_count );
   as2.alphaTestRef( 0.5f );
   checkIf( as1.compare(as2) == 16, pass_count, fail_count );

   Gfx::DepthState ds1, ds2;
   checkIf( ds1.compare(ds2) == 0, pass_count, fail_count );
   ds2.depthTesting( !ds2.depthTesting() );
   checkIf( ds1.compare(ds2) == 1, pass_count, fail_count );
   ds1.depthWriting( !ds1.depthWriting() );
   checkIf( ds1.compare(ds2) == 5, pass_count, fail_count );
   ds1.depthTestFunc( Gfx::COMPARE_FUNC_NOTEQUAL );
   checkIf( ds1.compare(ds2) == 7, pass_count, fail_count );

   Gfx::StencilState ss1, ss2;
   checkIf( ss1.compare(ss2) == 0, pass_count, fail_count );
   ss2.stencilPassDepthPassOp( Gfx::STENCIL_OP_INVERT );
   checkIf( ss1.compare(ss2) == 128, pass_count, fail_count );
   ss2.stencilPassDepthFailOp( Gfx::STENCIL_OP_INVERT );
   checkIf( ss1.compare(ss2) == 192, pass_count, fail_count );
   ss2.stencilFailOp( Gfx::STENCIL_OP_INVERT );
   checkIf( ss1.compare(ss2) == 224, pass_count, fail_count );
   // swap with a field1 entry here
   ss2.stencilTestWriteMask( 0xAB );
   checkIf( ss1.compare(ss2) == 232, pass_count, fail_count );
   ss2.stencilTestFunc( Gfx::COMPARE_FUNC_NOTEQUAL );
   checkIf( ss1.compare(ss2) == 248, pass_count, fail_count );
   ss1.stencilTesting( !ss1.stencilTesting() );
   checkIf( ss1.compare(ss2) == 249, pass_count, fail_count );
   ss2.stencilTestRefMask( 0xBA );
   checkIf( ss1.compare(ss2) == 253, pass_count, fail_count );
   ss2.stencilTestRef( 0xAA );
   checkIf( ss1.compare(ss2) == 255, pass_count, fail_count );

   uint total_count = pass_count + fail_count;
   double rate = 100.0 * pass_count/total_count;
   printf("State comparison test suite: %d/%d passed (%g %%)\n",
          pass_count, total_count, rate);

   return fail_count == 0;
}

//------------------------------------------------------------------------------
//!
void  all( RCP<Gfx::Manager>& mgr )
{
   constants(mgr);
   simple(mgr);                           Thread::sleep( 2.0 );
   simple_texture(mgr);                   Thread::sleep( 2.0 );
   simple_texture_rot(mgr);               Thread::sleep( 2.0 );
   simple_mipmap(mgr);                    Thread::sleep( 2.0 );
   simple_cubemap(mgr);                   Thread::sleep( 2.0 );
   render_to_texture(mgr);                Thread::sleep( 2.0 );
   render_to_texture_mipmap(mgr);         Thread::sleep( 2.0 );
   render_to_texture_mipmap_manual(mgr);  Thread::sleep( 2.0 );
   simple_node(mgr);                      Thread::sleep( 2.0 );
   simple_filter(mgr);
}


/*==============================================================================
   UTILITY
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool  runTest( RCP<Gfx::Manager>& mgr, const char* testStr )
{
   String testname = String(testStr).lower();

#define IF_TEST( name ) \
   if( testname == #name ) \
   { \
      printf("\n"); \
      printf("RUNNING: %s\n", #name); \
      name(mgr); \
      printf("DONE RUNNING: %s\n", #name); \
   }

   IF_TEST(simple)
   else
   IF_TEST(simple_texture)
   else
   IF_TEST(simple_mipmap)
   else
   IF_TEST(simple_cubemap)
   else
   IF_TEST(simple_texture_rot)
   else
   IF_TEST(render_to_texture)
   else
   IF_TEST(render_to_texture_mipmap)
   else
   IF_TEST(render_to_texture_mipmap_manual)
   else
   IF_TEST(simple_node)
   else
   IF_TEST(simple_filter)
   else
   IF_TEST(hdr)
   else
   IF_TEST(flicker)
   else
   IF_TEST(constants)
   else
   IF_TEST(state_compare)
   else
   IF_TEST(all)
   else
   {
      printf("Unknown example: %s\n", testname.cstr());
      return false;
   }
#undef IF_TEST

   //Show result for a few seconds, then quit
   Thread::sleep( 2.0 );

   return true;
}

/*==============================================================================
   MAIN
==============================================================================*/

int
main
( int argc, char** argv )
{
   uint w = 320;
   uint h = 240;
   //w = 800;
   //h = 600;
   //w = 1280;
   //h = 720;
   Gfx::Window window;

   //Some parameters
   window.setSize(w, h);
   window.initialize();

   //Render the pass set up above
   RCP<Gfx::Context> cntx = Gfx::Context::getDefaultContext( MANAGER_STR, window.getHandle() );
   RCP<Gfx::Manager> mgr =  Gfx::Manager::create( cntx.ptr() );

   mgr->setSize(w, h);

   initShaderStrings(mgr->API());

   String testname;

   // Read the argument and set the screen size.
   bool ranSomeTest = false;
   for( int i = 1; i < argc; ++i )
   {
      if( argv[i][0] == '-' )
      {
         //Parse arguments
         //(none for now)
      }
      else
      {
         if( !runTest(mgr, argv[i]) )
         {
            return 1;
         }
         ranSomeTest = true;
      }
   }

   if( !ranSomeTest)
   {
      while(false)
      {
         runTest(mgr, "simple_mipmap");
      }

      // Run default test
      if( !runTest(mgr, "all") )
      {
         //return 1;
      }
   }

   printf("TESTS FINISHED.  Press a key to quit\n");
   fflush(NULL);
   getchar();  //will wait for enter

   return 0;
}
