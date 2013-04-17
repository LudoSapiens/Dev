/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/D3D/D3DManager.h>

#if GFX_D3D_SUPPORT

#include <Gfx/Mgr/D3D/D3DContext.h>

//Complete forward declarations
#include <Gfx/Geom/Buffer.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Prog/Program.h>
#include <Gfx/FB/RenderState.h>
#include <Gfx/Tex/Texture.h>
#include <Gfx/Tex/TextureState.h>

#include <CGMath/Mat4.h>

#include <Base/Util/Bits.h>
#include <Base/Dbg/DebugStream.h>

#include <cassert>


USING_NAMESPACE

using namespace Gfx;


/*==============================================================================
   UNNAME_NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_d3d, "D3DManager" );

/*==============================================================================
   Utilities
==============================================================================*/

//------------------------------------------------------------------------------
//!
DWORD formatToUsage( const D3DFORMAT d3d_tfmt, const TextureFlags flags )
{
   if( isRenderable(flags) )
   {
      switch(d3d_tfmt)
      {
      case D3DFMT_A2R10G10B10:
      case D3DFMT_A8R8G8B8:
      case D3DFMT_X8R8G8B8:
      case D3DFMT_A1R5G5B5:
      case D3DFMT_X1R5G5B5:
      case D3DFMT_R5G6B5:
      case D3DFMT_A16B16G16R16:
      case D3DFMT_A16B16G16R16F:
         return D3DUSAGE_RENDERTARGET;
      case D3DFMT_D16_LOCKABLE:
      case D3DFMT_D32:
      case D3DFMT_D15S1:
      case D3DFMT_D24S8:
      case D3DFMT_D24X8:
      case D3DFMT_D24X4S4:
      case D3DFMT_D32F_LOCKABLE:
      case D3DFMT_D24FS8:
      case D3DFMT_D16:
         return D3DUSAGE_DEPTHSTENCIL;
      default:
         return 0;
      }
   }
   else
   {
      return 0;
   }
}

//------------------------------------------------------------------------------
//!
inline D3DPRIMITIVETYPE
toD3D( const PrimitiveType pt )
{
   switch( pt )
   {
      case PRIM_INVALID       : return (D3DPRIMITIVETYPE)-1;
      case PRIM_POINTS        : return D3DPT_POINTLIST;
      case PRIM_LINES         : return D3DPT_LINELIST;
      case PRIM_LINE_LOOP     : return (D3DPRIMITIVETYPE)-1;
      case PRIM_LINE_STRIP    : return D3DPT_LINESTRIP;
      case PRIM_TRIANGLES     : return D3DPT_TRIANGLELIST;
      case PRIM_TRIANGLE_STRIP: return D3DPT_TRIANGLESTRIP;
      case PRIM_TRIANGLE_FAN  : return D3DPT_TRIANGLEFAN;
      case PRIM_QUADS         : return (D3DPRIMITIVETYPE)-1;
      case PRIM_QUAD_STRIP    : return (D3DPRIMITIVETYPE)-1;
      case PRIM_POLYGON       : return (D3DPRIMITIVETYPE)-1;
      default                 : return (D3DPRIMITIVETYPE)-1;
   }
}

//------------------------------------------------------------------------------
//!
inline D3DDECLTYPE
toD3D( const AttributeFormat afmt )
{
   switch( afmt )
   {
      case ATTRIB_FMT_INVALID        : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_8              : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_8_8            : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_8_8_8          : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_8_8_8_8        : return D3DDECLTYPE_UBYTE4N;
      case ATTRIB_FMT_16             : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_16_16          : return D3DDECLTYPE_USHORT2N;
      case ATTRIB_FMT_16_16_16       : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_16_16_16_16    : return D3DDECLTYPE_USHORT4N;
      case ATTRIB_FMT_16F            : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_16F_16F        : return D3DDECLTYPE_FLOAT16_2;
      case ATTRIB_FMT_16F_16F_16F    : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_16F_16F_16F_16F: return D3DDECLTYPE_FLOAT16_4;
      case ATTRIB_FMT_32             : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_32_32          : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_32_32_32       : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_32_32_32_32    : return D3DDECLTYPE_UNUSED;
      case ATTRIB_FMT_32F            : return D3DDECLTYPE_FLOAT1;
      case ATTRIB_FMT_32F_32F        : return D3DDECLTYPE_FLOAT2;
      case ATTRIB_FMT_32F_32F_32F    : return D3DDECLTYPE_FLOAT3;
      case ATTRIB_FMT_32F_32F_32F_32F: return D3DDECLTYPE_FLOAT4;
      default                        : return D3DDECLTYPE_UNUSED;
   }
}

//------------------------------------------------------------------------------
//!
inline void
toD3D( const AttributeType atype, uint& usage, uint& uid )
{
   switch( atype )
   {
      case ATTRIB_TYPE_POSITION:
         usage = D3DDECLUSAGE_POSITION;
         uid   = 0;
         break;
      case ATTRIB_TYPE_COLOR:
         usage = D3DDECLUSAGE_COLOR;
         uid   = 0;
         break;
      case ATTRIB_TYPE_TEXCOORD0:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 0;
         break;
      case ATTRIB_TYPE_TEXCOORD1:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 1;
         break;
      case ATTRIB_TYPE_TEXCOORD2:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 2;
         break;
      case ATTRIB_TYPE_TEXCOORD3:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 3;
         break;
      case ATTRIB_TYPE_TEXCOORD4:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 4;
         break;
      case ATTRIB_TYPE_TEXCOORD5:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 5;
         break;
      case ATTRIB_TYPE_TEXCOORD6:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 6;
         break;
      case ATTRIB_TYPE_TEXCOORD7:
         usage = D3DDECLUSAGE_TEXCOORD;
         uid   = 7;
         break;
      case Gfx::ATTRIB_TYPE_NORMAL:
         usage = D3DDECLUSAGE_NORMAL;
         uid   = 0;
         break;
      case Gfx::ATTRIB_TYPE_TANGENT:
         usage = D3DDECLUSAGE_TANGENT;
         uid   = 0;
         break;
      case Gfx::ATTRIB_TYPE_BINORMAL:
         usage = D3DDECLUSAGE_BINORMAL;
         uid   = 0;
         break;
      default:
         CHECK( false );
         usage = 0;
         uid   = 0;
         break;
   }
}

//------------------------------------------------------------------------------
//!
inline void
toD3D(
   const TextureFormat   tfmt,
   const TextureChannels co,
   D3DFORMAT&            d3d_fmt,
   TextureChannels&      d3d_co
)
{
   switch( tfmt )
   {
      case TEX_FMT_INVALID:
      {
         d3d_fmt = D3DFMT_UNKNOWN;
         d3d_co = TEX_CHANS_UNSPECIFIED;
      } break;

      case TEX_FMT_8:
      {
         switch(co)
         {
         case TEX_CHANS_L:
            d3d_fmt = D3DFMT_L8;
            d3d_co = TEX_CHANS_L;
            break;
         case TEX_CHANS_A:
            d3d_fmt = D3DFMT_A8;
            d3d_co = TEX_CHANS_A;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_8_8:
      {
         switch(co)
         {
         case TEX_CHANS_AL:
         case TEX_CHANS_LA:
            d3d_fmt = D3DFMT_A8L8;
            d3d_co = TEX_CHANS_LA;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_8_8_8_8:
      {
         switch( co )
         {
         case TEX_CHANS_RGBA:
         case TEX_CHANS_BGRA:
         case TEX_CHANS_ARGB:
         case TEX_CHANS_ABGR:
            d3d_fmt = D3DFMT_A8R8G8B8;  //D3DFMT_A8B8G8R8 doesn't work
            d3d_co = TEX_CHANS_BGRA;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_16:
      {
         switch(co)
         {
         case TEX_CHANS_Z:
            d3d_fmt = D3DFMT_D16;
            d3d_co = TEX_CHANS_Z;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_16_16:
      {
         switch(co)
         {
         case TEX_CHANS_RG:
            d3d_fmt = D3DFMT_G16R16;
            d3d_co = TEX_CHANS_RG;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_16_16_16_16:
      {
         switch( co )
         {
         case TEX_CHANS_RGBA:
         case TEX_CHANS_BGRA:
         case TEX_CHANS_ARGB:
         case TEX_CHANS_ABGR:
            d3d_fmt = D3DFMT_A16B16G16R16;
            d3d_co = TEX_CHANS_RGBA;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_16F:
      {
         switch(co)
         {
         case TEX_CHANS_Z:
            d3d_fmt = D3DFMT_R16F;
            d3d_co = TEX_CHANS_Z;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_16F_16F:
      {
         switch(co)
         {
         case TEX_CHANS_RG:
            d3d_fmt = D3DFMT_G16R16;
            d3d_co = TEX_CHANS_RG;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_16F_16F_16F_16F:
      {
         switch( co )
         {
         case TEX_CHANS_RGBA:
         case TEX_CHANS_BGRA:
         case TEX_CHANS_ARGB:
         case TEX_CHANS_ABGR:
            d3d_fmt = D3DFMT_A16B16G16R16F;
            d3d_co = TEX_CHANS_RGBA;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_32:
      {
         switch( co )
         {
         case TEX_CHANS_Z:
            d3d_fmt = D3DFMT_D32;
            d3d_co = TEX_CHANS_Z;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_32_32:
      {
         //Probably not supported
         d3d_fmt = D3DFMT_UNKNOWN;
         d3d_co = co;
      } break;

      case TEX_FMT_32_32_32_32:
      {
         //Probably not supported
         d3d_fmt = D3DFMT_UNKNOWN;
         d3d_co = co;
      } break;

      case TEX_FMT_32F:
      {
         switch( co )
         {
         case TEX_CHANS_R:
            d3d_fmt = D3DFMT_R32F;
            d3d_co = TEX_CHANS_R;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_32F_32F:
      {
         switch( co )
         {
         case TEX_CHANS_RG:
         case TEX_CHANS_GR:
            d3d_fmt = D3DFMT_G32R32F;
            d3d_co = TEX_CHANS_RG;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_32F_32F_32F_32F:
      {
         switch( co )
         {
         case TEX_CHANS_RGBA:
         case TEX_CHANS_BGRA:
         case TEX_CHANS_ARGB:
         case TEX_CHANS_ABGR:
            d3d_fmt = D3DFMT_A32B32G32R32F;
            d3d_co = TEX_CHANS_RGBA;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      case TEX_FMT_24_8:
      case TEX_FMT_8_24:
      {
         switch( co )
         {
         case TEX_CHANS_SZ:
         case TEX_CHANS_ZS:
            d3d_fmt = D3DFMT_D24S8;
            d3d_co = TEX_CHANS_SZ;
            break;
         default:
            CHECK( false );
            d3d_fmt = D3DFMT_UNKNOWN;
            d3d_co  = co;
            break;
         }
      } break;

      default:
      {
         //Probably not supported
         CHECK( false );
         d3d_fmt = D3DFMT_UNKNOWN;
         d3d_co  = co;
      } break;
   }
}


//------------------------------------------------------------------------------
//!
inline void
toD3DFilters(
   const TextureState&   state,
   D3DTEXTUREFILTERTYPE& magFilter,
   D3DTEXTUREFILTERTYPE& minFilter,
   D3DTEXTUREFILTERTYPE& mipFilter,
   DWORD&                maxAniso
)
{
   if( state.maxAniso() != 0 )
   {
      //Aniso case (force trilinear aniso)
      magFilter = D3DTEXF_ANISOTROPIC;
      minFilter = D3DTEXF_ANISOTROPIC;
      mipFilter = D3DTEXF_LINEAR;
      maxAniso = state.maxAniso();
   }
   else
   {
      switch( state.magFilter() )
      {
      case TEX_FILTER_POINT :  magFilter = D3DTEXF_POINT;  break;
      case TEX_FILTER_LINEAR:  magFilter = D3DTEXF_LINEAR; break;
      default               :  magFilter = D3DTEXF_POINT;  break;
      }
      switch( state.minFilter() )
      {
      case TEX_FILTER_POINT :  minFilter = D3DTEXF_POINT;  break;
      case TEX_FILTER_LINEAR:  minFilter = D3DTEXF_LINEAR; break;
      default               :  minFilter = D3DTEXF_POINT;  break;
      }
      //Ignoring ZFilter
      switch( state.mipFilter() )
      {
      case TEX_FILTER_NONE  :  mipFilter = D3DTEXF_NONE;   break;
      case TEX_FILTER_POINT :  mipFilter = D3DTEXF_POINT;  break;
      case TEX_FILTER_LINEAR:  mipFilter = D3DTEXF_LINEAR; break;
      default               :  mipFilter = D3DTEXF_POINT;  break;
      }
      maxAniso = 1;  //D3D default
   }
}

//------------------------------------------------------------------------------
//!
inline D3DTEXTUREADDRESS
toD3D( const TexClamp clamp )
{
   switch(clamp)
   {
      case TEX_CLAMP_WRAP             : return D3DTADDRESS_WRAP;
      case TEX_CLAMP_MIRROR           : return D3DTADDRESS_MIRROR;
      case TEX_CLAMP_LAST             : return D3DTADDRESS_CLAMP;
      case TEX_CLAMP_BORDER           : return D3DTADDRESS_BORDER;
      case TEX_CLAMP_MIRRORONCE_LAST  : return D3DTADDRESS_MIRRORONCE;
      case TEX_CLAMP_MIRRORONCE_BORDER: return D3DTADDRESS_MIRRORONCE;  //border not supported in D3D
      default                         : return D3DTADDRESS_CLAMP;
   }
}

//------------------------------------------------------------------------------
//!
inline D3DCMPFUNC
toD3D( CompareFunc func )
{
   switch( func )
   {
      case COMPARE_FUNC_NEVER       : return D3DCMP_NEVER;
      case COMPARE_FUNC_LESS        : return D3DCMP_LESS;
      case COMPARE_FUNC_EQUAL       : return D3DCMP_EQUAL;
      case COMPARE_FUNC_LESSEQUAL   : return D3DCMP_LESSEQUAL;
      case COMPARE_FUNC_GREATER     : return D3DCMP_GREATER;
      case COMPARE_FUNC_NOTEQUAL    : return D3DCMP_NOTEQUAL;
      case COMPARE_FUNC_GREATEREQUAL: return D3DCMP_GREATEREQUAL;
      case COMPARE_FUNC_ALWAYS      : return D3DCMP_ALWAYS;
      default                       : return D3DCMP_LESSEQUAL;
   }
}

//------------------------------------------------------------------------------
//!
inline D3DBLEND
toD3D( Gfx::AlphaBlend blend )
{
   switch( blend )
   {
      case ALPHA_BLEND_ZERO                 : return D3DBLEND_ZERO;
      case ALPHA_BLEND_ONE                  : return D3DBLEND_ONE;
      case ALPHA_BLEND_SRC_COLOR            : return D3DBLEND_SRCCOLOR;
      case ALPHA_BLEND_INV_SRC_COLOR        : return D3DBLEND_INVSRCCOLOR;
      case ALPHA_BLEND_SRC_ALPHA            : return D3DBLEND_SRCALPHA;
      case ALPHA_BLEND_INV_SRC_ALPHA        : return D3DBLEND_INVSRCALPHA;
      case ALPHA_BLEND_DST_ALPHA            : return D3DBLEND_DESTALPHA;
      case ALPHA_BLEND_INV_DST_ALPHA        : return D3DBLEND_INVDESTALPHA;
      case ALPHA_BLEND_DST_COLOR            : return D3DBLEND_DESTCOLOR;
      case ALPHA_BLEND_INV_DST_COLOR        : return D3DBLEND_INVDESTCOLOR;
      case ALPHA_BLEND_SRC_ALPHA_SAT        : return D3DBLEND_SRCALPHASAT;
      case ALPHA_BLEND_BOTH_SRC_ALPHA       : return D3DBLEND_BOTHSRCALPHA;
      case ALPHA_BLEND_BOTH_INV_SRC_ALPHA   : return D3DBLEND_BOTHINVSRCALPHA;
      case ALPHA_BLEND_BLEND_FACTOR         : return D3DBLEND_BLENDFACTOR;
      case ALPHA_BLEND_INV_BLEND_FACTOR     : return D3DBLEND_INVBLENDFACTOR;
      default                               : return D3DBLEND_ONE;
   }
}

//------------------------------------------------------------------------------
//!
inline D3DSTENCILOP
toD3D( StencilOp op )
{
   switch( op )
   {
      case STENCIL_OP_KEEP     : return D3DSTENCILOP_KEEP;
      case STENCIL_OP_ZERO     : return D3DSTENCILOP_ZERO ;
      case STENCIL_OP_REPLACE  : return D3DSTENCILOP_REPLACE;
      case STENCIL_OP_INCR_SAT : return D3DSTENCILOP_INCRSAT;
      case STENCIL_OP_DECR_SAT : return D3DSTENCILOP_DECRSAT;
      case STENCIL_OP_INVERT   : return D3DSTENCILOP_INVERT;
      case STENCIL_OP_INCR     : return D3DSTENCILOP_INCR;
      case STENCIL_OP_DECR     : return D3DSTENCILOP_DECR;
      default                  : return D3DSTENCILOP_KEEP;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState(
   const AlphaState& state,
   AlphaState&       curState,
   IDirect3DDevice9* device
)
{
   switch( state.compare(curState) )
   {
      case 0:
         // Identical, nothing to do.
         return;
      case 1:
         // Optimized path: only blending state changed.
         device->SetRenderState(
            D3DRS_ALPHABLENDENABLE, state.alphaBlending() ? TRUE : FALSE
         );
         break;
      case 4:
         // Testing.
         device->SetRenderState(
            D3DRS_ALPHATESTENABLE, state.alphaTesting() ? TRUE : FALSE
         );
         break;
      case 5:
         // Blending.
         device->SetRenderState(
            D3DRS_ALPHABLENDENABLE, state.alphaBlending() ? TRUE : FALSE
         );
         // Testing.
         device->SetRenderState(
            D3DRS_ALPHATESTENABLE, state.alphaTesting() ? TRUE : FALSE
         );
         break;
      default:
         // Perform full state update.
         // Blending.
         device->SetRenderState(
            D3DRS_ALPHABLENDENABLE, state.alphaBlending() ? TRUE : FALSE
         );
         device->SetRenderState( D3DRS_SRCBLEND, toD3D( state.alphaBlendSrc() ) );
         device->SetRenderState( D3DRS_DESTBLEND, toD3D( state.alphaBlendDst() ) );

         // Testing.
         device->SetRenderState(
            D3DRS_ALPHATESTENABLE, state.alphaTesting() ? TRUE : FALSE
         );
         device->SetRenderState( D3DRS_ALPHAREF, (DWORD)(state.alphaTestRef() * 255.0f));
         device->SetRenderState( D3DRS_ALPHAFUNC, toD3D( state.alphaTestFunc() ) );
   }

   // Update current state.
   curState = state;
}

//------------------------------------------------------------------------------
//!
inline void
applyState(
   const ColorState& state,
   ColorState&       curState,
   IDirect3DDevice9* device
)
{
   if( state != curState )
   {
      device->SetRenderState(
         D3DRS_COLORWRITEENABLE,
         state.colorWriting() ?
         D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
         D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA
         : 0
      );
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState(
   const CullState&  state,
   CullState&        curState,
   IDirect3DDevice9* device
)
{
   if( state != curState )
   {
      switch( state.side() )
      {
         case FACE_NONE: device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ); break;
         case FACE_BACK: device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW ); break;
         case FACE_FRONT: device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW ); break;
         case FACE_FRONT_AND_BACK: StdErr << "Not supported for now\n"; break;
         default: device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW ); break;
      }
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState(
   const DepthState& state,
   DepthState&       curState,
   IDirect3DDevice9* device
)
{
   if( state != curState )
   {
      device->SetRenderState(
         D3DRS_ZENABLE, state.depthTesting() ? D3DZB_TRUE : D3DZB_FALSE
      );
      device->SetRenderState( D3DRS_ZFUNC, toD3D( state.depthTestFunc() ) );
      device->SetRenderState(
         D3DRS_ZWRITEENABLE, state.depthWriting() ? TRUE : FALSE
      );
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
inline void
applyState(
   const OffsetState& state,
   OffsetState&       curState,
   IDirect3DDevice9*  device
)
{
   if( state != curState )
   {
      // inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }
      device->SetRenderState(
         D3DRS_SLOPESCALEDEPTHBIAS, F2DW(state.factor())
      );
      device->SetRenderState(
         D3DRS_DEPTHBIAS, F2DW(state.constant())
      );
      curState = state;
   }

}

//------------------------------------------------------------------------------
//!
inline void
applyState(
   const StencilState& state,
   StencilState&       curState,
   IDirect3DDevice9*   device
)
{
   if( state != curState )
   {
      device->SetRenderState(
         D3DRS_STENCILENABLE, state.stencilTesting() ? TRUE : FALSE
      );
      device->SetRenderState( D3DRS_STENCILREF, state.stencilTestRef() );
      device->SetRenderState( D3DRS_STENCILMASK, state.stencilTestRefMask() );
      device->SetRenderState( D3DRS_STENCILWRITEMASK, state.stencilTestWriteMask() );
      device->SetRenderState( D3DRS_STENCILFUNC, toD3D( state.stencilTestFunc() ) );
      device->SetRenderState( D3DRS_STENCILFAIL, toD3D( state.stencilFailOp() ) );
      device->SetRenderState( D3DRS_STENCILZFAIL, toD3D( state.stencilPassDepthFailOp() ) );
      device->SetRenderState( D3DRS_STENCILPASS, toD3D( state.stencilPassDepthPassOp() ) );
      curState = state;
   }
}

//------------------------------------------------------------------------------
//!
size_t
extractConstants(
   ConstantBuffer::Container&   constants,
   size_t                       currentOffset,
   ID3DXConstantTable*          constTable,
   const ShaderType             shaderType,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   CHECK( constTable != NULL );

   D3DXCONSTANTTABLE_DESC tableDesc;
   HRESULT res = constTable->GetDesc(&tableDesc);
   if( res != D3D_OK )
   {
      DBG_MSG( os_d3d, "GetDesc failed" );
      return 0;
   }
   else
   {
      DBG_MSG( os_d3d, "Found " << tableDesc.Constants << " constants" );
   }

   const ConstantType invalid = (Gfx::ConstantType)-1;
   ConstantType constType = invalid;

   // Iterator over all of the constants
   for( uint i = 0; i < tableDesc.Constants; ++i )
   {
      D3DXHANDLE constantID = constTable->GetConstant( NULL, i );

      D3DXCONSTANT_DESC constantDesc[1];
      UINT constantDescSize = 1;

      res = constTable->GetConstantDesc( constantID, constantDesc, &constantDescSize );
      if( res != D3D_OK )
      {
         DBG_MSG( os_d3d, "GetConstantDesc failed" );
         return 0;
      }

      // Iterate over all of the constant descriptions.
      for( UINT cDescID = 0; cDescID < constantDescSize; ++cDescID )
      {
         DBG_MSG( os_d3d, "Constant[" << i << "]:"
                       << " name=" << constantDesc[cDescID].Name
                       << " class=" << (uint)constantDesc[cDescID].Class
                       << " type=" << (uint)constantDesc[cDescID].Type
                       << " elements=" << (uint)constantDesc[cDescID].Elements
                       << " regSet=" << (uint)constantDesc[cDescID].RegisterSet
                       << " regCount=" << (uint)constantDesc[cDescID].RegisterCount );

         uint count = (uint)constantDesc[cDescID].Elements;
         if( count == 1 )
         {
            count = 0; //Do not keep pointer, but copy data instead.
         }

         switch( constantDesc[cDescID].Class )
         {
            case D3DXPC_SCALAR:
            {
               if( constantDesc[cDescID].Type == D3DXPT_FLOAT )
               {
                  DBG_MSG( os_d3d, "FLOAT SCALAR" );
                  constType = CONST_FLOAT;
               }
               else
               {
                  printf("%s - Only floats are supported.\n", constantDesc[cDescID].Name);
               }
            } break;
            case D3DXPC_VECTOR:
            {
               if( constantDesc[cDescID].Type == D3DXPT_FLOAT )
               {
                  constType = (ConstantType)(CONST_FLOAT + constantDesc[cDescID].Columns - 1);
                  DBG_MSG( os_d3d, "FLOAT VECTOR OF " << constType );
                  CHECK(constType <= CONST_FLOAT4);
               }
               else
               {
                  printf("%s - Only floats are supported.\n", constantDesc[cDescID].Name);
               }
            } break;
            case D3DXPC_MATRIX_ROWS:
            case D3DXPC_MATRIX_COLUMNS:
            {
               if( constantDesc[cDescID].Type == D3DXPT_FLOAT )
               {
                  if( constantDesc[cDescID].Rows == 4 && constantDesc[cDescID].Columns == 4 )
                  {
                     DBG_MSG( os_d3d, "FLOAT MATRIX 4x4" );
                     constType = CONST_MAT4;
                  }
                  else
                  if( constantDesc[cDescID].Rows == 3 && constantDesc[cDescID].Columns == 3 )
                  {
                     DBG_MSG( os_d3d, "FLOAT MATRIX 3x3" );
                     constType = CONST_MAT3;
                  }
                  else
                  if( constantDesc[cDescID].Rows == 2 && constantDesc[cDescID].Columns == 2 )
                  {
                     DBG_MSG( os_d3d, "FLOAT MATRIX 2x2" );
                     constType = CONST_MAT2;
                  }
                  else
                  {
                     printf("Matrix size of %dx%d is not supported.\n", constantDesc[cDescID].Columns, constantDesc[cDescID].Rows);
                     constType = invalid;
                  }
               }
               else
               {
                  printf("%s - Only floats are supported.\n", constantDesc[cDescID].Name);
                  constType = invalid;
               }
            } break;
            case D3DXPC_OBJECT:
            case D3DXPC_STRUCT:
            {
               // Silently ignore samplers and structures
               constType = invalid;
            } break;
            default:
            {
               printf("ERROR - Invalid constant class: %d\n", constantDesc[cDescID].Class);
            } break;
         }

         // Ignore irrelevant uniforms, and ones starting with "gfx"
         String constName( constantDesc[cDescID].Name );
         if( strncmp( constantDesc[cDescID].Name, "gfx", 3 ) == 0 )
         {
            constType = invalid;
         }
         else if( ignoreGroups )
         {
            for( uint32_t g = 0; g < ignoreGroups->size(); ++g )
            {
               if( (*ignoreGroups)[g].has( constName ) )
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
            size_t constOffset = Gfx::toBytes(constType);
            constants.pushBack(
               Gfx::ConstantBuffer::Constant(
                  constName.cstr(),
                  constType,
                  currentOffset,
                  count,
                  shaderType
               )
            );
            currentOffset += constOffset;
         }
      } //for( UINT cDescID = 0; cDescID < constantDescSize; ++cDescID )
   } //for( uint i = 0; i < tableDesc.Constants; ++i )

   return currentOffset;
}


//------------------------------------------------------------------------------
//!
IDirect3D9* d3d;
int         d3dCount = 0;

LPCSTR shaderProfile[3];

D3DVERTEXELEMENT9 vdecl[32];
D3DVERTEXELEMENT9 vend = D3DDECL_END();

float identityMatrix[16] =
{
   1.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f
};

AlphaState    alphaStateDef;    //!< Default alpha states.
ColorState    colorStateDef;    //!< Default color states.
ColorState    noColorStateDef;  //!< Default disabled color states.
CullState     cullStateDef;     //!< Default cull states.
DepthState    depthStateDef;    //!< Default depth states.
DepthState    noDepthStateDef;  //!< Default disabled depth states.
OffsetState   offsetStateDef;   //!< Default polygon offset states.
StencilState  stencilStateDef;  //!< Default stencil states.

/*==============================================================================
   CLASS D3DGeometry
==============================================================================*/
class  D3DGeometry :
   public Gfx::Geometry
{

public:

   /*----- methods -----*/

   D3DGeometry( const PrimitiveType type ) :
      Geometry( type ),
      _vertexDesc(0)
   {}

   virtual ~D3DGeometry()
   {
      if( _vertexDesc )
      {
         _vertexDesc->Release();
      }
   }

   inline IDirect3DVertexDeclaration9*
   getVertexDeclaration( uint revision, IDirect3DDevice9* device ) const
   {
      if( revision != _revision )
      {
         _revision = revision;

         uint curv = 0;
         for( uint i = 0; i < _buffers.size(); ++i )
         {
            for( uint cura = 0; cura < _buffers[i]->numAttributes(); ++cura, ++curv )
            {
               uint usage;
               uint uid;
               const RCP<const VertexBuffer::Attribute>& curAttrib = _buffers[i]->getAttribute( cura );
               toD3D( curAttrib->_type, usage, uid );
               vdecl[curv].Stream     = i;
               vdecl[curv].Offset     = curAttrib->_offset;
               vdecl[curv].Type       = toD3D( curAttrib->_format );
               vdecl[curv].Method     = D3DDECLMETHOD_DEFAULT;
               vdecl[curv].Usage      = usage;
               vdecl[curv].UsageIndex = uid;
            }
         }
         vdecl[curv] = vend;

         if( _vertexDesc )
         {
            _vertexDesc->Release();
         }
         device->CreateVertexDeclaration( vdecl, &_vertexDesc );
      }
      return _vertexDesc;
   }

   /*----- data members -----*/

   mutable IDirect3DVertexDeclaration9* _vertexDesc;
};

/*==============================================================================
   CLASS D3DIndexBuffer
==============================================================================*/
class  D3DIndexBuffer :
   public Gfx::IndexBuffer
{

public:

   /*----- methods -----*/

   D3DIndexBuffer( const IndexFormat format, const BufferFlags flags ):
      Gfx::IndexBuffer(format, flags),
      _ptr(0)
   {}

   virtual ~D3DIndexBuffer()
   {
      if( _ptr != 0 )
      {
         _ptr->Release();
      }
   }

   /*----- data members -----*/

   IDirect3DIndexBuffer9* _ptr;

};

/*==============================================================================
   CLASS D3DVertexBuffer
==============================================================================*/
class  D3DVertexBuffer :
   public Gfx::VertexBuffer
{

public:

   /*----- methods -----*/

   D3DVertexBuffer( const BufferFlags flags ):
      Gfx::VertexBuffer(flags),
      _ptr(0)
   {}

   virtual ~D3DVertexBuffer()
   {
      if( _ptr != 0 )
      {
         _ptr->Release();
      }
   }

   /*----- data members -----*/

   IDirect3DVertexBuffer9* _ptr;

};

/*==============================================================================
   CLASS D3DShader
==============================================================================*/

String vs_str_default =
"float4x4 gfxWorldViewProjectionMatrix;\n"
"float4x4 gfxWorldViewMatrix;\n"
"float4x4 gfxProjectionMatrix;\n"
"float4x4 gfxViewMatrix;\n"
"float4x4 gfxWorldMatrix;\n"
"float3 gfxCameraPosition;\n";

String ps_str_default =
"float3 gfxCameraPosition;\n";

class D3DShader:
   public Gfx::Shader
{

public:

   /*----- methods -----*/

   // Only Managers can create this object.
   D3DShader( Gfx::ShaderType type, const String& code, IDirect3DDevice9* device ) :
      Shader( type ), _constTable( 0 ), _vsh( 0 )
   {
      ID3DXBuffer* shCode;
      ID3DXBuffer* shError;

      String progCode;
      if( type == Gfx::VERTEX_SHADER )
      {
         progCode += vs_str_default;
      }
      else
      if( type == Gfx::FRAGMENT_SHADER )
      {
         progCode += ps_str_default;
      }
      else
      {
         //GEOM
      }
      progCode += code;

      HRESULT hrErr   = D3DXCompileShader(
         progCode.cstr(),
         progCode.size(),
         0,
         0,
         "main",
         shaderProfile[type],
         0,
         &shCode,
         &shError,
         &_constTable
      );

      if( FAILED(hrErr)  )
      {
         printf( "Error: %s\n",  (char*)shError->GetBufferPointer() );
         printf( "-----------------\n" );
         printf( "%s\n", progCode.cstr() );
         printf( "-----------------\n" );
         shError->Release();
      }

      if( type == Gfx::VERTEX_SHADER )
      {
         hrErr = device->CreateVertexShader( (DWORD*)shCode->GetBufferPointer(),  &_vsh );
         if( FAILED(hrErr) )
         {
            printf("Failed to create vertex shader.\n");
         }
      }
      else if( type == Gfx::FRAGMENT_SHADER )
      {
         device->CreatePixelShader( (DWORD*)shCode->GetBufferPointer(),  &_psh );
         if( FAILED(hrErr) )
         {
            printf("Failed to create vertex shader.\n");
         }
      }
      else
      {
         //GEOMETRY
         printf("Geometry shader not yet supported.\n");
      }
      shCode->Release();
   }

   virtual ~D3DShader()
   {
      if( _vsh )
      {

         if( _type == Gfx::VERTEX_SHADER )
         {
            _vsh->Release();
         }
         else if( _type == Gfx::FRAGMENT_SHADER )
         {
            _psh->Release();
         }
         else
         {
            //GEOMETRY
         }
      }
      _constTable->Release();
   }

   /*----- data members -----*/

   ID3DXConstantTable* _constTable;

   union {
      IDirect3DVertexShader9* _vsh;
      IDirect3DPixelShader9*  _psh;
   };
};

/*==============================================================================
   CLASS D3DProgram
==============================================================================*/

class D3DProgram:
   public Gfx::Program
{

public:

   /*----- Types -----*/

   enum
   {
      PROJ_MATRIX  = 0x01,
      VIEW_MATRIX  = 0x02,
      WORLD_MATRIX = 0x04,
      WV_MATRIX    = 0x08,
      WVP_MATRIX   = 0x10,
      CAM_POS_VS   = 0x20,
      CAM_POS_PS   = 0x40
   };

   /*----- methods -----*/

   D3DProgram() :
      _gfxConstants(0),
      _vsh(0), _psh(0), _psTable(0)
   {
   }

   virtual ~D3DProgram()
   {
   }

   virtual void addShader( const RCP<Gfx::Shader>& shader )
   {
      Program::addShader( shader );
      D3DShader* d3dShader = (D3DShader*)shader.ptr();
      if( d3dShader->_type == Gfx::VERTEX_SHADER )
      {
         //if _vsh != 0 ERROR!
         _vsh     = d3dShader->_vsh;
         _vsTable = d3dShader->_constTable;

         // Activate bits flag for wanted matrices.
         _gfxConstants = 0;
         if( _vsTable->GetConstantByName( 0, "gfxWorldViewProjectionMatrix" ) )
         {
            _gfxConstants |= WVP_MATRIX;
         }
         if( _vsTable->GetConstantByName( 0, "gfxWorldViewMatrix" ) )
         {
            _gfxConstants |= WV_MATRIX;
         }
         if( _vsTable->GetConstantByName( 0, "gfxProjectionMatrix" ) )
         {
            _gfxConstants |= PROJ_MATRIX;
         }
         if( _vsTable->GetConstantByName( 0, "gfxViewMatrix" ) )
         {
            _gfxConstants |= VIEW_MATRIX;
         }
         if( _vsTable->GetConstantByName( 0, "gfxWorldMatrix" ) )
         {
            _gfxConstants |= WORLD_MATRIX;
         }
         if( _vsTable->GetConstantByName( 0, "gfxCameraPosition" ) )
         {
            _gfxConstants |= CAM_POS_VS;
         }
      }
      else if( d3dShader->_type == Gfx::FRAGMENT_SHADER )
      {
         //if _psh != 0 ERROR!
         _psh     = d3dShader->_psh;
         _psTable = d3dShader->_constTable;

         if( _psTable->GetConstantByName( 0, "gfxCameraPosition" ) )
         {
            _gfxConstants |= CAM_POS_PS;
         }
      }
      else
      {
         //GEOMETRY
      }
   }

   virtual bool removeShader( const RCP<Gfx::Shader>& shader )
   {
      if( Program::removeShader( shader ) )
      {
         D3DShader* d3dShader = (D3DShader*)shader.ptr();
         if( d3dShader->_type == Gfx::VERTEX_SHADER )
         {
            _vsh     = 0;
            _vsTable = 0;
         }
         else if( d3dShader->_type == Gfx::FRAGMENT_SHADER )
         {
            _psh     = 0;
            _psTable = 0;
         }
         else
         {
            //GEOMETRY
         }
         return true;
      }
      return false;
   }

   inline void activate( IDirect3DDevice9* device ) const
   {
      CHECK( !_shaders.empty() );
      device->SetVertexShader( _vsh );
      device->SetPixelShader( _psh );
   }

   inline int getSampler( const ConstString& name ) const
   {
      return _psTable->GetSamplerIndex( name.cstr() );
   }

   /*----- data members -----*/

   uint                    _gfxConstants;
   IDirect3DVertexShader9* _vsh;
   IDirect3DPixelShader9*  _psh;
   ID3DXConstantTable*     _psTable;
   ID3DXConstantTable*     _vsTable;
};


/*==============================================================================
   CLASS D3DTexture
==============================================================================*/
class  D3DTexture :
   public Gfx::Texture
{

public:

   /*----- methods -----*/

   D3DTexture() :
      _ptr2D(NULL), _ptr(NULL)
   {}

   virtual ~D3DTexture()
   {
      if( _ptr )
      {
         switch(type())
         {
            case TEX_TYPE_1D:
               _ptr1D->Release();
               break;
            case TEX_TYPE_2D:
               _ptr2D->Release();
               break;
            case TEX_TYPE_3D:
               _ptr3D->Release();
               break;
            case TEX_TYPE_CUBEMAP:
               _ptrCube->Release();
               break;
            default:
               assert(0);  //Unsupported texture type
               break;
         }
      }
   }

   /*----- data members -----*/

   union
   {
      IDirect3DTexture9*        _ptr1D;
      IDirect3DTexture9*        _ptr2D;
      IDirect3DVolumeTexture9*  _ptr3D;
      IDirect3DCubeTexture9*    _ptrCube;
   };
   IDirect3DBaseTexture9*       _ptr;
   TextureChannels  _dstChannelOrder;

}; //D3DTexture


// Mipmap generation variable
String sRTT_VS_2D =
"struct VS_IN\n"
"{\n"
"   float4 pos: POSITION;\n"
"   float2 tex: TEXCOORD0;\n"
"};\n"
"struct VS_OUT\n"
"{\n"
"   float4 pos: POSITION;\n"
"   float2 tex: TEXCOORD0;\n"
"};\n"
"VS_OUT main( VS_IN In )\n"
"{\n"
"   VS_OUT Out;\n"
"   Out.pos = mul( gfxWorldViewProjectionMatrix, In.pos );\n"
"   Out.tex = In.pos * 0.5 + 0.5;\n"
"   return Out;\n"
"}\n";

//Note: sampler_state filter settings don't seem to work
String sRTT_PS_2D =
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

UNNAMESPACE_END



/*==============================================================================
   CLASS D3DManager::Cache
==============================================================================*/

class D3DManager::Cache
{
public:
   /*----- methods -----*/
   Cache();
   ~Cache();
   void  invalidate();

   /*----- data members -----*/

   const D3DProgram* _prog;      //!< Current shader program.
   TextureState      _texState[32];  //!< The texture states
   AlphaState        _alphaState;    //!< Current alpha states.
   ColorState        _colorState;    //!< Current color states.
   CullState         _cullState;     //!< Current cull states.
   DepthState        _depthState;    //!< Current depth states.
   OffsetState       _offsetState;   //!< Current offset states.
   StencilState      _stencilState;  //!< Current stencil states.
   uint              _curNbStreams;  //!< Current number of vertex data stream.
};

//------------------------------------------------------------------------------
//!
D3DManager::Cache::Cache() :
   _curNbStreams(0)
{
   invalidate();
}

//------------------------------------------------------------------------------
//!
D3DManager::Cache::~Cache()
{
}

//------------------------------------------------------------------------------
//!
void
D3DManager::Cache::invalidate()
{
   for( uint i = 0; i < 32; ++i )
   {
      _texState[i].setInvalidFields();
   }
   _prog = 0;
   _alphaState.setInvalidFields();
   _colorState.setInvalidFields();
   _cullState.setInvalidFields();
   _depthState.setInvalidFields();
   _offsetState.setInvalidFields();
   _stencilState.setInvalidFields();
}


/*==============================================================================
   CLASS D3DManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
D3DManager::D3DManager( D3DContext* context ):
   Manager( context, "Direct3D" ),
   _device(NULL),
   _oldRenderTarget(NULL),
   _oldDepthStencilSurface(NULL)
{
   DBG_BLOCK( os_d3d, "D3DManager::D3DManager" );

   HWND winHandle = context->window();

   // Initialize Direct3D.
   if( d3dCount++ == 0 )
   {
      d3d = Direct3DCreate9( D3D_SDK_VERSION );
   }

   // Create a device.
   D3DPRESENT_PARAMETERS params;
   ZeroMemory( &params, sizeof( D3DPRESENT_PARAMETERS ) );
   params.BackBufferCount            = 1;
   params.MultiSampleType            = D3DMULTISAMPLE_NONE;
   params.MultiSampleQuality         = 0;
   params.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
   params.hDeviceWindow              = winHandle;
   params.Flags                      = 0;
   params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
   params.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;
   //params.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;
   params.BackBufferFormat           = D3DFMT_UNKNOWN;
   params.EnableAutoDepthStencil     = FALSE;
   //params.EnableAutoDepthStencil     = TRUE;
   params.AutoDepthStencilFormat     = D3DFMT_D24S8;

   params.Windowed                   = TRUE;
   params.BackBufferWidth            = 0;
   params.BackBufferHeight           = 0;

   UINT adapter = D3DADAPTER_DEFAULT;
   D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

#define USE_NVPERFHUD 1
#if USE_NVPERFHUD
   for( uint a = 0; a < d3d->GetAdapterCount(); ++a )
   {
      D3DADAPTER_IDENTIFIER9 id;
      HRESULT res;
      res = d3d->GetAdapterIdentifier( a, 0, &id );

      if( strstr( id.Description, "PerfHUD" ) != 0 )
      {
         adapter    = a;
         deviceType = D3DDEVTYPE_REF;
         break;
      }
   }
#endif

   d3d->CreateDevice(
      adapter,
      deviceType,
      winHandle,
      D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
      &params,
      &_device
   );

   // Set default state.
   _device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
   _device->SetRenderState( D3DRS_LIGHTING, FALSE );
   _device->SetRenderState( D3DRS_SCISSORTESTENABLE ,TRUE );

   noDepthStateDef.depthTesting( false );
   noDepthStateDef.depthWriting( false );
   noColorStateDef.colorWriting( false );

   // Queries.
   shaderProfile[VERTEX_SHADER]   = D3DXGetVertexShaderProfile( _device );
   shaderProfile[FRAGMENT_SHADER] = D3DXGetPixelShaderProfile( _device );

   DBG_MSG( os_d3d, "vertex profile: " << shaderProfile[VERTEX_SHADER] );
   DBG_MSG( os_d3d, "fragment profile: " << shaderProfile[FRAGMENT_SHADER] );

   _cache = new Cache();
}

//------------------------------------------------------------------------------
//!
D3DManager::~D3DManager()
{
   delete _cache;

   // Release Direct3D.
   if( --d3dCount == 0 )
   {
      d3d->Release();
   }

   _device->Release();
}

//------------------------------------------------------------------------------
//!
void
D3DManager::display()
{
   _device->Present( 0, 0, 0, 0 );
}

//------------------------------------------------------------------------------
//!
float
D3DManager::oneToOneOffset()
{
   return -0.5f;
}

//------------------------------------------------------------------------------
//!
RCP<IndexBuffer>
D3DManager::createBuffer(
   const IndexFormat  format,
   const BufferFlags  flags,
   const size_t       sizeInBytes,
   const void*        data
)
{
   RCP<IndexBuffer> buffer( new D3DIndexBuffer(format, flags) );
   setData( buffer, sizeInBytes, data );
   return buffer;
}

//------------------------------------------------------------------------------
//!
RCP<VertexBuffer>
D3DManager::createBuffer(
   const BufferFlags  flags,
   const size_t       sizeInBytes,
   const void*        data
)
{
   RCP<VertexBuffer> buffer( new D3DVertexBuffer(flags) );
   setData( buffer, sizeInBytes, data );
   return buffer;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setData(
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

   D3DIndexBuffer* d3dBuffer = (D3DIndexBuffer*)buffer.ptr();

   // Allocate buffer.
   if( d3dBuffer->_ptr == 0 || d3dBuffer->_sizeInBytes != sizeInBytes )
   {
      if( d3dBuffer->_ptr != 0 )
      {
         d3dBuffer->_ptr->Release();
      }
      assert( buffer->_format != INDEX_FMT_8 );
      DWORD   usage;
      D3DPOOL pool;
      if( d3dBuffer->isStreamable() )
      {
         usage = D3DUSAGE_DYNAMIC;
         pool  = D3DPOOL_DEFAULT;
      }
      else
      {
         usage = D3DUSAGE_WRITEONLY;
         pool  = D3DPOOL_MANAGED;
      }
      _device->CreateIndexBuffer(
         sizeInBytes,
         usage,
         buffer->_format == INDEX_FMT_16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
         pool,
         &d3dBuffer->_ptr,
         0
      );
      d3dBuffer->_sizeInBytes = sizeInBytes;
   }

   if( data )
   {
      // Copy data.
      void* d3dData;
      d3dBuffer->_ptr->Lock( 0, sizeInBytes, &d3dData, 0 );
      memcpy( d3dData, data, sizeInBytes );
      d3dBuffer->_ptr->Unlock();
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setData(
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

   D3DVertexBuffer* d3dBuffer = (D3DVertexBuffer*)buffer.ptr();

   // Allocate buffer.
   if( d3dBuffer->_ptr == 0 || d3dBuffer->_sizeInBytes != sizeInBytes )
   {
      if( d3dBuffer->_ptr != 0 )
      {
         d3dBuffer->_ptr->Release();
      }
      DWORD   usage;
      D3DPOOL pool;
      if( d3dBuffer->isStreamable() )
      {
         usage = D3DUSAGE_DYNAMIC;
         pool  = D3DPOOL_DEFAULT;
      }
      else
      {
         usage = D3DUSAGE_WRITEONLY;
         pool  = D3DPOOL_MANAGED;
      }
      _device->CreateVertexBuffer(
         sizeInBytes,
         usage,
         0,
         pool,
         &d3dBuffer->_ptr,
         0
      );
      d3dBuffer->_sizeInBytes = sizeInBytes;
   }

   if( data )
   {
      // Copy data.
      void* d3dData;
      d3dBuffer->_ptr->Lock( 0, sizeInBytes, &d3dData, 0 );
      memcpy( d3dData, data, sizeInBytes );
      d3dBuffer->_ptr->Unlock();
   }

   return true;
}

//------------------------------------------------------------------------------
//!
void*
D3DManager::map( const RCP<IndexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes, const size_t sizeInBytes )
{
   D3DIndexBuffer* d3dBuffer = (D3DIndexBuffer*)buffer.ptr();
   void* d3dData;
   d3dBuffer->_ptr->Lock( offsetInBytes, sizeInBytes, &d3dData, 0 );
   return d3dData;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::unmap( const RCP<IndexBuffer>& buffer )
{
   D3DIndexBuffer* d3dBuffer = (D3DIndexBuffer*)buffer.ptr();
   HRESULT res = d3dBuffer->_ptr->Unlock();
   return res == D3D_OK;
}

//------------------------------------------------------------------------------
//!
void*
D3DManager::map( const RCP<VertexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes, const size_t sizeInBytes )
{
   D3DVertexBuffer* d3dBuffer = (D3DVertexBuffer*)buffer.ptr();
   void* d3dData;
   d3dBuffer->_ptr->Lock( offsetInBytes, sizeInBytes, &d3dData, 0 );
   return d3dData;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::unmap( const RCP<VertexBuffer>& buffer )
{
   D3DVertexBuffer* d3dBuffer = (D3DVertexBuffer*)buffer.ptr();
   HRESULT res = d3dBuffer->_ptr->Unlock();
   return res == D3D_OK;
}


//------------------------------------------------------------------------------
//!
RCP<Framebuffer>
D3DManager::createFramebuffer()
{
   return RCP<Framebuffer>( new Framebuffer() );
}

//------------------------------------------------------------------------------
//!
RCP<Geometry>
D3DManager::createGeometry( const PrimitiveType pt )
{
   return RCP<Geometry>( new D3DGeometry(pt) );
}

//------------------------------------------------------------------------------
//!
RCP<Shader>
D3DManager::createShader( ShaderType type, const String& code )
{
   return RCP<Shader>( new D3DShader( type, code, _device ) );
}

//------------------------------------------------------------------------------
//!
RCP<Program>
D3DManager::createProgram()
{
   return RCP<Program>( new D3DProgram() );
}

//------------------------------------------------------------------------------
//!
size_t
D3DManager::getConstants(
   const RCP<Program>&          program,
   ConstantBuffer::Container&   constants,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   DBG_BLOCK( os_d3d, "D3DManager::getConstants" );

   const D3DProgram* d3dProg = (const D3DProgram*)program.ptr();

   if( d3dProg->_vsTable == NULL )
   {
      DBG_MSG( os_d3d, "vsTable is NULL... Aborting." );
      return 0;
   }

   if( d3dProg->_psTable == NULL )
   {
      DBG_MSG( os_d3d, "psTable is NULL... Aborting." );
      return 0;
   }

   size_t  totalSize = 0;

   // Clear ignore groups.
   if( usedIgnoreGroup )
   {
      CHECK(ignoreGroups != NULL);
      // Align to chunks of 32 (if uint*) or 8 (if uchar*).
      uint32_t nBits = alignTo( (uint32_t)ignoreGroups->size(), (uint32_t)8*sizeof(*usedIgnoreGroup) );
      setbits( usedIgnoreGroup, 0, nBits, 0 );
   }

   totalSize += extractConstants(
      constants,
      totalSize,
      d3dProg->_vsTable,
      VERTEX_SHADER,
      ignoreGroups, usedIgnoreGroup
   );
   totalSize += extractConstants(
      constants,
      totalSize,
      d3dProg->_psTable,
      FRAGMENT_SHADER,
      ignoreGroups, usedIgnoreGroup
   );

   return totalSize;
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
D3DManager::createConstants( size_t size )
{
   return RCP<ConstantBuffer>( new ConstantBuffer( size ) );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
D3DManager::createConstants( const ConstantBuffer::Container& constants, size_t size )
{
   return RCP<ConstantBuffer>( new ConstantBuffer( constants, size ) );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
D3DManager::createConstants(
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
      return NULL;
   }
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
D3DManager::create1DTexture(
   const uint            width,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   // D3D doesn't support 1D textures, so we do Wx0.
   return create2DTexture( width, 0, tfmt, chOrder, flags );
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
D3DManager::create2DTexture(
   const uint            width,
   const uint            height,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   IDirect3DTexture9*  ptr;
   D3DFORMAT d3d_tfmt;
   TextureChannels d3d_co;
   toD3D( tfmt, chOrder, d3d_tfmt, d3d_co );
   DWORD d3d_usage = formatToUsage( d3d_tfmt, flags );

   if( isMipmapped(flags) && !isRenderable(flags) ) d3d_usage |= D3DUSAGE_AUTOGENMIPMAP;  //temp
   HRESULT err = _device->CreateTexture(
      width,
      height,
      isMipmapped(flags) ? 0 : 1, //Number of mip levels (0 means auto-determine)
      d3d_usage,
      d3d_tfmt,
      isRenderable(flags) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
      &ptr,
      NULL
   );

   if( err != D3D_OK )
   {
      printf("Error - D3DXCreateTexture failed\n");
      printf("--> %dx%d %s %s %s\n",
             width, height, toString(tfmt), toString(chOrder), toString(flags));
      return NULL;
   }

   D3DTexture* d3d_texture = new D3DTexture();
   d3d_texture->_ptr2D = ptr;
   d3d_texture->_ptr   = ptr;
   d3d_texture->set2D(width, height);
   d3d_texture->format(tfmt);
   d3d_texture->channelOrder(chOrder);
   d3d_texture->flags(flags);
   d3d_texture->_dstChannelOrder = d3d_co;

   RCP<Texture> texture(d3d_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
D3DManager::create3DTexture(
   const uint            width,
   const uint            height,
   const uint            depth,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   IDirect3DVolumeTexture9*  ptr;
   D3DFORMAT d3d_tfmt;
   TextureChannels d3d_co;
   toD3D(tfmt, chOrder, d3d_tfmt, d3d_co);
   HRESULT err = _device->CreateVolumeTexture(
      width,
      height,
      depth,
      isMipmapped(flags) ? 0 : 1, //Number of mip levels (0 means auto-determine)
      0, //0, D3DUSAGE_RENDERTARGET, or D3DUSAGE_DYNAMIC
      d3d_tfmt,
      D3DPOOL_MANAGED,
      &ptr,
      NULL
   );

   if( err != D3D_OK )
   {
      printf("Error - D3DXCreateTexture failed\n");
      return NULL;
   }

   D3DTexture* d3d_texture = new D3DTexture();
   d3d_texture->_ptr3D = ptr;
   d3d_texture->set3D(width, height, depth);
   d3d_texture->format(tfmt);
   d3d_texture->channelOrder(chOrder);
   d3d_texture->flags(flags);
   d3d_texture->_dstChannelOrder = d3d_co;

   RCP<Texture> texture(d3d_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
D3DManager::createCubeTexture(
   const uint            edgeLength,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   IDirect3DCubeTexture9*  ptr;
   D3DFORMAT d3d_tfmt;
   TextureChannels d3d_co;
   toD3D( tfmt, chOrder, d3d_tfmt, d3d_co );

   HRESULT err = _device->CreateCubeTexture(
      edgeLength,
      isMipmapped(flags) ? 0 : 1, //Number of mip levels (0 means auto-determine)
      0, //0, D3DUSAGE_RENDERTARGET, or D3DUSAGE_DYNAMIC
      d3d_tfmt,
      D3DPOOL_MANAGED,
      &ptr,
      NULL
   );

   if( err != D3D_OK )
   {
      printf("Error - D3DXCreateTexture failed\n");
      return NULL;
   }

   D3DTexture* d3d_texture = new D3DTexture();
   d3d_texture->_ptrCube = ptr;
   d3d_texture->setCubemap(edgeLength);
   d3d_texture->format(tfmt);
   d3d_texture->channelOrder(chOrder);
   d3d_texture->flags(flags);
   d3d_texture->_dstChannelOrder = d3d_co;

   RCP<Texture> texture(d3d_texture);
   return texture;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   D3DTexture* d3d_texture = (D3DTexture*)texture.ptr();

   if( d3d_texture->_ptr2D == NULL )
   {
      printf("Error - NULL texture pointer\n");
      return false;
   }

   // Lock surface and get pointer to data.
   D3DLOCKED_RECT lrect;
   HRESULT err = d3d_texture->_ptr2D->LockRect(
      level,
      &lrect,
      NULL,
      D3DLOCK_DISCARD
   );
   if( err != D3D_OK )
   {
      printf("Error - Could not lock texture\n");
      return false;
   }

   // Copy one row at a time.
   Gfx::TextureFormat format = d3d_texture->format();
   Gfx::TextureChannels channel = d3d_texture->channelOrder();
   Gfx::TextureChannels dst_channel = d3d_texture->_dstChannelOrder;
   uint width = d3d_texture->levelWidth(level);
   uint sy = d3d_texture->levelHeight(level);
   uint srcPitch = width * toBytes(format);
   for( uint y = 0; y < sy; ++y )
   {
      const uchar* srcP = (const uchar*)data + y*srcPitch;
      uchar* dstP = (uchar*)lrect.pBits + y*lrect.Pitch;
      //memcpy(dstP, srcP, srcPitch);
      if( !Gfx::convert( width, format, channel, srcP,
                                format, dst_channel, dstP) )
      {
         printf("Error - Problem while converting row #%d\n", y);
         assert(0);
         return false;
      }
   }

   // Unlock surface.
   err = d3d_texture->_ptr2D->UnlockRect(
      level
   );
   if( err != D3D_OK )
   {
      printf("Error - Could not unlock texture\n");
      return false;
   }

   if( level == 0 && !skipDefinedRegionUpdate )
   {
      d3d_texture->definedRegionX().setRange(0, d3d_texture->width());
      d3d_texture->definedRegionY().setRange(0, d3d_texture->height());
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          offset_x,
   const uint          width,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   D3DTexture* d3d_texture = (D3DTexture*)texture.ptr();

   if( d3d_texture->_ptr1D == NULL )
   {
      printf("Error - NULL texture pointer\n");
      return false;
   }

   // Lock surface and get pointer to data.
   D3DLOCKED_RECT lrect;
   RECT rect;
   rect.left = offset_x;
   rect.right = rect.left + width;
   rect.top = 0;
   rect.bottom = d3d_texture->height();
   HRESULT err = d3d_texture->_ptr1D->LockRect(
      level,
      &lrect,
      &rect,
      D3DLOCK_DISCARD
   );
   if( err != D3D_OK )
   {
      printf("Error - Could not lock texture\n");
      return false;
   }

   // Copy the data.
   TextureFormat format = d3d_texture->format();
   if( !Gfx::convert( width, format, d3d_texture->channelOrder(), (const uchar*)data,
                             format, d3d_texture->_dstChannelOrder, (uchar*)lrect.pBits) )
   {
      printf("Error - Problem while converting 1D texture\n");
      assert(0);
      return false;
   }

   // Unlock surface.
   err = d3d_texture->_ptr2D->UnlockRect(
      level
   );
   if( err != D3D_OK )
   {
      printf("Error - Could not unlock texture\n");
      return false;
   }

   if( level == 0 && !skipDefinedRegionUpdate )
   {
      d3d_texture->definedRegionX().update(offset_x);
      d3d_texture->definedRegionX().update(width);
      d3d_texture->definedRegionY().update(0);  //D3D doesn't have 1D textures per se
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setData(
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
   D3DTexture* d3d_texture = (D3DTexture*)texture.ptr();

   if( d3d_texture->_ptr2D == NULL )
   {
      printf("Error - NULL texture pointer\n");
      return false;
   }

   // Lock surface and get pointer to data.
   D3DLOCKED_RECT lrect;
   RECT rect;
   rect.left = offset_x;
   rect.right = rect.left + width;
   rect.top = offset_y;
   rect.bottom = rect.top + height;
   HRESULT err = d3d_texture->_ptr2D->LockRect(
      level,
      &lrect,
      &rect,
      D3DLOCK_DISCARD
   );
   if( err != D3D_OK )
   {
      printf("Error - Could not lock texture\n");
      return false;
   }

   // Copy one row at a time.
   TextureFormat format = d3d_texture->format();
   TextureChannels src_co = d3d_texture->channelOrder();
   TextureChannels dst_co = d3d_texture->_dstChannelOrder;
   uint sy = height;
   uint srcPitch = width * toBytes(d3d_texture->format());
   for( uint y = 0; y < sy; ++y )
   {
      const uchar* srcP = (const uchar*)data + y*srcPitch;
      uchar* dstP = (uchar*)lrect.pBits + y*lrect.Pitch;
      if( !Gfx::convert( width, format, src_co, srcP,
                                format, dst_co, dstP ) )
      {
         printf("Error - Problem while converting row #%d\n", y);
         assert(0);
         return false;
      }
   }

   // Unlock surface.
   err = d3d_texture->_ptr2D->UnlockRect(
      level
   );
   if( err != D3D_OK )
   {
      printf("Error - Could not unlock texture\n");
      return false;
   }

   if( level == 0 && !skipDefinedRegionUpdate )
   {
      d3d_texture->definedRegionX().update(offset_x);
      d3d_texture->definedRegionX().update(width);
      d3d_texture->definedRegionY().update(offset_y);
      d3d_texture->definedRegionY().update(height);
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setData(
   const RCP<Texture>& texture,
   const uint          level,
   const uint          offset_x,
   const uint          offset_y,
   const uint          offset_z,
   const uint          width,
   const uint          height,
   const uint          depth,
   const void*         data,
   const bool          skipDefinedRegionUpdate
)
{
   printf("D3DManager::setData on volume map is not implemented\n");
   // Need to loop one slice at a time.
   return false;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::generateMipmaps( const RCP<Texture>& texture )
{
   DBG_BLOCK( os_d3d, "D3DManager::generateMipmaps()" );
   if( texture->isMipmapped() )
   {
      D3DTexture* d3dTex = (D3DTexture*)texture.ptr();
      switch(d3dTex->type())
      {
         case TEX_TYPE_1D:
            DBG_MSG( os_d3d, "Generating for 1D texture" );
            d3dTex->_ptr1D->GenerateMipSubLevels();
            break;
         case TEX_TYPE_2D:
            DBG_MSG( os_d3d, "Generating for 2D texture" );
            if( !d3dTex->isRenderable() )
            {
               d3dTex->_ptr2D->GenerateMipSubLevels();
            }
            else
            {
               Gfx::TextureState tState;
               tState.setBilinear();
               //tState.setPointSampling();
               tState.clampX(Gfx::TEX_CLAMP_LAST);
               tState.clampY(Gfx::TEX_CLAMP_LAST);
               tState.mipFilter(Gfx::TEX_FILTER_NONE);
               RCP<Gfx::SamplerList> sampler = new Gfx::SamplerList();
               sampler->addSampler("tex", d3dTex, tState);
               //Program
               RCP<Gfx::Program> program( createProgram() );
               //program = RCP<Gfx::Program>( createProgram() ); FIXME WHY?
               program->addShader( createShader( Gfx::VERTEX_SHADER, sRTT_VS_2D ) );
               program->addShader( createShader( Gfx::FRAGMENT_SHADER, sRTT_PS_2D ) );
               uint numLevels = d3dTex->getNumLevelsFromSize();
               for( uint level = 1; level < numLevels; ++level )
               {
                  //printf("Level=%d\n", level);
                  tState->baseLevel(level-1);  //Level to be read
                  RCP<Gfx::Pass> pass = createFilterPass(sampler, program, d3dTex, level);
                  render(*pass);
               }
            }  break;
         case TEX_TYPE_3D:
            DBG_MSG( os_d3d, "Generating for 3D texture" );
            d3dTex->_ptr3D->GenerateMipSubLevels();
            break;
         case TEX_TYPE_CUBEMAP:
            DBG_MSG( os_d3d, "Generating for CubeMap texture" );
            d3dTex->_ptrCube->GenerateMipSubLevels();
            break;
         default:
            DBG_MSG( os_d3d, "ERROR - Invalid texture type" );
            assert(0);  //Unsupported texture type
            break;
      }
      return true;
   }
   else
   {
      DBG_MSG( os_d3d ,"ERROR - Trying to generate mipmaps on non-mipmapped texture" );
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::setFramebuffer( const Framebuffer* fb )
{
   DBG_BLOCK( os_d3d, "D3DManager::setFramebuffer" );
   const Framebuffer* d3d_fb = (const Framebuffer*)fb;

   HRESULT hr;
   if( d3d_fb )
   {
      // Use specified buffers
      if( d3d_fb->_colorBuffer.isValid() )
      {
         DBG_MSG( os_d3d, "Replacing render target (aka color buffer)" );
         if( _oldRenderTarget == NULL )
         {
            //Save current render target
            DBG_MSG( os_d3d, "Saving original render target" );
            _device->GetRenderTarget(0, &_oldRenderTarget);
         }

         const D3DTexture* d3d_color = (const D3DTexture*)d3d_fb->_colorBuffer.ptr();
         uint level = d3d_fb->_colorBufferLevel;
         DBG_MSG( os_d3d, "Binding color buffer with level " << level );
         LPDIRECT3DSURFACE9 colorSurface = NULL;
         hr = d3d_color->_ptr2D->GetSurfaceLevel(level, &colorSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - GetSurfaceLevel failed for color surface\n");
            return false;
         }
         hr = _device->SetRenderTarget(0, colorSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - SetRenderTarget failed for color surface\n");
            return false;
         }
         colorSurface->Release();
         d3d_fb->unsetDirty(Framebuffer::FB_BUFFER_BIT_COLOR);

         _curWidth  = d3d_fb->width();
         _curHeight = d3d_fb->height();
      }

      if( d3d_fb->_depthBuffer.isValid() )
      {
         DBG_MSG( os_d3d, "Replacing depth buffer" );
         if( _oldDepthStencilSurface == NULL )
         {
            DBG_MSG( os_d3d, "Saving original depth/stencil surface" );
            //Save current render target
            _device->GetDepthStencilSurface(&_oldDepthStencilSurface);
         }

         const D3DTexture* d3d_depth = (const D3DTexture*)d3d_fb->_depthBuffer.ptr();
         uint level = d3d_fb->_depthBufferLevel;
         DBG_MSG( os_d3d, "Binding depth buffer with level " << level );
         LPDIRECT3DSURFACE9 depthSurface = NULL;
         hr = d3d_depth->_ptr2D->GetSurfaceLevel(level, &depthSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - GetSurfaceLevel failed for depth surface\n");
            return false;
         }
         hr = _device->SetDepthStencilSurface(depthSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - SetDepthStencilSurface failed for depth surface\n");
            return false;
         }
         depthSurface->Release();
         d3d_fb->unsetDirty(Framebuffer::FB_BUFFER_BIT_DEPTH);

         _curWidth  = d3d_fb->width();
         _curHeight = d3d_fb->height();
      }

      if( d3d_fb->_stencilBuffer.isValid() && d3d_fb->isDirty(Framebuffer::FB_BUFFER_BIT_STENCIL) )
      {
         DBG_MSG( os_d3d, "Replacing stencil buffer" );
         if( _oldDepthStencilSurface == NULL )
         {
            DBG_MSG( os_d3d, "Saving original depth/stencil surface" );
            //Save current render target
            _device->GetDepthStencilSurface(&_oldDepthStencilSurface);
         }

         const D3DTexture* d3d_stencil = (const D3DTexture*)d3d_fb->_stencilBuffer.ptr();
         uint level = d3d_fb->_stencilBufferLevel;
         DBG_MSG( os_d3d, "Binding stencil buffer with level " << level );
         LPDIRECT3DSURFACE9 stencilSurface = NULL;
         hr = d3d_stencil->_ptr2D->GetSurfaceLevel(level, &stencilSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - GetSurfaceLevel failed for stencil surface\n");
            return false;
         }
         hr = _device->SetDepthStencilSurface(stencilSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - SetDepthStencilSurface failed for stencil surface\n");
            return false;
         }
         stencilSurface->Release();
         d3d_fb->unsetDirty(Framebuffer::FB_BUFFER_BIT_STENCIL);
      }

      _doingRTT = true;
   }
   else
   {
      // Use backbuffer as render target
      if( _oldRenderTarget != NULL )
      {
         // Restore the back buffer saved in the last (render-to-texture) pass
         hr = _device->SetRenderTarget(0, _oldRenderTarget);
         if( FAILED(hr) )
         {
            printf("ERROR - SetRenderTarget failed to restore\n");
            return false;
         }
         _oldRenderTarget = NULL;
         DBG_MSG( os_d3d, "Restored render target!" );
      }

      // Restore depth/stencil buffer
      if( _oldDepthStencilSurface != NULL )
      {
         // Restore the back buffer saved in the last (render-to-texture) pass
         hr = _device->SetDepthStencilSurface(_oldDepthStencilSurface);
         if( FAILED(hr) )
         {
            printf("ERROR - SetDepthStencilSurface failed to restore\n");
            return false;
         }
         _oldDepthStencilSurface = NULL;
         DBG_MSG( os_d3d, "Restored original depth/stencil surface!" );
      }
      _curWidth  = _width;
      _curHeight = _height;

      _doingRTT = false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::executeGeometry(
   const Geometry*     geom,
   const Program*      prog,
   const ConstantList* constants,
   const SamplerList*  samp,
   const float*        matrices[],
   const float*        camPosition,
   const uint*         range
)
{
   DBG_BLOCK( os_d3d, "D3DManager::executeGeometry" );

   if( geom == NULL )
   {
      DBG_MSG( os_d3d, "Empty geometry...  Aborting." );
      return false;
   }
   if( prog == NULL )
   {
      DBG_MSG( os_d3d, "Empty program...  Aborting." );
      return false;
   }

   // Shader.
   const D3DProgram* d3dProg = (const D3DProgram*)prog;
   if( _cache->_prog != d3dProg )
   {
      _cache->_prog = d3dProg;
      d3dProg->activate( _device );
   }

   // Matrices.
   if( d3dProg->_gfxConstants & D3DProgram::PROJ_MATRIX )
   {
      d3dProg->_vsTable->SetMatrixTranspose( _device, "gfxProjectionMatrix", (const D3DXMATRIX*)matrices[0] );
   }
   if( d3dProg->_gfxConstants & D3DProgram::VIEW_MATRIX )
   {
      d3dProg->_vsTable->SetMatrixTranspose( _device, "gfxViewMatrix", (const D3DXMATRIX*)matrices[1] );
   }
   if( d3dProg->_gfxConstants & D3DProgram::WORLD_MATRIX )
   {
      d3dProg->_vsTable->SetMatrixTranspose( _device, "gfxWorldMatrix", (const D3DXMATRIX*)matrices[2] );
   }
   if( d3dProg->_gfxConstants & D3DProgram::WV_MATRIX  )
   {
      Mat4f wv = (*(const Mat4f*)matrices[1]) * (*(const Mat4f*)matrices[2]);
      d3dProg->_vsTable->SetMatrixTranspose( _device, "gfxWorldViewMatrix", (const D3DXMATRIX*)wv.ptr() );
   }
   if( d3dProg->_gfxConstants & D3DProgram::WVP_MATRIX  )
   {
      Mat4f wvp = (*(const Mat4f*)matrices[3]) * (*(const Mat4f*)matrices[2]);
      d3dProg->_vsTable->SetMatrixTranspose( _device, "gfxWorldViewProjectionMatrix", (const D3DXMATRIX*)wvp.ptr() );
   }

   // Camera.
   if( d3dProg->_gfxConstants & D3DProgram::CAM_POS_VS )
   {
      if( camPosition != NULL )
      {
         d3dProg->_vsTable->SetFloatArray( _device, "gfxCameraPosition", camPosition, 3 );
      }
      else
      {
         printf("ERROR - Camera position required by vertex shader, but not defined in the pass\n");
      }
   }
   if( d3dProg->_gfxConstants & D3DProgram::CAM_POS_PS )
   {
      if( camPosition != NULL )
      {
         d3dProg->_psTable->SetFloatArray( _device, "gfxCameraPosition", camPosition, 3 );
      }
      else
      {
         printf("ERROR - Camera position required by pixel shader, but not defined in the pass\n");
      }
   }

   // Constants.
   if( constants )
   {
      const ConstantList::ContainerType& cbuffers = constants->buffers();
      for( uint i = 0; i < cbuffers.size(); ++i )
      {
         const uchar* cbuffer = cbuffers[i]->_buffer;
         const ConstantBuffer::Container& constDesc = cbuffers[i]->_constants;

         for( uint c = 0; c < constDesc.size(); ++c )
         {
            const char* name = constDesc[c].name().cstr();

            ID3DXConstantTable* constTable;
            switch( constDesc[c].shaderType() )
            {
               case VERTEX_SHADER:
                  constTable = d3dProg->_vsTable;
                  break;
               case FRAGMENT_SHADER:
                  constTable = d3dProg->_psTable;
                  break;
               case GEOMETRY_SHADER:
                  constTable = NULL; /* FIXME */
                  break;
               default:
                  printf("Error - Don't know shader type for '%s'.\n", name);
                  CHECK( false );
                  constTable = NULL;
                  break;
            }

            // FIXME: What about mat2x2 mat3x3???
            if( constDesc[c].count() == 0 )
            {
               switch( constDesc[c].type() )
               {
                  case CONST_FLOAT : constTable->SetFloat( _device, name, *(const float*)(cbuffer + constDesc[c].offset()) ); break;
                  case CONST_FLOAT2: constTable->SetFloatArray( _device, name, (const float*)(cbuffer + constDesc[c].offset()), 2 ); break;
                  case CONST_FLOAT3: constTable->SetFloatArray( _device, name, (const float*)(cbuffer + constDesc[c].offset()), 3 ); break;
                  case CONST_FLOAT4: constTable->SetFloatArray( _device, name, (const float*)(cbuffer + constDesc[c].offset()), 4 ); break;
                  case CONST_MAT2  : constTable->SetMatrixTranspose( _device, name, (const D3DXMATRIX*)(cbuffer + constDesc[c].offset()) ); break;
                  case CONST_MAT3  : constTable->SetMatrixTranspose( _device, name, (const D3DXMATRIX*)(cbuffer + constDesc[c].offset()) ); break;
                  case CONST_MAT4  : constTable->SetMatrixTranspose( _device, name, (const D3DXMATRIX*)(cbuffer + constDesc[c].offset()) ); break;
               }
            }
            else
            {
               UINT count = (UINT)constDesc[c].count();
               const float* data = *(const float**)(cbuffer + constDesc[c].offset());
               switch( constDesc[c].type() )
               {
                  case CONST_FLOAT : constTable->SetFloatArray( _device, name, (const float*)data, 1*count ); break;
                  case CONST_FLOAT2: constTable->SetFloatArray( _device, name, (const float*)data, 2*count ); break;
                  case CONST_FLOAT3: constTable->SetFloatArray( _device, name, (const float*)data, 3*count ); break;
                  case CONST_FLOAT4: constTable->SetFloatArray( _device, name, (const float*)data, 4*count ); break;
                  case CONST_MAT2  : constTable->SetMatrixTransposeArray( _device, name, (const D3DXMATRIX*)data, count); break;
                  case CONST_MAT3  : constTable->SetMatrixTransposeArray( _device, name, (const D3DXMATRIX*)data, count ); break;
                  case CONST_MAT4  : constTable->SetMatrixTransposeArray( _device, name, (const D3DXMATRIX*)data, count ); break;
               }
            }
         }
      }
   }

   // Textures.
   if( samp )
   {
      const SamplerList::ContainerType& samplers = samp->samplers();
      SamplerList::ContainerType::ConstIterator curSampler = samplers.begin();
      SamplerList::ContainerType::ConstIterator endSampler = samplers.end();
      for( ; curSampler != endSampler; ++curSampler )
      {
         const Sampler* s         = (*curSampler).ptr();
         int samplerID            = d3dProg->getSampler( s->name() );
         const D3DTexture* d3dTex = (const D3DTexture*)s->texture().ptr();

         _device->SetTexture( samplerID, d3dTex->_ptr );

         // Set texture parameters.
         const TextureState& ts = s->state();
         uint tsDiff            = ts.compare( _cache->_texState[samplerID] );

         if( tsDiff )
         {
            DWORD maxA;
            D3DTEXTUREFILTERTYPE magF, minF, mipF;
            toD3DFilters(ts, magF, minF, mipF, maxA);
            switch( tsDiff )
            {
               case 1:
                  _device->SetSamplerState( samplerID, D3DSAMP_MAGFILTER, magF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MINFILTER, minF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MIPFILTER, mipF );
                  break;
               case 2:
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSU, toD3D(ts.clampX()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSV, toD3D(ts.clampY()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSW, toD3D(ts.clampZ()) );
                  break;
               case 3:
                  _device->SetSamplerState( samplerID, D3DSAMP_MAGFILTER, magF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MINFILTER, minF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MIPFILTER, mipF );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSU, toD3D(ts.clampX()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSV, toD3D(ts.clampY()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSW, toD3D(ts.clampZ()) );
                  break;
               default:
                  _device->SetSamplerState( samplerID, D3DSAMP_MAGFILTER, magF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MINFILTER, minF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MIPFILTER, mipF );
                  _device->SetSamplerState( samplerID, D3DSAMP_MAXANISOTROPY, maxA );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSU, toD3D(ts.clampX()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSV, toD3D(ts.clampY()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_ADDRESSW, toD3D(ts.clampZ()) );
                  _device->SetSamplerState( samplerID, D3DSAMP_MAXMIPLEVEL, ts.baseLevel() );
                  //LOD Bias (takes a DWORD as the input?!?)
                  //_device->SetSamplerState( samplerID, D3DSAMP_MIPMAPLODBIAS, ts.LODBias() );
                  //Missing border color
            }
            _cache->_texState[samplerID] = ts;
         }
      }
   }


   // Geometry.
   const D3DGeometry* d3dGeom = (const D3DGeometry*)geom;
   const Geometry::BufferContainer& buffers = geom->buffers();

   // Set stream source.
   uint i   = 0;
   uint rev = 0;
   for( ; i < buffers.size(); ++i )
   {
      const D3DVertexBuffer* vbuffer = (const D3DVertexBuffer*)buffers[i].ptr();
      _device->SetStreamSource( i, vbuffer->_ptr, 0, vbuffer->_strideInBytes );
      rev += vbuffer->_revision;
   }
   for( ; i < _cache->_curNbStreams; ++i )
   {
      _device->SetStreamSource( i, 0, 0, 0 );
   }
   _cache->_curNbStreams = buffers.size();

   // Vertex declaration.
   _device->SetVertexDeclaration( d3dGeom->getVertexDeclaration( rev, _device ) );

   // Draw primitives.
   const D3DIndexBuffer* ibuffer = (const D3DIndexBuffer*)geom->indexBuffer().ptr();
   if( ibuffer )
   {
      // With an index buffer.
      _device->SetIndices( ibuffer->_ptr );

      size_t numVertices = buffers[0]->numVertices();
      if( range )
      {
         size_t numPrimitives = geom->numPrimitives( range[1] );
         _device->DrawIndexedPrimitive(
            toD3D( geom->primitiveType() ),
            0,
            0,
            numVertices,
            range[0],
            numPrimitives
         );
      }
      else
      {
         size_t numPrimitives = geom->numPrimitives();
         _device->DrawIndexedPrimitive(
            toD3D( geom->primitiveType() ),
            0,
            0,
            numVertices,
            0,
            numPrimitives
         );
      }
   }
   else
   {
      // With incrementing indices.
      _device->SetIndices( NULL );

      size_t numVertices = buffers[0]->numVertices();
      if( range )
      {
         size_t numPrimitives = geom->numPrimitives( range[1] );
         _device->DrawPrimitive(
            toD3D( geom->primitiveType() ),
            range[0],
            numPrimitives
         );
      }
      else
      {
         size_t numPrimitives = geom->numPrimitives();
         _device->DrawPrimitive(
            toD3D( geom->primitiveType() ),
            0,
            numPrimitives
         );
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
D3DManager::render( const Pass& pass )
{
   DBG_BLOCK( os_d3d, "D3DManager::render" );

   bool ok = true;

   ok &= setFramebuffer( pass._framebuffer.ptr() );

   bool colorRendering   = true;
   bool depthRendering   = true;
   bool stencilRendering = true;

   if( pass._framebuffer.isValid() )
   {
      if( pass._framebuffer->_colorBuffer.isNull() )
      {
         colorRendering = false;
      }
      if( pass._framebuffer->_depthBuffer.isNull() )
      {
         depthRendering = false;
      }
      if( pass._framebuffer->_stencilBuffer.isNull() )
      {
         stencilRendering = false;
      }
   }
   else
   {
      depthRendering   = false;
      stencilRendering = false;
   }


   D3DVIEWPORT9 viewport;
   viewport.MinZ = 0.0f;
   viewport.MaxZ = 1.0f;

   // Set viewport and scissor.
   viewport.X      = 0;
   viewport.Y      = 0;
   viewport.Width  = _curWidth;
   viewport.Height = _curHeight;
   _device->SetViewport( &viewport );

   RECT rect;
   rect.left   = 0;
   rect.top    = 0;
   rect.right  = _curWidth;
   rect.bottom = _curHeight;
   _device->SetScissorRect( &rect );

   // Clear buffers.
   if( pass._clearBuffers )
   {
      DWORD flags;
      switch( pass._clearBuffers )
      {
         case CLEAR_COLOR  : flags = D3DCLEAR_TARGET; break;
         case CLEAR_DEPTH  : flags = D3DCLEAR_ZBUFFER; break;
         case CLEAR_STENCIL: flags = D3DCLEAR_STENCIL; break;
         case CLEAR_COLOR | CLEAR_DEPTH   : flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER; break;
         case CLEAR_COLOR | CLEAR_STENCIL : flags = D3DCLEAR_TARGET | D3DCLEAR_STENCIL; break;
         case CLEAR_DEPTH | CLEAR_STENCIL : flags = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL; break;
         case CLEAR_ALL:
            flags = 0;
            if( colorRendering )
            {
               flags |= D3DCLEAR_TARGET;
            }
            if( depthRendering )
            {
               flags |= D3DCLEAR_ZBUFFER;
            }
            if( stencilRendering )
            {
               flags |= D3DCLEAR_STENCIL;
            }
            break;
         default:
            DBG_MSG( os_d3d, "Invalid _clearBuffers" );
            flags = 0;
            break;
      }
      _device->Clear(
         0,
         0,
         flags,
         D3DCOLOR_COLORVALUE( pass._color[0], pass._color[1], pass._color[2], pass._color[3] ),
         pass._z,
         pass._stencil
      );
   }

   // Set to default render states.
   applyState( alphaStateDef, _cache->_alphaState, _device );
   applyState( colorRendering ? colorStateDef : noColorStateDef, _cache->_colorState, _device );
   applyState( depthRendering ? depthStateDef : noDepthStateDef, _cache->_depthState, _device );
   applyState( cullStateDef, _cache->_cullState, _device );
   applyState( offsetStateDef, _cache->_offsetState, _device );
   applyState( stencilStateDef, _cache->_stencilState, _device );


   _device->BeginScene();

   // Keep current information in the pass.
   Mat4f proj( Mat4f::identity() );
   Mat4f viewProj( Mat4f::identity() );
   if( _doingRTT )
   {
      // Flip Y axis
      proj(1, 1)     = -1.0f;
      viewProj(1, 1) = -1.0f;
   }

   const Geometry*     curGeom  = 0;
   const Program*      curProg  = 0;
   const SamplerList*  curSamp  = 0;
   const ConstantList* curConst = 0;
   const float* curMatrices[4]  = { proj.ptr(), identityMatrix, identityMatrix, viewProj.ptr() };
   const float* curCamPosition  = 0;
   const uint*           range  = 0;
   bool                   wire  = false;


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
            const char* msg = (*curCmd)._cPtr;
            DBG_MSG( os_d3d, msg );
         } break;
         case Pass::PASS_CMD_RENDER:
         {
            // Save state (we reuse many variables existing in this scope).
            AlphaState alphaState     = _cache->_alphaState;
            ColorState colorState     = _cache->_colorState;
            CullState colorState      = _cache->_cullState;
            DepthState depthState     = _cache->_depthState;
            OffsetState offsetState   = _cache->_offsetState;
            StencilState stencilState = _cache->_stencilState;

            // Render node.
            const RCP<RenderNode> rn = (RenderNode*)(*curCmd)._data.ptr(); // Sneaky const_cast at the same time.
            Manager::render( rn );

            // Restore various states.
            // - Framebuffer.
            ok &= setFramebuffer( pass._framebuffer.ptr() );
            // - Viewport and scissor.
            _device->SetViewport( &viewport );
            _device->SetScissorRect( &rect );

            // - State.
            applyState( alphaState,   _cache->_alphaState, _device );
            applyState( colorState,   _cache->_colorState, _device );
            applyState( cullState,    _cache->_cullState, _device );
            applyState( depthState,   _cache->_depthState, _device );
            applyState( offsetState,  _cache->_offsetState, _device );
            applyState( stencilState, _cache->_stencilState, _device );
            // (no need to set clear color, as we have no CLEAR commands)
            // - Wireframe.
            if( wire )
            {
               _device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
            }
            else
            {
               _device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
            }
         } break;
         case Pass::PASS_CMD_SET_ALPHA_STATE:
         {
            applyState( *(const AlphaState*)(*curCmd)._data.ptr(), _cache->_alphaState, _device );
         } break;
         case Pass::PASS_CMD_SET_COLOR_STATE:
         {
            applyState( *(const ColorState*)(*curCmd)._data.ptr(), _cache->_colorState, _device );
         } break;
         case Pass::PASS_CMD_SET_CULL_STATE:
         {
            applyState( *(const CullState*)(*curCmd)._data.ptr(), _cache->_cullState, _device );
         } break;
         case Pass::PASS_CMD_SET_DEPTH_STATE:
         {
            applyState( *(const DepthState*)(*curCmd)._data.ptr(), _cache->_depthState, _device );
         } break;
         case Pass::PASS_CMD_SET_OFFSET_STATE:
         {
            applyState( *(const OffsetState*)(*curCmd)._data.ptr(), _cache->_offsetState, _device );
         } break;
         case Pass::PASS_CMD_SET_STENCIL_STATE:
         {
            applyState( *(const StencilState*)(*curCmd)._data.ptr(), _cache->_stencilState, _device );
         } break;
         case Pass::PASS_CMD_SET_WIREFRAME:
         {
            _device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
            wire = true;
         } break;
         case Pass::PASS_CMD_SET_FILL:
         {
            _device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
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
            // Keep it for the exec clause.
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
            // Keep it for the exec clause.
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
            // TODO
         } break;
         case Pass::PASS_CMD_COPY_DEPTH:
         {
            // TODO
         } break;
         case Pass::PASS_CMD_COPY_DEPTH_STENCIL:
         {
            // TODO
         } break;
         case Pass::PASS_CMD_COPY_STENCIL:
         {
            // TODO
         } break;
         case Pass::PASS_CMD_SET_SCISSOR:
         {
            const int* r   = (*curCmd)._iPtr;
            if( _doingRTT )
            {
               rect.top        = r[1];
               rect.bottom     = r[1] + r[3];
            }
            else
            {
               rect.top        = _curHeight - ( r[1] + r[3] );
               rect.bottom     = _curHeight - r[1];
            }
            rect.left       = r[0];
            rect.right      = r[0] + r[2];
            _device->SetScissorRect( &rect );
         } break;
         case Pass::PASS_CMD_SET_VIEWPORT:
         {
            const int* r   = (*curCmd)._iPtr;
            viewport.X      = r[0];
            viewport.Y      = _doingRTT ? r[1] : _curHeight - ( r[1] + r[3] );
            viewport.Width  = r[2];
            viewport.Height = r[3];
            DBG_MSG( os_d3d, "SetViewport(" << r[0] << "," << r[1] << "," << r[2] << "," << r[3]<<")" );
            DBG_MSG( os_d3d, " ... +{" << (int)viewport.X << "," << (int)viewport.Y << "},"
                                       << (int)viewport.Width << "x" << (int)viewport.Height );
            _device->SetViewport( &viewport );
         } break;
         case Pass::PASS_CMD_SET_PROJECTION:
         {
            // Transform matrix to d3d.
            float sfy = _doingRTT ? -1.0f : 1.0f;
            proj = Mat4f::translation( 0.0f, 0.0f, 0.5f ) *
                   Mat4f::scaling( 1.0f, sfy, 0.5f ) *
                  (*(const Mat4f*)(*curCmd)._fPtr);
            // Update view projection matrix.
            viewProj = proj * (*(const Mat4f*)curMatrices[1]);

         } break;
         case Pass::PASS_CMD_SET_VIEW:
         {
            curMatrices[1] = (*curCmd)._fPtr;
            // Update view projection matrix.
            viewProj = proj * (*(const Mat4f*)curMatrices[1]);
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
      ++curCmd;
   }

   if( wire )
   {
      _device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
   }

   _device->EndScene();

   if( _doingRTT )
   {
      if( pass._fbGenerateMipmaps )
      {
         DBG_MSG( os_d3d, "Render-to-texture pass finished - Generating mipmap chain where appropriate" );
         Framebuffer* fb = const_cast<Gfx::Framebuffer*>(pass._framebuffer.ptr());
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

   return ok;
}

#endif //GFX_D3D_SUPPORT
