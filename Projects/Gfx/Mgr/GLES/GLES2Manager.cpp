/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GLES/GLES2Manager.h>

#if GFX_GLES_2_SUPPORT

//Complete forward declarations
#include <Gfx/FB/RenderState.h>
#include <Gfx/Geom/Buffer.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Mgr/GLES/GLESContext.h>
#include <Gfx/Prog/Program.h>
#include <Gfx/Tex/Sampler.h>
#include <Gfx/Tex/Texture.h>
#include <Gfx/Tex/TextureState.h>

#include <CGMath/Mat4.h>

#include <Base/ADT/Map.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Bits.h>
#include <Base/Util/Platform.h>

#include <cassert>
#include <cstdio>

#define BUFFER_OFFSET(i) ((char*)nullptr + (i))

#define glCheckError( v ) { \
   GLenum err = glGetError(); \
   if (err != GL_NO_ERROR) { \
      fprintf(stderr, "glCheckError: %04x caught at %s:%u\n", err, __FILE__, __LINE__); \
      CHECK(v); \
   } \
}

/*==============================================================================
   GLOBAL NAMESPACE
==============================================================================*/

//--------------------------------------------
// OpenGL headers and initialization routines
//--------------------------------------------

#if PLAT_APPLE

//#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#elif PLAT_ANDROID

#include <KHR/khrplatform.h>
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2ext.h>
typedef char GLchar;  // Apparently, GLchar got deprecated.

#elif PLAT_WINDOWS

#include <KHR/khrplatform.h>
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#define snprintf _snprintf

#else

#error Add support for your platform

#endif


/* GL_OES_texture3D */
#ifndef GL_OES_texture3D
#define GL_TEXTURE_WRAP_R_OES                                   0x8072
#define GL_TEXTURE_3D_OES                                       0x806F
#define GL_TEXTURE_BINDING_3D_OES                               0x806A
#define GL_MAX_3D_TEXTURE_SIZE_OES                              0x8073
#define GL_SAMPLER_3D_OES                                       0x8B5F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES        0x8CD4
#endif


USING_NAMESPACE
using namespace Gfx;


/*==============================================================================
   UNNAMED NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_gl, "GLES2Manager" );

#if PLAT_APPLE
// Apple declares the extension function pointers, but not the other platform.
#define gl_MapBufferOES    glMapBufferOES
#define gl_UnmapBufferOES  glUnmapBufferOES
#else
PFNGLMAPBUFFEROESPROC    gl_MapBufferOES;
PFNGLUNMAPBUFFEROESPROC  gl_UnmapBufferOES;
#endif

extern String vs_str_default;
extern String ps_str_default;

//------------------------------------------------------------------------------
//!
void initDefines( String& vsPreamble, String& psPreamble )
{
   // Prepare some GLSL #defines for some oddball cases.
   String defines;

#if __APPLE__
   defines += "#define DYN_LOOP_SLOW 1\n";
#else
   defines += "#define DYN_LOOP_SLOW 0\n";
#endif

   vsPreamble += defines;
   psPreamble += defines;
}

//------------------------------------------------------------------------------
//!
void createTextureFormatTable();


/*==============================================================================
   Utilities
==============================================================================*/

inline const char* gluErrorString( GLenum err )
{
   static char tmp[1024];
   const char* errStr;
   switch( err )
   {
      case GL_NO_ERROR         : errStr = "<none>"; break;
      case GL_INVALID_ENUM     : errStr = "INVALID ENUM"; break;
      case GL_INVALID_VALUE    : errStr = "INVALID VALUE"; break;
      case GL_INVALID_OPERATION: errStr = "INVALID OPERATION"; break;
      case GL_OUT_OF_MEMORY    : errStr = "OUT OF MEMORY"; break;
      default                  : errStr = "<unknown>"; break;
   }
   snprintf( tmp, 1024, "0x%04x (%s)", err, errStr );
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline void displayGLError(const char* fmt, GLenum err)
{
   printf(fmt, gluErrorString(err));
}

//------------------------------------------------------------------------------
//!
bool  checkForErrors(const char* fmt)
{
   GLenum err = glGetError();

   if( err == GL_NO_ERROR )
   {
      return false;
   }
   else
   {
      while( err != GL_NO_ERROR )
      {
         displayGLError(fmt, err);
         CHECK( false );
         err = glGetError();
      }
      return true;
   }
}

#if 0
//------------------------------------------------------------------------------
//!
void printGLErrors()
{
   GLenum err = glGetError();

   if( err == GL_NO_ERROR )
   {
      printf("GL_ERROR: none\n");
   }
   else
   {
      printf("GL_ERROR: %d\n", (int)err );
      printf("---------\n");
      while( err != GL_NO_ERROR )
      {
         printf("%s\n", gluErrorString(err));
         err = glGetError();
      }
      printf("---------\n");
   }
}
#endif

#ifdef _DEBUG
//------------------------------------------------------------------------------
//!
bool checkFramebufferStatus()
{
   GLenum status;
   status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   switch(status) {
      case GL_FRAMEBUFFER_COMPLETE:
         break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
         printf("Unsupported framebuffer format\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
         printf("Incomplete attachment in framebuffer\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
         printf("Framebuffer incomplete, missing attachment\n");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
         printf("Framebuffer incomplete, attached images must have same dimensions\n");
      default:
         printf("Unknown glCheckFramebufferStatus error: 0x%04x\n", status);
         break;
   }
   return status == GL_FRAMEBUFFER_COMPLETE;
}
#endif

//------------------------------------------------------------------------------
//!
void initFuncs()
{
   checkForErrors( "GLES2Manager initFuncs() - %s\n");
#if !PLAT_APPLE
   gl_MapBufferOES   = (PFNGLMAPBUFFEROESPROC  )eglGetProcAddress("glMapBufferOES"  );
   gl_UnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC)eglGetProcAddress("glUnmapBufferOES");
#endif
   CHECK( gl_MapBufferOES   );
   CHECK( gl_UnmapBufferOES );
   checkForErrors( "GLES2Manager initFuncs() - %s\n");
}

//------------------------------------------------------------------------------
//!
void initStates()
{
   //
   // Settings of default states.
   //
   glEnable    ( GL_BLEND );
   glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable    ( GL_SCISSOR_TEST );
   glEnable    ( GL_CULL_FACE );

   checkForErrors( "GLES2Manager initStates() - %s\n");
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( const PrimitiveType pt )
{
   switch( pt )
   {
      case PRIM_INVALID       : return (GLenum)-1;
      case PRIM_POINTS        : return GL_POINTS;
      case PRIM_LINES         : return GL_LINES;
      case PRIM_LINE_LOOP     : return GL_LINE_LOOP;
      case PRIM_LINE_STRIP    : return GL_LINE_STRIP;
      case PRIM_TRIANGLES     : return GL_TRIANGLES;
      case PRIM_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
      case PRIM_TRIANGLE_FAN  : return GL_TRIANGLE_FAN;
      case PRIM_QUADS         : return (GLenum)-1; //GL_QUADS;
      case PRIM_QUAD_STRIP    : return (GLenum)-1; //GL_QUAD_STRIP;
      case PRIM_POLYGON       : return (GLenum)-1; //GL_POLYGON;
      default                 : return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline void
toGL( const AttributeFormat afmt, uint& numChannels, GLenum& type )
{
   numChannels = toNumChannels(afmt);
   switch( afmt )
   {
      case ATTRIB_FMT_INVALID:
         type = (GLenum)-1;
         break;
      case ATTRIB_FMT_8      :
      case ATTRIB_FMT_8_8    :
      case ATTRIB_FMT_8_8_8  :
      case ATTRIB_FMT_8_8_8_8:
         type = GL_UNSIGNED_BYTE;
         break;
      case ATTRIB_FMT_16         :
      case ATTRIB_FMT_16_16      :
      case ATTRIB_FMT_16_16_16   :
      case ATTRIB_FMT_16_16_16_16:
         type = GL_UNSIGNED_SHORT;
         break;
      case ATTRIB_FMT_16F            :
      case ATTRIB_FMT_16F_16F        :
      case ATTRIB_FMT_16F_16F_16F    :
      case ATTRIB_FMT_16F_16F_16F_16F:
         type = GL_HALF_FLOAT_OES;
         break;
      case ATTRIB_FMT_32         :
      case ATTRIB_FMT_32_32      :
      case ATTRIB_FMT_32_32_32   :
      case ATTRIB_FMT_32_32_32_32:
         type = GL_UNSIGNED_INT;
         break;
      case ATTRIB_FMT_32F            :
      case ATTRIB_FMT_32F_32F        :
      case ATTRIB_FMT_32F_32F_32F    :
      case ATTRIB_FMT_32F_32F_32F_32F:
         type = GL_FLOAT;
         break;
      default:
         type = (GLenum)-1;
         break;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( const IndexFormat afmt )
{
   switch( afmt )
   {
      case INDEX_FMT_INVALID:
         return (GLenum)-1;
      case INDEX_FMT_8:
         return GL_UNSIGNED_BYTE;
      case INDEX_FMT_16:
         return GL_UNSIGNED_SHORT;
      case INDEX_FMT_32:
         return GL_UNSIGNED_INT;
      default:
         return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGLCubemapTarget( const uint slice )
{
   switch( slice )
   {
      case TEX_SLICE_NEG_X: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
      case TEX_SLICE_POS_X: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
      case TEX_SLICE_NEG_Y: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
      case TEX_SLICE_POS_Y: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
      case TEX_SLICE_NEG_Z: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
      case TEX_SLICE_POS_Z: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
      default             : return (GLenum)-1;
   }
}

struct TexFmtDataEntry
{
   GLint   internalFormat;
   GLenum  format;
   GLenum  type;
   TexFmtDataEntry( GLint intFmt = 0, GLenum fmt = (GLenum)-1, GLenum typ = 0 ):
      internalFormat(intFmt), format(fmt), type(typ) {}
};

//!< A table with all of the texture format data entries.
const int _gNumTexFormats = TEX_FMT_8_24+1;
TexFmtDataEntry  _gTextureFormatTable[_gNumTexFormats];

//------------------------------------------------------------------------------
//!
void createTextureFormatTable()
{
   //Note: the 32b fixed don't exist yet (but might once integer make it into the core)
   //                   TextureFormat                               internalFormat             format                type
   _gTextureFormatTable[TEX_FMT_INVALID        ] = TexFmtDataEntry( 0                        , (GLenum)-1          , (GLenum)-1               );
   _gTextureFormatTable[TEX_FMT_8              ] = TexFmtDataEntry( GL_LUMINANCE             , GL_LUMINANCE        , GL_UNSIGNED_BYTE         );
   _gTextureFormatTable[TEX_FMT_8_8            ] = TexFmtDataEntry( GL_LUMINANCE_ALPHA       , GL_LUMINANCE_ALPHA  , GL_UNSIGNED_BYTE         );
   //_gTextureFormatTable[TEX_FMT_8_8_8          ] = TexFmtDataEntry( GL_RGB8_OES              , GL_RGB              , GL_UNSIGNED_BYTE         ); //Probably not supported
   _gTextureFormatTable[TEX_FMT_8_8_8_8        ] = TexFmtDataEntry( GL_RGBA                  , GL_RGBA             , GL_UNSIGNED_BYTE         );
   _gTextureFormatTable[TEX_FMT_16             ] = TexFmtDataEntry( GL_LUMINANCE             , GL_LUMINANCE        , GL_UNSIGNED_SHORT        );
   _gTextureFormatTable[TEX_FMT_16_16          ] = TexFmtDataEntry( GL_LUMINANCE_ALPHA       , GL_LUMINANCE_ALPHA  , GL_UNSIGNED_SHORT        );
   //_gTextureFormatTable[TEX_FMT_16_16_16       ] = TexFmtDataEntry( GL_RGB16                 , GL_RGB              , GL_UNSIGNED_SHORT        ); //Probably not supported
   _gTextureFormatTable[TEX_FMT_16_16_16_16    ] = TexFmtDataEntry( GL_RGBA                  , GL_RGBA             , GL_UNSIGNED_SHORT        );
   _gTextureFormatTable[TEX_FMT_16F            ] = TexFmtDataEntry( GL_RED_EXT               , GL_RED_EXT          , GL_HALF_FLOAT_OES        );
   //_gTextureFormatTable[TEX_FMT_16F            ] = TexFmtDataEntry( GL_LUMINANCE             , GL_LUMINANCE        , GL_HALF_FLOAT_OES        );
   //_gTextureFormatTable[TEX_FMT_16F_16F        ] = TexFmtDataEntry( GL_LUMINANCE_ALPHA       , GL_LUMINANCE_ALPHA  , GL_HALF_FLOAT_OES        );
   _gTextureFormatTable[TEX_FMT_16F_16F        ] = TexFmtDataEntry( GL_RG_EXT                , GL_RG_EXT           , GL_HALF_FLOAT_OES        );
   //_gTextureFormatTable[TEX_FMT_16F_16F_16F    ] = TexFmtDataEntry( GL_RGB16F_ARB            , GL_RGB              , GL_HALF_FLOAT_ARB        ); //Probably not supported
   _gTextureFormatTable[TEX_FMT_16F_16F_16F_16F] = TexFmtDataEntry( GL_RGBA                  , GL_RGBA             , GL_HALF_FLOAT_OES        );
   _gTextureFormatTable[TEX_FMT_32             ] = TexFmtDataEntry( GL_LUMINANCE             , GL_LUMINANCE        , GL_UNSIGNED_INT          );
   //_gTextureFormatTable[TEX_FMT_32_32          ] = TexFmtDataEntry( 2                        , GL_LUMINANCE_ALPHA  , GL_UNSIGNED_INT          );
   //_gTextureFormatTable[TEX_FMT_32_32_32       ] = TexFmtDataEntry( 3                        , GL_RGB              , GL_UNSIGNED_INT          );
   //_gTextureFormatTable[TEX_FMT_32_32_32_32    ] = TexFmtDataEntry( 4                        , GL_RGBA             , GL_UNSIGNED_INT          );
   _gTextureFormatTable[TEX_FMT_32F            ] = TexFmtDataEntry( GL_LUMINANCE             , GL_LUMINANCE        , GL_FLOAT                 );
   _gTextureFormatTable[TEX_FMT_32F_32F        ] = TexFmtDataEntry( GL_LUMINANCE_ALPHA       , GL_LUMINANCE_ALPHA  , GL_FLOAT                 );
   //_gTextureFormatTable[TEX_FMT_32F_32F_32F    ] = TexFmtDataEntry( GL_RGB32F_ARB            , GL_RGB              , GL_FLOAT                 );
   _gTextureFormatTable[TEX_FMT_32F_32F_32F_32F] = TexFmtDataEntry( GL_RGBA                  , GL_RGBA             , GL_FLOAT                 );
   _gTextureFormatTable[TEX_FMT_24_8           ] = TexFmtDataEntry( GL_DEPTH_STENCIL_OES     , GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES ); //GL_DEPTH_COMPONENT24_OES
   _gTextureFormatTable[TEX_FMT_8_24           ] = TexFmtDataEntry( GL_DEPTH_STENCIL_OES     , GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES );
}

//------------------------------------------------------------------------------
//!
inline void
toGL( const TextureFormat tfmt, GLint& internalFormat, GLenum& format, GLenum& type )
{
   CHECK( tfmt < _gNumTexFormats );
   const TexFmtDataEntry& entry = _gTextureFormatTable[tfmt];
   internalFormat = entry.internalFormat;
   format         = entry.format;
   type           = entry.type;
}

//------------------------------------------------------------------------------
//!
inline GLint
toGLInternalFormat( const TextureFormat tfmt )
{
   CHECK( tfmt < _gNumTexFormats );
   return _gTextureFormatTable[tfmt].internalFormat;
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( const MapMode mode )
{
   switch( mode )
   {
      case MAP_READ      : return (GLenum)-1; // FIXME
      case MAP_WRITE     : return GL_WRITE_ONLY_OES;
      case MAP_READ_WRITE: return GL_WRITE_ONLY_OES; // FIXME
      default            : return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline void
toGLFilters(
   const TextureState& state,
   GLenum&             magFilter,
   GLenum&             minFilter,
   GLint&              maxAniso
)
{
   if( state.maxAniso() != 0 )
   {
      magFilter = GL_LINEAR;
      minFilter = GL_LINEAR_MIPMAP_LINEAR;
      maxAniso  = state.maxAniso();
   }
   else
   {
      switch( state.magFilter() )
      {
         case TEX_FILTER_POINT : magFilter = GL_NEAREST; break;
         case TEX_FILTER_LINEAR: magFilter = GL_LINEAR;  break;
         default               : magFilter = GL_NEAREST; break;
      }
      switch( state.minFilter() )
      {
         case TEX_FILTER_POINT:
         {
            switch( state.mipFilter() )
            {
               case TEX_FILTER_NONE:   minFilter = GL_NEAREST;                break;
               case TEX_FILTER_POINT:  minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
               case TEX_FILTER_LINEAR: minFilter = GL_NEAREST_MIPMAP_LINEAR;  break;
               default:                minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
            }
         } break;
         case TEX_FILTER_LINEAR:
         {
            switch( state.mipFilter() )
            {
               case TEX_FILTER_NONE:   minFilter = GL_LINEAR;                 break;
               case TEX_FILTER_POINT:  minFilter = GL_LINEAR_MIPMAP_NEAREST;  break;
               case TEX_FILTER_LINEAR: minFilter = GL_LINEAR_MIPMAP_LINEAR;   break;
               default:                minFilter = GL_LINEAR_MIPMAP_NEAREST;  break;
            }
         } break;
         default:
         {
            switch( state.mipFilter() )
            {
               case TEX_FILTER_NONE:   minFilter = GL_NEAREST;                break;
               case TEX_FILTER_POINT:  minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
               case TEX_FILTER_LINEAR: minFilter = GL_NEAREST_MIPMAP_LINEAR;  break;
               default:                minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
            }
         } break;
      }
      maxAniso = 1;
   }
}

#if 0
//------------------------------------------------------------------------------
//!
inline GLenum
removeMipmap( const GLenum f )
{
   switch( f )
   {
      case GL_NEAREST:
      case GL_NEAREST_MIPMAP_NEAREST:
      case GL_NEAREST_MIPMAP_LINEAR:
         return GL_NEAREST;
      case GL_LINEAR:
      case GL_LINEAR_MIPMAP_NEAREST:
      case GL_LINEAR_MIPMAP_LINEAR:
         return GL_NEAREST;
      default:
         assert(0 && "INVALID ENUM");
         return (GLenum)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( TextureType texType )
{
   switch( texType )
   {
      case TEX_TYPE_UNSPECIFIED: return (GLenum)-1;
      case TEX_TYPE_1D         : return (GLenum)-1; //GL_TEXTURE_1D;
      case TEX_TYPE_1D_ARRAY   : return (GLenum)-1;
      case TEX_TYPE_2D         : return GL_TEXTURE_2D;
      case TEX_TYPE_2D_ARRAY   : return (GLenum)-1;
      case TEX_TYPE_3D         : return (GLenum)-1; //GL_TEXTURE_3D;
      case TEX_TYPE_CUBEMAP    : return GL_TEXTURE_CUBE_MAP;
      default                  : return (GLenum)-1;
   }
}
#endif

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( TexClamp clamp )
{
   switch( clamp )
   {
      case TEX_CLAMP_WRAP             : return GL_REPEAT;
      case TEX_CLAMP_MIRROR           : return GL_MIRRORED_REPEAT;
      case TEX_CLAMP_LAST             : return GL_CLAMP_TO_EDGE;
      case TEX_CLAMP_BORDER           : return GL_CLAMP_TO_EDGE; //GL_CLAMP_TO_BORDER;
      case TEX_CLAMP_MIRRORONCE_LAST  : return GL_MIRRORED_REPEAT; //GL_MIRROR_CLAMP_TO_EDGE;
      case TEX_CLAMP_MIRRORONCE_BORDER: return GL_MIRRORED_REPEAT; //GL_MIRROR_CLAMP_TO_BORDER;
      default                         : return GL_CLAMP_TO_EDGE;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( CompareFunc func )
{
   switch( func )
   {
      case COMPARE_FUNC_NEVER       : return GL_NEVER;
      case COMPARE_FUNC_LESS        : return GL_LESS;
      case COMPARE_FUNC_EQUAL       : return GL_EQUAL;
      case COMPARE_FUNC_LESSEQUAL   : return GL_LEQUAL;
      case COMPARE_FUNC_GREATER     : return GL_GREATER;
      case COMPARE_FUNC_NOTEQUAL    : return GL_NOTEQUAL;
      case COMPARE_FUNC_GREATEREQUAL: return GL_GEQUAL;
      case COMPARE_FUNC_ALWAYS      : return GL_ALWAYS;
      default                       : return GL_LEQUAL;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( Gfx::AlphaBlend blend )
{
   switch( blend )
   {
      case ALPHA_BLEND_ZERO                 : return GL_ZERO;
      case ALPHA_BLEND_ONE                  : return GL_ONE;
      case ALPHA_BLEND_SRC_COLOR            : return GL_SRC_COLOR;
      case ALPHA_BLEND_INV_SRC_COLOR        : return GL_ONE_MINUS_SRC_COLOR;
      case ALPHA_BLEND_SRC_ALPHA            : return GL_SRC_ALPHA;
      case ALPHA_BLEND_INV_SRC_ALPHA        : return GL_ONE_MINUS_SRC_ALPHA;
      case ALPHA_BLEND_DST_ALPHA            : return GL_DST_ALPHA;
      case ALPHA_BLEND_INV_DST_ALPHA        : return GL_ONE_MINUS_DST_ALPHA;
      case ALPHA_BLEND_DST_COLOR            : return GL_DST_COLOR;
      case ALPHA_BLEND_INV_DST_COLOR        : return GL_ONE_MINUS_DST_COLOR;
      case ALPHA_BLEND_SRC_ALPHA_SAT        : return GL_SRC_ALPHA_SATURATE;
      case ALPHA_BLEND_BOTH_SRC_ALPHA       : return GL_SRC_ALPHA;           // ???
      case ALPHA_BLEND_BOTH_INV_SRC_ALPHA   : return GL_ONE_MINUS_SRC_ALPHA; // ???
      case ALPHA_BLEND_BLEND_FACTOR         : return GL_CONSTANT_COLOR;
      case ALPHA_BLEND_INV_BLEND_FACTOR     : return GL_ONE_MINUS_CONSTANT_COLOR;
      default                               : return GL_ONE;
   }
}

//------------------------------------------------------------------------------
//!
inline GLenum
toGL( StencilOp op )
{
   switch( op )
   {
      case STENCIL_OP_KEEP     : return GL_KEEP;
      case STENCIL_OP_ZERO     : return GL_ZERO ;
      case STENCIL_OP_REPLACE  : return GL_REPLACE;
      case STENCIL_OP_INCR_SAT : return GL_INCR;
      case STENCIL_OP_DECR_SAT : return GL_DECR;
      case STENCIL_OP_INVERT   : return GL_INVERT;
      case STENCIL_OP_INCR     : return GL_INCR_WRAP;
      case STENCIL_OP_DECR     : return GL_DECR_WRAP;
      default                  : return GL_KEEP;
   }
}

//------------------------------------------------------------------------------
//!
inline GLuint
toGL( AttributeType at )
{
   switch( at )
   {
      case ATTRIB_TYPE_UNSPECIFIED: return (GLuint)-1; break;
      case ATTRIB_TYPE_POSITION   : return  1; break;
      case ATTRIB_TYPE_COLOR      : return  2; break;
      case ATTRIB_TYPE_TEXCOORD0  : return  3; break;
      case ATTRIB_TYPE_TEXCOORD1  : return  4; break;
      case ATTRIB_TYPE_TEXCOORD2  : return  5; break;
      case ATTRIB_TYPE_TEXCOORD3  : return  6; break;
      case ATTRIB_TYPE_TEXCOORD4  : return  7; break;
      case ATTRIB_TYPE_TEXCOORD5  : return  8; break;
      case ATTRIB_TYPE_TEXCOORD6  : return  9; break;
      case ATTRIB_TYPE_TEXCOORD7  : return 10; break;
      case ATTRIB_TYPE_NORMAL     : return 11; break;
      case ATTRIB_TYPE_TANGENT    : return 12; break;
      case ATTRIB_TYPE_BINORMAL   : return 13; break;
      default                     : return (GLuint)-1; break;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const AlphaState& state, AlphaState& curState )
{
   switch( state.compare(curState) )
   {
      case 0:
         // Identical, nothing to do.
         return;
      case 1:
         // Optimized path: only blending state changed.
         if( state.alphaBlending() )
         {
            glEnable( GL_BLEND );
         }
         else
         {
            glDisable( GL_BLEND );
         }
         break;
      case 4:
         // Testing.
         if( state.alphaTesting() )
         {
            //glEnable( GL_ALPHA_TEST );
            StdErr << "OpenGL ES doesn't support alpha testing" << nl;
         }
         else
         {
            //glDisable( GL_ALPHA_TEST );
         }
         break;
      case 5:
         // Blending.
         if( state.alphaBlending() )
         {
            glEnable( GL_BLEND );
         }
         else
         {
            glDisable( GL_BLEND );
         }
         // Testing.
         if( state.alphaTesting() )
         {
            //glEnable( GL_ALPHA_TEST );
            StdErr << "OpenGL ES doesn't support alpha testing" << nl;
         }
         else
         {
            //glDisable( GL_ALPHA_TEST );
         }
         break;
      default:
         // Perform full state update.

         // Blending.
         if( state.alphaBlending() )
         {
            glEnable( GL_BLEND );
         }
         else
         {
            glDisable( GL_BLEND );
         }
         glBlendFunc( toGL( state.alphaBlendSrc() ), toGL( state.alphaBlendDst() ) );

         // Testing.
         if( state.alphaTesting() )
         {
            //glEnable( GL_ALPHA_TEST );
            StdErr << "OpenGL ES doesn't support alpha testing" << nl;
         }
         else
         {
            //glDisable( GL_ALPHA_TEST );
         }
         // FIXME glAlphaFunc( toGL( state.alphaTestFunc() ), state.alphaTestRef() );
         break;
   }

   // Update current state.
   curState = state;
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const ColorState& state, ColorState& curState )
{
   if( state != curState )
   {
      if( state.colorWriting() )
      {
         glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
      }
      else
      {
         glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const CullState& state, CullState& curState )
{
   if( state != curState )
   {
      switch( state.side() )
      {
         case FACE_NONE:            glDisable( GL_CULL_FACE );                                  break;
         case FACE_FRONT:           glEnable( GL_CULL_FACE ); glCullFace( GL_FRONT );           break;
         case FACE_BACK:            glEnable( GL_CULL_FACE ); glCullFace( GL_BACK );            break;
         case FACE_FRONT_AND_BACK:  glEnable( GL_CULL_FACE ); glCullFace( GL_FRONT_AND_BACK );  break;
         default:                   glEnable( GL_CULL_FACE ); glCullFace( GL_BACK );            break;
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const DepthState& state, DepthState& curState )
{
   if( state != curState )
   {
      if( state.depthTesting() )
      {
         glEnable( GL_DEPTH_TEST );
      }
      else
      {
         glDisable( GL_DEPTH_TEST );
      }
      glDepthFunc( toGL( state.depthTestFunc() ) );
      glDepthMask( state.depthWriting() ? GL_TRUE : GL_FALSE );
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const OffsetState& state, OffsetState& curState )
{
   if( state != curState )
   {
      if( state.factor() != 0.0f || state.constant() != 0.0f )
      {
         glEnable( GL_POLYGON_OFFSET_FILL );
         glPolygonOffset( state.factor(), state.constant() );
      }
      else
      {
         glDisable( GL_POLYGON_OFFSET_FILL );
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState( const StencilState& state, StencilState& curState )
{
   if( state != curState )
   {
      if( state.stencilTesting() )
      {
         glEnable( GL_STENCIL_TEST );
      }
      else
      {
         glDisable( GL_STENCIL_TEST );
      }
      glStencilFunc(
         toGL( state.stencilTestFunc() ),
         state.stencilTestRef(),
         state.stencilTestRefMask()
      );

      glStencilOp(
         toGL( state.stencilFailOp() ),
         toGL( state.stencilPassDepthFailOp() ),
         toGL( state.stencilPassDepthPassOp() )
      );
      glStencilMask( state.stencilTestWriteMask() );

      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
float identityMatrix[16] =
{
   1.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f
};


AlphaState    alphaStateDef;    //!< Default alpha states.
ColorState    colorStateDef;    //!< Default color states.
CullState     cullStateDef;     //!< Default cull states.
DepthState    depthStateDef;    //!< Default depth states.
OffsetState   offsetStateDef;   //!< Default polygon offset states.
StencilState  stencilStateDef;  //!< Default stencil states.


/*==============================================================================
  CLASS BufferMapInfo
==============================================================================*/
class BufferMapInfo
{
public:

   /*----- methods -----*/

   BufferMapInfo( const size_t offsetInBytes, const size_t sizeInBytes );
   ~BufferMapInfo();

   inline size_t  offsetInBytes() const { return _offsetInBytes; }
   inline size_t  sizeInBytes() const { return _sizeInBytes; }

   inline       void*  data()       { return (void*)_data; }
   inline const void*  data() const { return (void*)_data; }

protected:

   /*----- data members -----*/

   size_t  _offsetInBytes;
   size_t  _sizeInBytes;
   uchar*  _data;

}; //class BufferMapInfo

//------------------------------------------------------------------------------
//!
BufferMapInfo::BufferMapInfo( const size_t offsetInBytes, const size_t sizeInBytes ):
   _offsetInBytes( offsetInBytes ), _sizeInBytes( sizeInBytes )
{
   _data = new uchar[sizeInBytes];
}

//------------------------------------------------------------------------------
//!
BufferMapInfo::~BufferMapInfo()
{
   delete [] _data;
}


//OpenGL ES specific subclasses of Gfx
/*==============================================================================
   CLASS OpenGLIndexBuffer
==============================================================================*/

class OpenGLIndexBuffer:
   public IndexBuffer
{
public:

   /*----- methods -----*/
   OpenGLIndexBuffer( const IndexFormat format, const BufferFlags flags );

   virtual ~OpenGLIndexBuffer();

   inline GLenum  usage() const
   {
      return isStreamable() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
   }

   /*----- data members -----*/
   GLuint          _id;  //!< A unique identifier
   BufferMapInfo*  _mapInfo;

};  //class OpenGLIndexBuffer

//------------------------------------------------------------------------------
//!
OpenGLIndexBuffer::OpenGLIndexBuffer
( const IndexFormat format, const BufferFlags flags ):
   IndexBuffer(format, flags),
   _mapInfo(nullptr)
{
   glGenBuffers(1, &_id);
}

//------------------------------------------------------------------------------
//!
OpenGLIndexBuffer::~OpenGLIndexBuffer
( void )
{
   CHECK( _mapInfo == nullptr );
   glDeleteBuffers(1, &_id);  //should silently ignore invalid IDs
}


/*==============================================================================
   CLASS OpenGLVertexBuffer
==============================================================================*/

class OpenGLVertexBuffer:
   public VertexBuffer
{
public:

   /*----- static methods -----*/


   static void disable( uint attributes )
   {
      if( attributes & ATTRIB_TYPE_POSITION )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_POSITION) );
      }
      if( attributes & ATTRIB_TYPE_COLOR )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_COLOR) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD0 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD0) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD1 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD1) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD2 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD2) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD3 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD3) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD4 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD4) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD5 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD5) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD6 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD6) );
      }
      if( attributes & ATTRIB_TYPE_TEXCOORD7 )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TEXCOORD7) );
      }
      if( attributes & ATTRIB_TYPE_NORMAL )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_NORMAL) );
      }
      if( attributes & ATTRIB_TYPE_TANGENT )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_TANGENT) );
      }
      if( attributes & ATTRIB_TYPE_BINORMAL )
      {
         glDisableVertexAttribArray( toGL(ATTRIB_TYPE_BINORMAL) );
      }
   }


   /*----- methods -----*/

   OpenGLVertexBuffer( const BufferFlags flags ):
      VertexBuffer(flags),
      _mapInfo(nullptr)
   {
      glGenBuffers(1, &_id);
   }

   virtual ~OpenGLVertexBuffer()
   {
      CHECK(_mapInfo == nullptr);
      glDeleteBuffers(1, &_id);  //should silently ignore invalid IDs
   }

   uint activate() const
   {
      glBindBuffer( GL_ARRAY_BUFFER, _id );

      uint attributes = 0;
      AttributesContainer::ConstIterator curAttrib = _attributes.begin();
      AttributesContainer::ConstIterator endAttrib = _attributes.end();
      for( ; curAttrib != endAttrib; ++curAttrib )
      {
         uint elemCount;
         GLenum elemType;
         toGL( (*curAttrib)->_format, elemCount, elemType );
         attributes |= (*curAttrib)->_type;
         GLuint aid = toGL((*curAttrib)->_type);
         glEnableVertexAttribArray( aid );
         glVertexAttribPointer( aid, elemCount, elemType, true, GLsizei(_strideInBytes), BUFFER_OFFSET( (*curAttrib)->_offset ) );
      }
      return attributes;
   }

   inline GLenum  usage() const
   {
      return isStreamable() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
   }

   /*----- data members -----*/
   GLuint          _id;  //!< A unique identifier
   BufferMapInfo*  _mapInfo;

};  //class OpenGLVertexBuffer


/*==============================================================================
  CLASS OpenGLGeometry
==============================================================================*/
class OpenGLGeometry:
   public Gfx::Geometry
{
public:

   /*----- methods -----*/

   OpenGLGeometry( const PrimitiveType type ):
      Geometry( type )
   {
      glGenVertexArraysOES(1, &_id);
   }

   virtual ~OpenGLGeometry()
   {
      glDeleteVertexArraysOES(1, &_id);
   }

   uint  getRevision() const
   {
      uint rev = 0;
      for( auto cur = _buffers.begin(); cur != _buffers.end(); ++cur )
      {
         rev += (*cur)->revision();
      }
      return rev;
   }

   void  activate() const
   {
      glBindVertexArrayOES( _id );
      uint rev = getRevision();
      if( rev != _revision )
      {
         // Refresh the VAO.
         uint attributes = 0;
         BufferContainer::const_iterator curBuf = buffers().begin();
         BufferContainer::const_iterator endBuf = buffers().end();
         for( ; curBuf != endBuf; ++curBuf )
         {
            const OpenGLVertexBuffer* oglBuffer = (const OpenGLVertexBuffer*)(*curBuf).ptr();
            attributes |= oglBuffer->activate();
         }

         // Disable inactive attributes.
         uint remAttributes = Gfx::ATTRIB_TYPE_ALL & (~attributes);
         OpenGLVertexBuffer::disable( remAttributes );

         // Bind the index buffer to the VAO.
         const OpenGLIndexBuffer* indices = (const OpenGLIndexBuffer*)(indexBuffer().ptr());
         if( indices ) glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indices->_id );
         else          glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

         // Mark with the computed revision.
         _revision = rev;
      }
   }

protected:

   /*----- data members -----*/

   GLuint  _id;

}; //class OpenGLGeometry


/*==============================================================================
   CLASS OpenGLFramebuffer
==============================================================================*/

class OpenGLFramebuffer:
   public Gfx::Framebuffer
{
public:

   /*----- methods -----*/
   OpenGLFramebuffer()
   {
      glGenFramebuffers(1, &_id);
   }

   virtual ~OpenGLFramebuffer()
   {
      glDeleteFramebuffers(1, &_id);
   }

   /*----- data members -----*/
   GLuint  _id;

};  //class OpenGLFramebuffer


/*==============================================================================
   CLASS OpenGLShader
==============================================================================*/

String vs_str_default =
"uniform mat4 gfxProjectionMatrix;\n"
"uniform mat4 gfxViewMatrix;\n"
"uniform mat4 gfxWorldMatrix;\n"
"uniform mat4 gfxWorldViewMatrix;\n"
"uniform mat4 gfxWorldViewProjectionMatrix;\n"
"uniform vec3 gfxCameraPosition;\n"
"attribute vec4 gfxVertex;\n"
"attribute vec4 gfxColor;\n"
"attribute vec4 gfxMultiTexCoord0;\n"
"attribute vec4 gfxMultiTexCoord1;\n"
"attribute vec4 gfxMultiTexCoord2;\n"
"attribute vec4 gfxMultiTexCoord3;\n"
"attribute vec4 gfxMultiTexCoord4;\n"
"attribute vec4 gfxMultiTexCoord5;\n"
"attribute vec4 gfxMultiTexCoord6;\n"
"attribute vec4 gfxMultiTexCoord7;\n"
"attribute vec3 gfxNormal;\n"
"attribute vec3 gfxTangent;\n"
"attribute vec3 gfxBinormal;\n";

String ps_str_default =
//"precision highp float;\n"
"precision mediump float;\n"
"uniform highp vec3 gfxCameraPosition;\n"
"uniform highp mat4 gfxWorldViewMatrix;\n"
"uniform highp mat4 gfxWorldMatrix;\n";

class OpenGLShader:
   public Shader
{

public:

   /*----- methods -----*/

   // Only Managers can create this object.
   OpenGLShader( ShaderType type, const String& code ):
      Shader( type )
   {
      // Compile shader with attributes as header.
      GLint size[2]      = { GLint(vs_str_default.size()), GLint(code.size()) };
      const char* str[2] = { vs_str_default.cstr(), code.cstr() };

      if( type == VERTEX_SHADER )
      {
         size[0] = GLint(vs_str_default.size());
         str[0]  = vs_str_default.cstr();
         _id     = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( _id, 2, str, size );
      }
      else if( type == FRAGMENT_SHADER )
      {
         size[0] = GLint(ps_str_default.size());
         str[0]  = ps_str_default.cstr();
         _id     = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( _id, 2, str, size );
      }
      else
      {
         //GEOMETRY
      }

      glCompileShader( _id );

      // Check for errors.
      GLint status;
      glGetShaderiv( _id, GL_COMPILE_STATUS, &status );

      if( status != 1 )
      {
         GLchar error[2048];
         glGetShaderInfoLog( _id, sizeof(error)-1, 0, error );
         printf( "error compiling: %s\n", error );
      }
   }

   virtual ~OpenGLShader()
   {
      glDeleteShader( _id );
   }

   /*----- data members -----*/
   GLuint  _id;  //!< A unique identifier

};


/*==============================================================================
   CLASS OpenGLProgram
==============================================================================*/

class OpenGLProgram:
   public Program
{

public:

   /*----- methods -----*/

   OpenGLProgram() :
      _dirty( true )
   {
      _id = glCreateProgram();
   }

   virtual ~OpenGLProgram()
   {
      glDeleteProgram( _id );
   }

   virtual void addShader( const RCP<Shader>& shader )
   {
      Program::addShader( shader );
      _dirty = true;
      OpenGLShader* oglShader = (OpenGLShader*)shader.ptr();
      glAttachShader( _id, oglShader->_id );
   }

   virtual bool removeShader( const RCP<Shader>& shader )
   {
      if( Program::removeShader( shader ) )
      {
         _dirty = true;
         OpenGLShader* oglShader = (OpenGLShader*)shader.ptr();
         glDetachShader( _id, oglShader->_id );
         return true;
      }
      return false;
   }

   inline void compile() const
   {
      CHECK( !_shaders.empty() );
      if( _dirty )
      {
         _dirty = false;
         // Set default attributes.
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_POSITION ), "gfxVertex"         );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_COLOR    ), "gfxColor"          );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD0), "gfxMultiTexCoord0" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD1), "gfxMultiTexCoord1" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD2), "gfxMultiTexCoord2" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD3), "gfxMultiTexCoord3" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD4), "gfxMultiTexCoord4" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD5), "gfxMultiTexCoord5" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD6), "gfxMultiTexCoord6" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TEXCOORD7), "gfxMultiTexCoord7" );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_NORMAL   ), "gfxNormal"         );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_TANGENT  ), "gfxTangent"        );
         glBindAttribLocation( _id, toGL(ATTRIB_TYPE_BINORMAL ), "gfxBinormal"       );

         // Link.
         glLinkProgram( _id );

         // Check for errors.
         GLint status;
         glGetProgramiv( _id, GL_LINK_STATUS, &status );
         if( status != 1 )
         {
            GLchar error[2048];
            glGetShaderInfoLog( _id, sizeof(error)-1, 0, error );
            printf( "error linking: %s\n", error );
            CHECK( false );
         }

         // Find uniforms location.
         computeUniformLocation();
      }
   }

   void computeUniformLocation() const
   {
      _uniforms.clear();

      // Find uniform matrices location.
      _projLoc   = glGetUniformLocation( _id, "gfxProjectionMatrix" );
      _viewLoc   = glGetUniformLocation( _id, "gfxViewMatrix" );
      _worldLoc  = glGetUniformLocation( _id, "gfxWorldMatrix" );
      _wvLoc     = glGetUniformLocation( _id, "gfxWorldViewMatrix" );
      _wvpLoc    = glGetUniformLocation( _id, "gfxWorldViewProjectionMatrix" );
      _camPosLoc = glGetUniformLocation( _id, "gfxCameraPosition" );

      // Find out how many uniforms there are.
      GLint numUniforms = 0;
      glGetProgramiv( _id, GL_ACTIVE_UNIFORMS, &numUniforms );

      // Loop over all uniforms, keeping only the relevant ones.
      GLchar uniformName[256];
      GLint arraySize;
      GLenum uniformType;

      for( GLint i = 0; i < numUniforms; ++i )
      {
         // Retrieve the uniform.
         glGetActiveUniform( _id, i, (GLsizei)256, 0, &arraySize, &uniformType, uniformName );

         // Ignore irrelevant uniforms, and ones starting with "gfx".
         String constName( uniformName );
         if( strncmp( uniformName, "gfx", 3 ) != 0 )
         {
            // OpenGL names uniform arrays 'myVarName[0]'.
            // Keep the name intact (with trailing '[0]'), since Constants::getContant() support it.
            // The ATI driver erroneously reports N uniforms for 'myVar[N]', so only care about the first one.
            String::SizeType s = constName.find('[');
            if( s != String::npos )
            {
               if( constName[s+1] == '0' )
               {
                  // Need to add both with and without the trailing [0].
                  int loc = glGetUniformLocation( _id, uniformName );
                  _uniforms[constName.cstr()] = loc;
                  _uniforms[constName.sub(0, s).cstr()] = loc;
               }
            }
            else
            {
               // Add the name to the hashmap.
               _uniforms[constName.cstr()] = glGetUniformLocation( _id, uniformName );
            }
         }
      }
   }


   inline void activate( GLuint& id ) const
   {
      compile(); //only done if dirty
      if( id != _id )
      {
         glUseProgram( _id );
         id = _id;
      }
   }

   inline int getSampler( const ConstString& name ) const
   {
      Map<ConstString,int>::Iterator it = _uniforms.find( name );
      return it != _uniforms.end() ? (*it).second : -1;
   }

   inline int getConstant( const ConstString& name ) const
   {
      Map<ConstString,int>::Iterator it = _uniforms.find( name );
      return it != _uniforms.end() ? (*it).second : -1;
   }

   /*----- data members -----*/

   GLuint                       _id;  //!< A unique identifier
   mutable GLint                _projLoc;
   mutable GLint                _viewLoc;
   mutable GLint                _worldLoc;
   mutable GLint                _wvLoc;
   mutable GLint                _wvpLoc;
   mutable GLint                _camPosLoc;
   mutable bool                 _dirty;
   mutable Map<ConstString,int> _uniforms;
};


/*==============================================================================
   CLASS OpenGLTexture
==============================================================================*/

class OpenGLTexture:
   public Gfx::Texture
{
public:

   /*----- methods -----*/
   OpenGLTexture( GLenum target );

   virtual ~OpenGLTexture();

   bool  created( const uint level = 0 ) const { return getbits(_levelCreated, level, 1) & 0x01; }
   void  setCreated( const uint level = 0 ) { _levelCreated = setbits(_levelCreated, level, 1, 0x1); }
   void  unsetCreated( const uint level = 0 ) { _levelCreated = setbits(_levelCreated, level, 1, 0x0); }

   /*----- data members -----*/
   GLuint        _id;  //!< A unique identifier
   GLenum        _oglType;
   uint          _levelCreated;  //!< One dirty bit per level to indicate whether or not the level was initialized
   mutable Gfx::TextureState  _texState;

};  //class OpenGLTexture

//------------------------------------------------------------------------------
//!
OpenGLTexture::OpenGLTexture( GLenum target ) :
   _oglType( target ),
   _levelCreated( 0 )
{
   glGenTextures(1, &_id);
   _texState.setInvalidFields();
}

//------------------------------------------------------------------------------
//!
OpenGLTexture::~OpenGLTexture
( void )
{
   glDeleteTextures(1, &_id);  //should silently ignore invalid IDs
}

//------------------------------------------------------------------------------
//!
void initGLES()
{
   DBG_BLOCK( os_gl, "GLES2Manager init()" );

   checkForErrors( "GLES2Manager before initGLES() - %s\n");

   createTextureFormatTable();
   initFuncs();
   initStates();
   initDefines( vs_str_default, ps_str_default );

   checkForErrors( "GLES2Manager initGLES() - %s\n");
}

//------------------------------------------------------------------------------
//!
void termGLES()
{
   DBG_BLOCK( os_gl, "GLES2Manager terminate()" );
}

UNNAMESPACE_END


/*==============================================================================
   CLASS GLES2Manager::Cache
==============================================================================*/

class GLES2Manager::Cache
{
public:

   /*----- methods -----*/

   Cache();
   ~Cache();

   void  invalidate();

   /*----- data members -----*/

   AlphaState    _alphaState;    //!< Current alpha states.
   ColorState    _colorState;    //!< Current color states.
   CullState     _cullState;     //!< Current cull states.
   DepthState    _depthState;    //!< Current depth states.
   OffsetState   _offsetState;   //!< Current offset states.
   StencilState  _stencilState;  //!< Current stencil states.
   GLuint        _prog;          //!< Current program id.
};

//------------------------------------------------------------------------------
//!
GLES2Manager::Cache::Cache()
{
   invalidate();
}

//------------------------------------------------------------------------------
//!
GLES2Manager::Cache::~Cache()
{
}

//------------------------------------------------------------------------------
//!
void
GLES2Manager::Cache::invalidate()
{
   _alphaState.setInvalidFields();
   _colorState.setInvalidFields();
   _cullState.setInvalidFields();
   _depthState.setInvalidFields();
   _offsetState.setInvalidFields();
   _stencilState.setInvalidFields();
   _prog = 0;
}

/*==============================================================================
   CLASS GLES2Manager
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLES2Manager::GLES2Manager( GLESContext* context ):
   Manager( context, "OpenGL ES 2" ),
   _context( context ), _curTexUnitsUsed(0)
{
   DBG_BLOCK( os_gl, "Creating GLES2Manager" );

   _context->makeCurrent();
   initGLES();

   _cache = new Cache();
   //printInfo( StdErr );
}

//------------------------------------------------------------------------------
//!
GLES2Manager::~GLES2Manager()
{
   DBG_BLOCK( os_gl, "Destroying GLES2Manager" );
   delete _cache;
   _oneToOneGeom = nullptr;  //required because otherwise, glDeleteBuffersARB will fail (OpenGL scoped out)
   termGLES();
}

//------------------------------------------------------------------------------
//!
void
GLES2Manager::printInfo( TextStream& os ) const
{
   Manager::printInfo( os );

   const GLubyte* str = glGetString( GL_VENDOR );
   os << "VENDOR: " << (const char*)str << nl;

   str = glGetString( GL_RENDERER );
   os << "RENDERER: " << (const char*)str << nl;

   str = glGetString( GL_VERSION );
   os << "VERSION: " << (const char*)str << nl;

   str = glGetString( GL_EXTENSIONS );
   os << "EXTENSIONS: " << (const char*)str << nl;
}

//------------------------------------------------------------------------------
//!
void
GLES2Manager::setSize( uint width, uint height )
{
   Manager::setSize(width, height);
   _context->updateSize(); //for AGL to resize properly
}


//------------------------------------------------------------------------------
//!
void
GLES2Manager::display()
{
   _context->swapBuffers();
}

//------------------------------------------------------------------------------
//!
float
GLES2Manager::oneToOneOffset()
{
   return 0.0f;
}

//------------------------------------------------------------------------------
//! Screen grab.
bool
GLES2Manager::screenGrab( uint x, uint y, uint w, uint h, void* data )
{
   glReadPixels( x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data );
   return true;
}

//------------------------------------------------------------------------------
//!
RCP<IndexBuffer>
GLES2Manager::createBuffer(
   const IndexFormat  format,
   const BufferFlags  flags,
   const size_t       bufferSizeInBytes,
   const void*        data
)
{
   OpenGLIndexBuffer* ogl_buffer = new OpenGLIndexBuffer( format, flags );
   RCP<IndexBuffer> buffer( ogl_buffer );
   setData( buffer, bufferSizeInBytes, data );
   return buffer;
}

//------------------------------------------------------------------------------
//!
RCP<VertexBuffer>
GLES2Manager::createBuffer(
   const BufferFlags  flags,
   const size_t       bufferSizeInBytes,
   const void*        data
)
{
   OpenGLVertexBuffer* ogl_buffer = new OpenGLVertexBuffer( flags );
   RCP<VertexBuffer> buffer( ogl_buffer );
   setData( buffer, bufferSizeInBytes, data );
   return buffer;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<IndexBuffer>&  buffer,
   const size_t             sizeInBytes,
   const void*              data
)
{
   // Is there any data to copy?
   //if( !data )
   //{
   //   return false;
   //}

   OpenGLIndexBuffer* ogl_buffer = (OpenGLIndexBuffer*)buffer.ptr();
   buffer->_sizeInBytes = sizeInBytes;

   // id was already created, so just bind and send the data.
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_id ) ;
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(buffer->_sizeInBytes), data, ogl_buffer->usage() );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<VertexBuffer>&  buffer,
   const size_t              sizeInBytes,
   const void*               data
)
{
   // Is there any data to copy?
   //if( !data )
   //{
   //   return false;
   //}

   OpenGLVertexBuffer* ogl_buffer = (OpenGLVertexBuffer*)buffer.ptr();
   buffer->_sizeInBytes = sizeInBytes;
   // id was already created, so just bind and send the data.
   glBindBuffer( GL_ARRAY_BUFFER, ogl_buffer->_id );
   glBufferData( GL_ARRAY_BUFFER, GLsizeiptr(buffer->_sizeInBytes), data, ogl_buffer->usage() );
   return true;
}

//------------------------------------------------------------------------------
//!
void*
GLES2Manager::map( const RCP<IndexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes, const size_t sizeInBytes )
{
   OpenGLIndexBuffer* ogl_buffer = (OpenGLIndexBuffer*)buffer.ptr();
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_id );
   if( offsetInBytes == 0 && (sizeInBytes == 0 || sizeInBytes == ogl_buffer->sizeInBytes()) )
   {
      // Map the whole buffer.
      // According to http://developer.nvidia.com/object/using_VBOs.html, invalidating
      // the old data can yield better performance.
      // FIXME still necessary? glBufferData( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_sizeInBytes, nullptr, ogl_buffer->usage() );
      void* ptr = gl_MapBufferOES( GL_ELEMENT_ARRAY_BUFFER, toGL(mode) );
      checkForErrors("GLES2Manager::map(IndexBuffer) - %s\n");
      return ptr;
   }
   else
   {
      // Allocate a temporary buffer, and use glBufferSubData on unmap.
      CHECK( ogl_buffer->_mapInfo == nullptr );
      ogl_buffer->_mapInfo = new BufferMapInfo( offsetInBytes, sizeInBytes );
      if( (mode & MAP_READ) != 0x0 )
      {
         // Need to copy data from GPU.
         //glGetBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offsetInBytes, sizeInBytes, ogl_buffer->_mapInfo->data() );
         assert( 0 );
      }
      checkForErrors("GLES2Manager::map(IndexBuffer w/mapInfo) - %s\n");
      return ogl_buffer->_mapInfo->data();
   }
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::unmap( const RCP<IndexBuffer>& buffer )
{
   OpenGLIndexBuffer* ogl_buffer = (OpenGLIndexBuffer*)buffer.ptr();
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ogl_buffer->_id );
   bool ok;
   if( ogl_buffer->_mapInfo == nullptr )
   {
      ok = (gl_UnmapBufferOES( GL_ELEMENT_ARRAY_BUFFER ) == GL_TRUE);
      ok &= !checkForErrors("GLES2Manager::unmap(IndexBuffer) - %s\n");
   }
   else
   {
      glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, GLintptr(ogl_buffer->_mapInfo->offsetInBytes()), GLsizeiptr(ogl_buffer->_mapInfo->sizeInBytes()), ogl_buffer->_mapInfo->data() );
      delete ogl_buffer->_mapInfo;
      ogl_buffer->_mapInfo = nullptr;
      ok = !checkForErrors("GLES2Manager::unmap(IndexBuffer w/mapInfo) - %s\n");
   }
   return ok;

}

//------------------------------------------------------------------------------
//!
void*
GLES2Manager::map( const RCP<VertexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes, const size_t sizeInBytes )
{
   OpenGLVertexBuffer* ogl_buffer = (OpenGLVertexBuffer*)buffer.ptr();
   glBindBuffer( GL_ARRAY_BUFFER, ogl_buffer->_id );
   if( offsetInBytes == 0 && (sizeInBytes == 0 || sizeInBytes == ogl_buffer->sizeInBytes()) )
   {
      // Map the whole buffer.
      // According to http://developer.nvidia.com/object/using_VBOs.html, invalidating
      // the old data can yield better performance.
      glBufferData( GL_ARRAY_BUFFER, GLsizeiptr(ogl_buffer->_sizeInBytes), nullptr, ogl_buffer->usage() );
      void* ptr = gl_MapBufferOES( GL_ARRAY_BUFFER, toGL(mode) );
      checkForErrors("GLES2Manager::map(VertexBuffer) - %s\n");
      return ptr;
   }
   else
   {
      // Allocate a temporary buffer, and use glBufferSubData on unmap.
      CHECK( ogl_buffer->_mapInfo == nullptr );
      ogl_buffer->_mapInfo = new BufferMapInfo( offsetInBytes, sizeInBytes );
      if( (mode & MAP_READ) != 0x0 )
      {
         // Need to copy data from GPU.
         glBufferSubData( GL_ARRAY_BUFFER, GLintptr(offsetInBytes), GLsizeiptr(sizeInBytes), ogl_buffer->_mapInfo->data() );
      }
      checkForErrors("GLES2Manager::map(VertexBuffer w/mapInfo) - %s\n");
      return ogl_buffer->_mapInfo->data();
   }
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::unmap( const RCP<VertexBuffer>& buffer )
{
   OpenGLVertexBuffer* ogl_buffer = (OpenGLVertexBuffer*)buffer.ptr();
   glBindBuffer( GL_ARRAY_BUFFER, ogl_buffer->_id );
   bool ok;
   if( ogl_buffer->_mapInfo == nullptr )
   {
      ok = (gl_UnmapBufferOES( GL_ARRAY_BUFFER ) == GL_TRUE);
      ok &= !checkForErrors("GLES2Manager::unmap(VertexBuffer) - %s\n");
   }
   else
   {
      glBufferSubData( GL_ARRAY_BUFFER, GLintptr(ogl_buffer->_mapInfo->offsetInBytes()), GLsizeiptr(ogl_buffer->_mapInfo->sizeInBytes()), ogl_buffer->_mapInfo->data() );
      delete ogl_buffer->_mapInfo;
      ogl_buffer->_mapInfo = nullptr;
      ok = !checkForErrors("GLES2Manager::unmap(VertexBuffer w/mapInfo) - %s\n");
   }
   return ok;
}


//------------------------------------------------------------------------------
//!
RCP<Framebuffer>
GLES2Manager::createFramebuffer()
{
   OpenGLFramebuffer* ogl_fb = new OpenGLFramebuffer();
   return ogl_fb;
}

//------------------------------------------------------------------------------
//!
RCP<Geometry>
GLES2Manager::createGeometry( const PrimitiveType pt )
{
   return RCP<Geometry>( new OpenGLGeometry( pt ) );
}

//------------------------------------------------------------------------------
//!
RCP<Shader>
GLES2Manager::createShader( ShaderType type, const String& code )
{
   return RCP<Shader>( new OpenGLShader( type, code ) );
}

//------------------------------------------------------------------------------
//!
RCP<Program>
GLES2Manager::createProgram()
{
   return RCP<Program>( new OpenGLProgram() );
}

//------------------------------------------------------------------------------
//!
size_t
GLES2Manager::getConstants(
   const RCP<Program>&          program,
   ConstantBuffer::Container&   constants,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   DBG_BLOCK( os_gl, "GLES2Manager::getConstants" );

   const OpenGLProgram* oglProg = (const OpenGLProgram*)program.ptr();

   // Make sure the program is compiled before trying to access the uniform locations.
   oglProg->compile();

   // Find out how many uniforms there are.
   GLint numUniforms = 0;
   glGetProgramiv( oglProg->_id, GL_ACTIVE_UNIFORMS, &numUniforms );
   DBG_MSG( os_gl, "OpenGLProgram #" << oglProg->_id << " has " << (int)numUniforms << " uniforms" );

   // Clear ignore groups.
   if( usedIgnoreGroup )
   {
      CHECK( ignoreGroups != nullptr );
      // Align to chunks of 32 (if uint*) or 8 (if uchar*).
      uint32_t nBits = alignTo( (uint32_t)ignoreGroups->size(), (uint32_t)8*sizeof(*usedIgnoreGroup) );
      setbits( usedIgnoreGroup, 0, nBits, 0 );
   }

   // Loop over all uniforms, keeping only the relevant ones.
   GLchar uniformName[256];
   GLint arraySize;
   GLenum uniformType;

   const ConstantType invalid = (Gfx::ConstantType)-1;
   ConstantType constType;
   size_t currentOffset = 0;

   for( GLint i = 0; i < numUniforms; ++i )
   {
      // Retrieve the uniform.
      glGetActiveUniform( oglProg->_id, i, (GLsizei)256, 0, &arraySize, &uniformType, uniformName );
      DBG_MSG( os_gl, "[" << (int)i << "] " << uniformName );

      // Set constType only on the relevant ones.
      switch( uniformType )
      {
         case GL_FLOAT     : constType = Gfx::CONST_FLOAT;  break;
         case GL_FLOAT_VEC2: constType = Gfx::CONST_FLOAT2; break;
         case GL_FLOAT_VEC3: constType = Gfx::CONST_FLOAT3; break;
         case GL_FLOAT_VEC4: constType = Gfx::CONST_FLOAT4; break;
         case GL_FLOAT_MAT2: constType = Gfx::CONST_MAT2;   break;
         case GL_FLOAT_MAT3: constType = Gfx::CONST_MAT3;   break;
         case GL_FLOAT_MAT4: constType = Gfx::CONST_MAT4;   break;

         case GL_INT:
         case GL_INT_VEC2:
         case GL_INT_VEC3:
         case GL_INT_VEC4:
         case GL_BOOL:
         case GL_BOOL_VEC2:
         case GL_BOOL_VEC3:
         case GL_BOOL_VEC4:
         //case GL_FLOAT_MAT2x3:
         //case GL_FLOAT_MAT2x4:
         //case GL_FLOAT_MAT3x2:
         //case GL_FLOAT_MAT3x4:
         //case GL_FLOAT_MAT4x2:
         //case GL_FLOAT_MAT4x3:
            printf( "Unsupported uniform type for '%s'\n", uniformName );
            constType = invalid;
            break;

         //case GL_SAMPLER_1D:
         case GL_SAMPLER_2D:
         case GL_SAMPLER_3D_OES:
         case GL_SAMPLER_CUBE:
         //case GL_SAMPLER_1D_SHADOW:
         //case GL_SAMPLER_2D_SHADOW:
            // Silently ignore samplers.
            DBG_MSG( os_gl, "Ignoring '" << uniformName << "'" );
            constType = invalid;
            break;

         default:
            printf( "Unknown uniform type for '%s'\n", uniformName );
            constType = invalid;
            break;
      } //switch( uniformType )

      // Ignore irrelevant uniforms, and ones starting with "gfx"
      String constName( uniformName );

      if( strncmp( uniformName, "gfx", 3 ) == 0 )
      {
         constType = invalid;
      }
      else if( ignoreGroups )
      {
         // OpenGL can name uniform arrays 'myVarName[0]', so compare without the trailing brackets.
         String constNameClean( constName );
         String::SizeType s = constNameClean.find('[');
         if( s != String::npos )
         {
            constNameClean.erase( s, constName.size()-s );
         }
         for( uint32_t g = 0; g < ignoreGroups->size(); ++g )
         {
            if( (*ignoreGroups)[g].has( constNameClean ) )
            {
               constType = invalid;
               if( usedIgnoreGroup )
               {
                  setbits( usedIgnoreGroup, g, 1, 1 );
               }
            }
         }
      }

      if( constType != invalid )
      {
         // OpenGL names uniform arrays 'myVarName[0]'.
         String::SizeType s = constName.find('[');
         if( s != String::npos )
         {
            // Keep the name intact (with trailing '[0]'), since Constants::getContant() support it.
            //constName.erase( s, constName.size()-s );

            // The ATI driver erroneously reports N uniforms for 'myVar[N]', so only care about the first one.
            if( constName[s+1] != '0' ) continue;
         }
         else
         {
            // Force to 0 to indicate we store data, not pointer-to-data.
            if( arraySize == 1 ) arraySize = 0;
         }
         size_t constOffset = (arraySize == 0 ? Gfx::toBytes(constType) : sizeof(void*));

         constants.pushBack(
            Gfx::ConstantBuffer::Constant(
               constName.cstr(),
               constType,
               currentOffset,
               arraySize
            )
         );
         currentOffset += constOffset;
      }
   }

   CHECK( currentOffset == Gfx::ConstantBuffer::getSize(constants) );

   return currentOffset;
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
GLES2Manager::createConstants( const size_t size )
{
   return RCP<ConstantBuffer>( new ConstantBuffer( size ) );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
GLES2Manager::createConstants( const ConstantBuffer::Container& constants, size_t size )
{
   return RCP<ConstantBuffer>( new ConstantBuffer( constants, size ) );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
GLES2Manager::createConstants(
   const RCP<Program>&          program,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   ConstantBuffer::Container constants;
   size_t size = getConstants( program, constants, ignoreGroups, usedIgnoreGroup );
   if( !constants.empty() )
   {
      return RCP<ConstantBuffer>( new ConstantBuffer( constants, size ) );
   }
   else
   {
      return RCP<ConstantBuffer>( new ConstantBuffer(0) );
   }
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLES2Manager::create1DTexture(
   const uint            width,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_2D );
   ogl_texture->set1D( width );
   ogl_texture->format( tfmt );
   ogl_texture->channelOrder( chOrder );
   ogl_texture->flags( flags );
   RCP<Texture> texture( ogl_texture );
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLES2Manager::create2DTexture(
   const uint            width,
   const uint            height,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_2D );
   ogl_texture->set2D( width, height );
   ogl_texture->format( tfmt );
   ogl_texture->channelOrder( chOrder );
   ogl_texture->flags( flags );

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(tfmt, internalFormat, format, type);
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      DBG_MSG( os_gl, "Invalid format: " << (int)internalFormat << ", " << (int)format << ", " << (int)type );
      //Invalid format, OpenGL will choke
      return nullptr;
   }

#if 0 // Still needed?
   if( internalFormat == GL_RGBA16F_ARB && isRenderable(flags) )
   {
      // The ATI OpenGL driver errors out when binding a GL_HALF_FLOAT_ARB texture, even with a nullptr data pointer.
      type = GL_UNSIGNED_BYTE;
   }
#endif

   // Call glTexImage2D in order to bind the size+format for the driver
   glBindTexture( GL_TEXTURE_2D, ogl_texture->_id );

   // The NVIDIA driver requires to call glGenerateMipmapEXT to allocate storage *BEFORE* binding the FBO
   // while OSX won't generate the mipmap chain if min filter is GL_NEAREST
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ogl_texture->isMipmapped()?GL_NEAREST_MIPMAP_NEAREST:GL_NEAREST );

   // OSX won't bind an NP2 texture if the wrap policy won't allow it
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   // Depth texture.
   if( chOrder == TEX_CHANS_Z )
   {
      internalFormat = GL_DEPTH_COMPONENT;
      format         = GL_DEPTH_COMPONENT;
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT );
   }

   // Set depth comparison.
   if( tfmt == TEX_FMT_24_8 )
   {
      // FIXME glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB );
   }


   // Create the texture (without any data)
   glTexImage2D(
      GL_TEXTURE_2D,
      0,  //level
      internalFormat,
      width,
      height,
      0, //border
      format,
      type,
      nullptr
   );

   DBG_MSG( os_gl, "Created OGL texture id=" << ogl_texture->_id );

   if( ogl_texture->isMipmapped() )
   {
      DBG_MSG( os_gl, "Generating storage for mipmap chain" );
      glGenerateMipmap( ogl_texture->_oglType );
   }

   RCP<Texture> texture( ogl_texture );
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLES2Manager::create3DTexture(
   const uint            width,
   const uint            height,
   const uint            depth,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_3D_OES );
   ogl_texture->set3D(width, height, depth);
   ogl_texture->format(tfmt);
   ogl_texture->channelOrder(chOrder);
   ogl_texture->flags(flags);
   RCP<Texture> texture(ogl_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
GLES2Manager::createCubeTexture(
   const uint            edgeLength,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   OpenGLTexture* ogl_texture = new OpenGLTexture( GL_TEXTURE_CUBE_MAP );
   ogl_texture->setCubemap(edgeLength);
   ogl_texture->format(tfmt);
   ogl_texture->channelOrder(chOrder);
   ogl_texture->flags(flags);
   RCP<Texture> texture(ogl_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         glTexImage2D(GL_TEXTURE_2D,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      default:
      {
         printf("Sorry, unsupported texture type\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          offset_x,
   const uint          width,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_1D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width = ogl_texture->levelWidth(level);
            if( offset_x != 0 ||
                width != level_width )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the nullptr data pointer)
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            level_width, 1,
                            0, //border
                            format,
                            type,
                            nullptr);
               //Copy the data to the GPU
               glTexSubImage2D(GL_TEXTURE_2D,
                               level,
                               offset_x, 0,
                               width, 1,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            width, 1,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(GL_TEXTURE_2D,
                            level,
                            offset_x, 0,
                            width, 1,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
         }
         /**
         printf("Done setting region {%d}-{%d} in a %d texture ID=%d\n",
                ogl_texture->definedOffsetX(),
                ogl_texture->definedWidth(),
                ogl_texture->levelWidth(level),
                ogl_texture->_id);
         printf("Defined range is +{%d}-{%d}\n",
                ogl_texture->definedOffsetX(),
                ogl_texture->definedWidth());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 1D)\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          offset_x,
   const uint          offset_y,
   const uint          width,
   const uint          height,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the nullptr data pointer)
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            0, //border
                            format,
                            type,
                            nullptr);
               //Copy the data to the GPU
               glTexSubImage2D(GL_TEXTURE_2D,
                               level,
                               offset_x, offset_y,
                               width, height,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            width, height,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(GL_TEXTURE_2D,
                            level,
                            offset_x, offset_y,
                            width, height,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 2D)\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          slice,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         glTexImage2D(GL_TEXTURE_2D,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      }  break;
      case TEX_TYPE_2D_ARRAY:
      {
#if 1
         CHECK( false );
         // Unsupported.
#else
         glBindTexture(GL_TEXTURE_2D_ARRAY, ogl_texture->_id);
         glTexImage3DOES(GL_TEXTURE_2D_ARRAY,
                         level,
                         internalFormat,
                         ogl_texture->levelWidth(level),
                         ogl_texture->levelHeight(level),
                         slice,
                         0, //border
                         format,
                         type,
                         data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d array texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
#endif
      }  break;
      case TEX_TYPE_CUBEMAP:
      {
         glBindTexture(GL_TEXTURE_CUBE_MAP, ogl_texture->_id);
         GLenum target = toGLCubemapTarget( slice );
         glTexImage2D(target,
                      level,
                      internalFormat,
                      ogl_texture->levelWidth(level),
                      ogl_texture->levelHeight(level),
                      0, //border
                      format,
                      type,
                      data);
         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().setRange(0, ogl_texture->width());
            ogl_texture->definedRegionY().setRange(0, ogl_texture->height());
         }
         /**
         printf("Done setting %dx%d cube texture ID=%d\n", ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      }  break;
      default:
      {
         printf("Sorry, unsupported texture type\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          slice,
   const uint          offset_x,
   const uint          offset_y,
   const uint          width,
   const uint          height,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_2D:
      {
         glBindTexture(GL_TEXTURE_2D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            0, //border
                            format,
                            type,
                            nullptr);
               //Copy the data to the GPU
               glTexSubImage2D(GL_TEXTURE_2D,
                               level,
                               offset_x, offset_y,
                               width, height,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(GL_TEXTURE_2D,
                            level,
                            internalFormat,
                            width, height,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(GL_TEXTURE_2D,
                            level,
                            offset_x, offset_y,
                            width, height,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      }  break;
      case TEX_TYPE_2D_ARRAY:
      {
#if 1
         CHECK( false );
         // Unsupported.
#else
         glBindTexture(GL_TEXTURE_2D_ARRAY, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            uint depth        = ogl_texture->depth();
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage3DOES(GL_TEXTURE_2D_ARRAY,
                               level,
                               internalFormat,
                               level_width,
                               level_height,
                               depth,
                               0, //border
                               format,
                               type,
                               nullptr);
               //Copy the data to the GPU
               glTexSubImage3DOES(GL_TEXTURE_2D_ARRAY,
                                  level,
                                  offset_x, offset_y, slice,
                                  width, height, 1,
                                  format,
                                  type,
                                  data);
            }
            else
            {
               //Copy the data to the GPU
               glTexSubImage3DOES(GL_TEXTURE_2D_ARRAY,
                                  level,
                                  offset_x, offset_y, slice,
                                  width, height, 1,
                                  format,
                                  type,
                                  data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage3DOES(GL_TEXTURE_2D_ARRAY,
                               level,
                               offset_x, offset_y, slice,
                               width, height, 1,
                               format,
                               type,
                               data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d array texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
#endif
      }  break;
      case TEX_TYPE_CUBEMAP:
      {
         GLuint target = toGLCubemapTarget( slice );
         glBindTexture(GL_TEXTURE_CUBE_MAP, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            if( offset_x != 0 || offset_y != 0 ||
                width != level_width || height != level_height )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the NULL data pointer)
               glTexImage2D(target,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            0, //border
                            format,
                            type,
                            NULL);
               //Copy the data to the GPU
               glTexSubImage2D(target,
                               level,
                               offset_x, offset_y,
                               width, height,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage2D(target,
                            level,
                            internalFormat,
                            width, height,
                            0, //border
                            format,
                            type,
                            data);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage2D(target,
                            level,
                            offset_x, offset_y,
                            width, height,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
         }
         /**
         printf("Done setting region {%d,%d}-{%dx%d} in a %dx%d cube texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d}-{%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight());
         **/
      }  break;
      default:
      {
         printf("Sorry, invalid texture type (should be 2D/2Da/Cube)\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*offset_x*/,
   const uint          /*offset_y*/,
   const uint          /*offset_z*/,
   const uint          /*width*/,
   const uint          /*height*/,
   const uint          /*depth*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
#if 0
   OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();

   GLint  internalFormat;
   GLenum format;
   GLenum type;
   toGL(ogl_texture->format(), internalFormat, format, type );
   if( internalFormat == 0 || format == (GLenum)-1 || type == (GLenum)-1 )
   {
      //Invalid format, OpenGL will choke
      return false;
   }

   switch(ogl_texture->type())
   {
      case TEX_TYPE_3D:
      {
         glBindTexture(GL_TEXTURE_3D, ogl_texture->_id);
         if( !ogl_texture->created(level) )
         {
            uint level_width  = ogl_texture->levelWidth(level);
            uint level_height = ogl_texture->levelHeight(level);
            uint level_depth  = ogl_texture->levelDepth(level);
            if( offset_x != 0 || offset_y != 0 || offset_z != 0 ||
                width != level_width || height != level_height || depth != level_depth )
            {
               //Need to fill the whole texture, which is done by the driver (hence, the nullptr data pointer)
               glTexImage3D(GL_TEXTURE_3D,
                            level,
                            internalFormat,
                            level_width,
                            level_height,
                            level_depth,
                            0, //border
                            format,
                            type,
                            nullptr);
               //Copy the data to the GPU
               glTexSubImage3D(GL_TEXTURE_3D,
                               level,
                               offset_x, offset_y, offset_z,
                               width, height, depth,
                               format,
                               type,
                               data);
            }
            else
            {
               //Copy the data to the GPU
               glTexImage3D(GL_TEXTURE_3D,
                            level,
                            internalFormat,
                            width,
                            height,
                            depth,
                            0, //border
                            format,
                            type,
                            nullptr);
            }
            ogl_texture->setCreated(level);
         }
         else
         {
            //Copy the data to the GPU
            glTexSubImage3D(GL_TEXTURE_3D,
                            level,
                            offset_x, offset_y, offset_z,
                            width, height, depth,
                            format,
                            type,
                            data);
         }

         if( level == 0 && !skipDefinedRegionUpdate )
         {
            ogl_texture->definedRegionX().update(offset_x);
            ogl_texture->definedRegionX().update(width);
            ogl_texture->definedRegionY().update(offset_y);
            ogl_texture->definedRegionY().update(height);
            ogl_texture->definedRegionZ().update(offset_z);
            ogl_texture->definedRegionZ().update(depth);
         }
         /**
         printf("Done setting region {%d,%d,%d}-{%dx%dx%d} in a %dx%dx%d texture ID=%d\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(), ogl_texture->definedOffsetZ(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(), ogl_texture->gdefinedDepth(),
                ogl_texture->levelWidth(level), ogl_texture->levelHeight(level), ogl_texture->levelDepth(level),
                ogl_texture->_id);
         printf("Defined range is +{%d,%d,%d}-{%dx%dx%d}\n",
                ogl_texture->definedOffsetX(), ogl_texture->definedOffsetY(), ogl_texture->definedOffsetZ(),
                ogl_texture->definedWidth(), ogl_texture->definedHeight(), ogl_texture->definedDepth());
         **/
      } break;
      default:
      {
         printf("Sorry, invalid texture type (should be 3D)\n");
         return false;
      }
   }
   return true;
#else
   assert(false);
   return false;
#endif
}

//------------------------------------------------------------------------------
//! Generate the mipmap chain using the base map
bool
GLES2Manager::generateMipmaps( const RCP<Texture>& texture )
{
   DBG_BLOCK( os_gl, "GLES2Manager::generateMipmaps()" );
   if( texture->isMipmapped() )
   {
      DBG_BLOCK( os_gl, "Generating for " << toString(texture->type()) );
      OpenGLTexture* ogl_texture = (OpenGLTexture*)texture.ptr();
      glBindTexture( ogl_texture->_oglType, ogl_texture->_id );
      glGenerateMipmap( ogl_texture->_oglType );
      return !checkForErrors("GLES2Manager::generateMipmaps - %s\n");
   }
   else
   {
      DBG_MSG( os_gl, "ERROR - Trying to generate mipmaps on non-mipmapped texture" );
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::setFramebuffer( const Framebuffer* fb )
{
   DBG_BLOCK( os_gl, "GLES2Manager::setFramebuffer" );
   const OpenGLFramebuffer* ogl_fb = (const OpenGLFramebuffer*)fb;

   if( ogl_fb )
   {
      // Use specified buffers
      DBG_MSG( os_gl, "Binding FBO #" << ogl_fb->_id );
      glBindFramebuffer(GL_FRAMEBUFFER, ogl_fb->_id);

      // Depth buffer.
      if( ogl_fb->_depthBuffer.isValid() )
      {
         const OpenGLTexture* ogl_depth = (const OpenGLTexture*)ogl_fb->_depthBuffer.ptr();
         uint level = ogl_fb->_depthBufferLevel;

         if( ogl_fb->isDirty( Framebuffer::FB_BUFFER_BIT_DEPTH ) )
         {
            glFramebufferTexture2D(
               GL_FRAMEBUFFER,
               GL_DEPTH_ATTACHMENT,
               GL_TEXTURE_2D, ogl_depth->_id, level
            );

            ogl_fb->unsetDirty( Framebuffer::FB_BUFFER_BIT_DEPTH );
            DBG_MSG( os_gl, "Setting depth buffer from texture #" << ogl_depth->_id << " l=" << level );
         }

         _curWidth  = ogl_depth->levelWidth( level );
         _curHeight = ogl_depth->levelHeight( level );
      }

      // Stencil buffer.
      if( ogl_fb->_stencilBuffer.isValid() && ogl_fb->isDirty( Framebuffer::FB_BUFFER_BIT_STENCIL ) )
      {
         const OpenGLTexture* ogl_stencil = (const OpenGLTexture*)ogl_fb->_stencilBuffer.ptr();
         uint level = ogl_fb->_stencilBufferLevel;
         glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_STENCIL_ATTACHMENT,
            GL_TEXTURE_2D, ogl_stencil->_id, level
         );
         ogl_fb->unsetDirty( Framebuffer::FB_BUFFER_BIT_STENCIL );
         DBG_MSG( os_gl, "Setting stencil buffer from texture #" << ogl_stencil->_id << " l=" << level );
      }

      // Color buffer.
      if( ogl_fb->_colorBuffer.isValid() )
      {
         const OpenGLTexture* ogl_color = (const OpenGLTexture*)ogl_fb->_colorBuffer.ptr();
         uint level = ogl_fb->_colorBufferLevel;

         if( ogl_fb->isDirty( Framebuffer::FB_BUFFER_BIT_COLOR ) )
         {
            glFramebufferTexture2D(
               GL_FRAMEBUFFER,
               GL_COLOR_ATTACHMENT0,
               GL_TEXTURE_2D, ogl_color->_id, level
            );
            ogl_fb->unsetDirty(Framebuffer::FB_BUFFER_BIT_COLOR);
            DBG_MSG( os_gl, "Setting color buffer from texture #" << ogl_color->_id << " l=" << level );
         }
         _curWidth  = ogl_color->levelWidth( level );
         _curHeight = ogl_color->levelHeight( level );
      }
      else
      {
         glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0 );
      }
#ifdef _DEBUG
      if( !checkFramebufferStatus() )
      {
         DBG_HALT();
         return false;
      }
#endif

      _doingRTT = true;
   }
   else
   {
      DBG_MSG( os_gl, "Unbinding FBO" );
      // Use backbuffer as render target
      glBindFramebuffer( GL_FRAMEBUFFER, _context->frameBuffer() );

      _curWidth  = _width;
      _curHeight = _height;
      _doingRTT  = false;
   }

#ifdef _DEBUG
   if( checkForErrors("GLES2Manager::setFramebuffer - %s\n") )
   {
      if( ogl_fb )
      {
         StdErr << "Color format: " << (ogl_fb->_colorBuffer.isValid() ? String(ogl_fb->_colorBuffer->format()) : String("<null>")) << nl;
         StdErr << "Depth format: " << (ogl_fb->_depthBuffer.isValid() ? String(ogl_fb->_depthBuffer->format()) : String("<null>")) << nl;
      }
      else
      {
         StdErr << "<null> framebuffer" << nl;
      }
   }
#endif

   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::executeGeometry(
   const Geometry*     geom,
   const Program*      prog,
   const ConstantList* constants,
   const SamplerList*  samp,
   const float*        matrices[],
   const float*        camPosition,
   const uint*         range
)
{
   DBG_BLOCK( os_gl, "GLES2Manager::executeGeometry" );

   if( geom == nullptr )
   {
      DBG_MSG( os_gl, "Empty geometry...  Aborting." );
      return false;
   }
   if( prog == nullptr )
   {
      DBG_MSG( os_gl, "Empty program...  Aborting." );
      return false;
   }

   // Shader.
   const OpenGLProgram* oglProg = (const OpenGLProgram*)prog;
   oglProg->activate( _cache->_prog );

   // Matrices.
   if( oglProg->_projLoc != -1 )
   {
      glUniformMatrix4fv( oglProg->_projLoc, 1, GL_FALSE, matrices[0] );
   }
   if( oglProg->_viewLoc != -1 )
   {
      glUniformMatrix4fv( oglProg->_viewLoc, 1, GL_FALSE, matrices[1] );
   }
   if( oglProg->_worldLoc != -1 )
   {
      glUniformMatrix4fv( oglProg->_worldLoc, 1, GL_FALSE, matrices[2] );
   }
   if( oglProg->_wvLoc != -1 )
   {
      Mat4f wv = (*(const Mat4f*)matrices[1]) * (*(const Mat4f*)matrices[2]);
      glUniformMatrix4fv( oglProg->_wvLoc, 1, GL_FALSE, wv.ptr() );
   }
   if( oglProg->_wvpLoc != -1 )
   {
      Mat4f wvp = (*(const Mat4f*)matrices[3]) * (*(const Mat4f*)matrices[2]);
      glUniformMatrix4fv( oglProg->_wvpLoc, 1, GL_FALSE, wvp.ptr() );
   }

   // Camera.
   if( oglProg->_camPosLoc != -1 )
   {
      if( camPosition != nullptr )
      {
         glUniform3fv( oglProg->_camPosLoc, 1, camPosition );
      }
      else
      {
         printf("ERROR - Camera position required by shader, but not defined in the pass\n");
      }
   }

   // Constants.
   if( constants )
   {
      DBG_MSG( os_gl, constants->size() << " constants" );
      for( uint i = 0; i < constants->size(); ++i )
      {
         const uchar* cbuffer                       = (*constants)[i]->_buffer;
         const ConstantBuffer::Container& constDesc = (*constants)[i]->_constants;

         for( uint c = 0; c < constDesc.size(); ++c )
         {
            DBG_MSG( os_gl, "Handling constant: " << constDesc[c].name().cstr() );
            GLint loc = oglProg->getConstant( constDesc[c].name() );
            DBG_MSG( os_gl, "const[" << c << "]: '" << constDesc[c].name().cstr() << "' loc=" << (int)loc );
            if( loc == -1 )  continue;
            GLint count = (GLint)constDesc[c].count();

            const float* data = constDesc[c].count() == 0 ?
               (const float*)(cbuffer + constDesc[c].offset()) :
              *(const float**)(cbuffer + constDesc[c].offset()); // Special pointer case.

            // OpenGL needs a count of 1.
            if( count == 0 )  count = 1;

            switch( constDesc[c].type() )
            {
               case CONST_FLOAT : glUniform1fv( loc, count, data ); break;
               case CONST_FLOAT2: glUniform2fv( loc, count, data ); break;
               case CONST_FLOAT3: glUniform3fv( loc, count, data ); break;
               case CONST_FLOAT4: glUniform4fv( loc, count, data ); break;
               case CONST_MAT2  : glUniformMatrix2fv( loc, count, GL_FALSE, data ); break;
               case CONST_MAT3  : glUniformMatrix3fv( loc, count, GL_FALSE, data ); break;
               case CONST_MAT4  : glUniformMatrix4fv( loc, count, GL_FALSE, data ); break;
            }
         }
      }
   }

   // Textures.
   uint curTexUnit = 0;
   if( samp )
   {
      for( uint i = 0; i < samp->size(); ++i )
      {
         const Sampler* s            = (*samp)[i].ptr();
         int samplerID               = oglProg->getSampler( s->name() );
         const OpenGLTexture* oglTex = (const OpenGLTexture*)s->texture().ptr();

         if( samplerID == -1 )
         {
            DBG_MSG( os_gl, "Sampler '" << s->name().cstr() << "' is not in the program... Skipped." );
            continue;
         }

         glActiveTexture( GL_TEXTURE0 + curTexUnit );
         glUniform1i( samplerID, curTexUnit );
         glBindTexture( oglTex->_oglType, oglTex->_id );

         DBG_MSG( os_gl, "Using texture #" << oglTex->_id << " in unit #" << curTexUnit );
         ++curTexUnit;

         // Set texture parameters.
         const TextureState& ts = s->state();
         uint tsDiff            = ts.compare( oglTex->_texState );
         if( oglTex->_texState != ts )
         {
            GLint maxA;
            GLenum magF, minF;
            toGLFilters( ts, magF, minF, maxA );
            switch( tsDiff )
            {
               case 1:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAG_FILTER, magF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MIN_FILTER, minF );
                  break;
               case 2:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_S, toGL( ts.clampX() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_T, toGL( ts.clampY() ) );
                  //glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_R_OES, toGL( ts.clampZ() ) );
                  break;
               case 3:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAG_FILTER, magF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MIN_FILTER, minF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_S, toGL( ts.clampX() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_T, toGL( ts.clampY() ) );
                  //glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_R_OES, toGL( ts.clampZ() ) );
                  break;
               default:
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAG_FILTER, magF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MIN_FILTER, minF );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_S, toGL( ts.clampX() ) );
                  glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_T, toGL( ts.clampY() ) );
                  //glTexParameteri( oglTex->_oglType, GL_TEXTURE_WRAP_R_OES, toGL( ts.clampZ() ) );
                  // FIXME glTexParameteri( oglTex->_oglType, GL_TEXTURE_BASE_LEVEL, ts.baseLevel() );
                  // FIXME glTexParameteri( oglTex->_oglType, GL_TEXTURE_MAX_LEVEL, ts.lastLevel() );
                  // FIXME glTexParameterf( oglTex->_oglType, GL_TEXTURE_LOD_BIAS, ts.LODBias() );
            }
            oglTex->_texState = ts;
         }
      }
   }
   // Deactive extraneous texture units.
   for( uint i = curTexUnit; i < _curTexUnitsUsed; ++i )
   {
      glActiveTexture( GL_TEXTURE0 + i );
      glBindTexture( GL_TEXTURE_2D, 0 );
   }
   // Update current number of active texture units.
   _curTexUnitsUsed = curTexUnit;

   // Geometry.
   const OpenGLGeometry* oglGeom = (const OpenGLGeometry*)geom;
   oglGeom->activate();

   // Draw primitives.
   const OpenGLIndexBuffer* indexBuffer = (const OpenGLIndexBuffer*)(geom->indexBuffer().ptr());
   if( indexBuffer != nullptr )
   {
      if( range )
      {
         DBG_MSG( os_gl, "Drawing " << geom->numPrimitives() << " " << toStr(geom->primitiveType()) << " with range: " << range );
         glDrawElements(
            toGL( geom->primitiveType() ),
            range[1],
            toGL( indexBuffer->format() ),
            (const void*)(toBytes( indexBuffer->format() ) * range[0])
         );
      }
      else
      {
         DBG_MSG( os_gl, "Drawing " << geom->numPrimitives() << " " << toStr(geom->primitiveType()) );
         glDrawElements(
            toGL( geom->primitiveType() ),
            GLsizei(indexBuffer->numIndices()),
            toGL( indexBuffer->format() ),
            0
         );
      }
   }
   else
   {
      // With incrementing indices.
      if( range )
      {
         glDrawArrays(
            toGL( geom->primitiveType() ),
            range[0],
            range[1]
         );
      }
      else
      {
         glDrawArrays(
            toGL( geom->primitiveType() ),
            0,
            GLsizei(geom->buffers()[0]->numVertices())
         );
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLES2Manager::render( const Pass& pass )
{
   DBG_BLOCK( os_gl, "GLES2Manager::render" );

   bool ok = true;
   bool wire = false;

   ok &= setFramebuffer( pass._framebuffer.ptr() );

   // Set viewport and scissor.
   glViewport( 0, 0, _curWidth, _curHeight );
   glScissor( 0, 0, _curWidth, _curHeight );

#if 0
   printf("Custom OpenGL rendering %dx%d\n", _curWidth, _curHeight);
   //glClearColor(1, 0, 0, 1);
   //glClear( GL_COLOR_BUFFER_BIT );
   return true;
#endif

   // Set to default render states.
   applyState( alphaStateDef, _cache->_alphaState );
   applyState( colorStateDef, _cache->_colorState );
   applyState( cullStateDef, _cache->_cullState );
   applyState( depthStateDef, _cache->_depthState );
   applyState( offsetStateDef, _cache->_offsetState );
   applyState( stencilStateDef, _cache->_stencilState );

   // Clear buffers.
   if( pass._clearBuffers )
   {
      glClearColor( pass._color[0], pass._color[1], pass._color[2], pass._color[3] );
      glClearDepthf( pass._z );
      glClearStencil( pass._stencil );
      switch( pass._clearBuffers )
      {
         case CLEAR_COLOR  : glClear( GL_COLOR_BUFFER_BIT ); break;
         case CLEAR_DEPTH  : glClear( GL_DEPTH_BUFFER_BIT ); break;
         case CLEAR_STENCIL: glClear( GL_STENCIL_BUFFER_BIT ); break;
         case CLEAR_COLOR | CLEAR_DEPTH   : glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); break;
         case CLEAR_COLOR | CLEAR_STENCIL : glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); break;
         case CLEAR_DEPTH | CLEAR_STENCIL : glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); break;
         case CLEAR_ALL: glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); break;
         default: DBG_MSG( os_gl, "Invalid _clearBuffers" ); break;
      }
   }

   // Keep current information in the pass.
   Mat4f viewProj( Mat4f::identity() );

   const Geometry*     curGeom  = 0;
   const Program*      curProg  = 0;
   const SamplerList*  curSamp  = 0;
   const ConstantList* curConst = 0;
   const float* curMatrices[4]  = { identityMatrix, identityMatrix, identityMatrix, viewProj.ptr() };
   const float* curCamPosition  = 0;
   const uint*           range  = 0;

   // Used only for PASS_CMD_RENDER.
   const int*       curScissor  = 0;
   const int*      curViewport  = 0;

   // Execute all command in the pass.
   Pass::CommandContainer::ConstIterator curCmd =  pass._commands.begin();
   Pass::CommandContainer::ConstIterator endCmd =  pass._commands.end();
   while( curCmd != endCmd )
   {
      switch( (*curCmd)._id )
      {
         case Pass::PASS_CMD_NOOP:
         {
         } break;
         case Pass::PASS_CMD_MESSAGE:
         {
            DBG( const char* msg = (*curCmd)._cPtr );
            DBG_MSG( os_gl, msg );
         } break;
         case Pass::PASS_CMD_RENDER:
         {
            // Save state (we reuse many variables existing in this scope).
            const Cache savedCache = *_cache;

            // Render node.
            const RCP<RenderNode> rn = (RenderNode*)(*curCmd)._data.ptr(); // Sneaky const_cast at the same time.
            Manager::render( rn );

            // Restore various states.
            // - Framebuffer.
            ok &= setFramebuffer( pass._framebuffer.ptr() );
            // - Viewport
            if( curViewport != 0 )
            {
               glViewport( curViewport[0], curViewport[1], curViewport[2], curViewport[3] );
            }
            else
            {
               glViewport( 0, 0, _curWidth, _curHeight );
            }
            // - Scissor.
            if( curScissor != 0 )
            {
               glScissor( curScissor[0], curScissor[1], curScissor[2], curScissor[3] );
            }
            else
            {
               glScissor( 0, 0, _curWidth, _curHeight );
            }
            // - State.
            applyState( savedCache._alphaState,   _cache->_alphaState   );
            applyState( savedCache._colorState,   _cache->_colorState   );
            applyState( savedCache._cullState,    _cache->_cullState    );
            applyState( savedCache._depthState,   _cache->_depthState   );
            applyState( savedCache._offsetState,  _cache->_offsetState   );
            applyState( savedCache._stencilState, _cache->_stencilState );
            // (no need to set clear color, as we have no CLEAR commands)
            // - Wireframe.
            if( wire )
            {
               //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); FIXME
            }
            else
            {
               //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); FIXME
            }
         } break;
         case Pass::PASS_CMD_SET_ALPHA_STATE:
         {
            applyState( *(const AlphaState*)(*curCmd)._data.ptr(), _cache->_alphaState );
         } break;
         case Pass::PASS_CMD_SET_COLOR_STATE:
         {
            applyState( *(const ColorState*)(*curCmd)._data.ptr(), _cache->_colorState );
         } break;
         case Pass::PASS_CMD_SET_CULL_STATE:
         {
            applyState( *(const CullState*)(*curCmd)._data.ptr(), _cache->_cullState );
         } break;
         case Pass::PASS_CMD_SET_DEPTH_STATE:
         {
            applyState( *(const DepthState*)(*curCmd)._data.ptr(), _cache->_depthState );
         } break;
         case Pass::PASS_CMD_SET_OFFSET_STATE:
         {
            applyState( *(const OffsetState*)(*curCmd)._data.ptr(), _cache->_offsetState );
         } break;
         case Pass::PASS_CMD_SET_STENCIL_STATE:
         {
            applyState( *(const StencilState*)(*curCmd)._data.ptr(), _cache->_stencilState );
         } break;
         case Pass::PASS_CMD_SET_WIREFRAME:
         {
            //FIXME glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            wire = true;
         } break;
         case Pass::PASS_CMD_SET_FILL:
         {
            // FIXME glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            wire = false;
         } break;
         case Pass::PASS_CMD_SET_PROGRAM:
         {
            curProg = (const Program*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_EXEC:
         {
            ok &= executeGeometry( curGeom, curProg, curConst, curSamp, curMatrices, curCamPosition, range );
         } break;
         case Pass::PASS_CMD_SET_GEOMETRY:
         {
            //Keep it for the exec clause.
            range   = 0;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_EXEC_GEOMETRY:
         {
            range   = 0;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
            ok &= executeGeometry( curGeom, curProg, curConst, curSamp, curMatrices, curCamPosition, range );
         } break;
         case Pass::PASS_CMD_SET_RANGEGEOMETRY:
         {
            //Keep it for the exec clause.
            range   = (*curCmd)._uPtr;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_EXEC_RANGEGEOMETRY:
         {
            range   = (*curCmd)._uPtr;
            curGeom = (const Geometry*)(*curCmd)._data.ptr();
            ok &= executeGeometry( curGeom, curProg, curConst, curSamp, curMatrices, curCamPosition, range );
         } break;
         case Pass::PASS_CMD_SET_CONSTANTS:
         {
            curConst = (const ConstantList*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_SET_CAMERA_POSITION:
         {
            curCamPosition = (const float*)(*curCmd)._fPtr;
         } break;
         case Pass::PASS_CMD_SET_SAMPLERS:
         {
            curSamp = (const SamplerList*)(*curCmd)._data.ptr();
         } break;
         case Pass::PASS_CMD_COPY_COLOR:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glFlush();
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_COPY_DEPTH:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_COPY_DEPTH_STENCIL:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_COPY_STENCIL:
         {
            const OpenGLTexture* dst = (const OpenGLTexture*)(*curCmd)._data.ptr();
            CHECK( dst->_oglType == GL_TEXTURE_2D );
            glBindTexture( dst->_oglType, dst->_id );
            glCopyTexImage2D( dst->_oglType, 0, toGLInternalFormat(dst->format()), 0, 0, _curWidth, _curHeight, 0 );
            glBindTexture( dst->_oglType, 0 );
         } break;
         case Pass::PASS_CMD_SET_SCISSOR:
         {
            curScissor = (*curCmd)._iPtr;
            glScissor( curScissor[0], curScissor[1], curScissor[2], curScissor[3] );
         } break;
         case Pass::PASS_CMD_SET_VIEWPORT:
         {
            curViewport = (*curCmd)._iPtr;
            glViewport( curViewport[0], curViewport[1], curViewport[2], curViewport[3] );
         } break;
         case Pass::PASS_CMD_SET_PROJECTION:
         {
            curMatrices[0] = (*curCmd)._fPtr;
            // Update view projection matrix.
            viewProj = (*(const Mat4f*)curMatrices[0]) * (*(const Mat4f*)curMatrices[1]);
         } break;
         case Pass::PASS_CMD_SET_VIEW:
         {
            curMatrices[1] = (*curCmd)._fPtr;
            // Update view projection matrix.
            viewProj = (*(const Mat4f*)curMatrices[0]) * (*(const Mat4f*)curMatrices[1]);
         } break;
         case Pass::PASS_CMD_SET_WORLD:
         {
            curMatrices[2] = (*curCmd)._fPtr;
         } break;
         default:
         {
            printf("UNKOWN PASS COMMAND TYPE: %d\n", (*curCmd)._id);
            ok = false;
         } break;
      }
#if 0 //_DEBUG && 0
      static char dbgMsg[1024];
      sprintf( dbgMsg, "After command %d (%s): %%s\n", (*curCmd)._id, Pass::toStr((*curCmd)._id) );
      checkForErrors(dbgMsg);
#endif
      ++curCmd;
   }

   if( wire )
   {
      // FIXME glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   }

   glFlush();

   if( _doingRTT )
   {
      Framebuffer* fb = const_cast<Gfx::Framebuffer*>(pass._framebuffer.ptr());
      if( pass._fbGenerateMipmaps )
      {
         if( fb->_colorBuffer.isValid() && fb->_colorBuffer->isMipmapped() )
         {
            generateMipmaps(fb->_colorBuffer);
         }
         if( fb->_depthBuffer.isValid() && fb->_depthBuffer->isMipmapped() )
         {
            generateMipmaps(fb->_depthBuffer);
         }
         if( fb->_stencilBuffer.isValid() && fb->_stencilBuffer->isMipmapped() )
         {
            generateMipmaps(fb->_stencilBuffer);
         }
      }
   }

   // Unbind current VAO.
   glBindVertexArrayOES( 0 );

   // Deactive extraneous texture units.
   for( uint i = 0; i < _curTexUnitsUsed; ++i )
   {
      glActiveTexture( GL_TEXTURE0 + i );
      glBindTexture( GL_TEXTURE_2D, 0 );
   }
   // Update current number of active texture units.
   _curTexUnitsUsed = 0;

   return ok;
}

#endif //GFX_GLES_2_SUPPORT
