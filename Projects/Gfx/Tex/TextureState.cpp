/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Tex/TextureState.h>

#include <cstdio>

USING_NAMESPACE

using namespace Gfx;


//------------------------------------------------------------------------------
//!
TextureState::TextureState():
   _fields(0)
{
   setDefaults();
}

//------------------------------------------------------------------------------
//!
//TextureState::~TextureState()
//{
//}

//------------------------------------------------------------------------------
//!
void
TextureState::setDefaults()
{
   minFilter(TEX_FILTER_LINEAR);
   magFilter(TEX_FILTER_LINEAR);
   zFilter(TEX_FILTER_LINEAR);
   mipFilter(TEX_FILTER_LINEAR);
   clampX(TEX_CLAMP_WRAP);
   clampY(TEX_CLAMP_WRAP);
   clampZ(TEX_CLAMP_WRAP);
   baseLevel(0);
   lastLevel(15);
   maxAniso(0);
   LODBias(0.0f);
}

//------------------------------------------------------------------------------
//!
void
TextureState::setPointSampling()
{
   minFilter(TEX_FILTER_POINT);
   magFilter(TEX_FILTER_POINT);
   zFilter(TEX_FILTER_MINMAG);
   mipFilter(TEX_FILTER_NONE);
}

//------------------------------------------------------------------------------
//!
void
TextureState::setBilinear()
{
   minFilter(TEX_FILTER_LINEAR);
   magFilter(TEX_FILTER_LINEAR);
   //zFilter(TEX_FILTER_POINT);
   mipFilter(TEX_FILTER_NONE);
}

//------------------------------------------------------------------------------
//!
void
TextureState::setTrilinear()
{
   minFilter(TEX_FILTER_LINEAR);
   magFilter(TEX_FILTER_LINEAR);
   //zFilter(TEX_FILTER_POINT);
   mipFilter(TEX_FILTER_LINEAR);
}

//------------------------------------------------------------------------------
//!
void
TextureState::setVolumeTrilinear()
{
   minFilter(TEX_FILTER_LINEAR);
   magFilter(TEX_FILTER_LINEAR);
   zFilter(TEX_FILTER_LINEAR);
   //mipFilter(TEX_FILTER_POINT);
}

//------------------------------------------------------------------------------
//!
void
TextureState::print()
const
{
   printf("TextureState:\n");
   printf("=============\n");
   printf("MinF: %s  MagF: %s  ZF: %s  MipF: %s\n",
          toStr(minFilter()).cstr(),
          toStr(magFilter()).cstr(),
          toStr(zFilter()).cstr(),
          toStr(mipFilter()).cstr());
   printf("ClampX: %s  ClampY: %s  ClampZ: %s\n",
          toStr(clampX()).cstr(),
          toStr(clampY()).cstr(),
          toStr(clampZ()).cstr());
   printf("BaseLevel: %d  MaxAniso: %d  LodBias: %g\n",
          baseLevel(), maxAniso(), LODBias());
   printf("=============\n");
}

//------------------------------------------------------------------------------
//!
String Gfx::toStr( const TexFilter f )
{
   switch(f)
   {
      case TEX_FILTER_NONE  : return String("none/minmag");
      case TEX_FILTER_POINT : return String("point");
      case TEX_FILTER_LINEAR: return String("linear");
      case TEX_FILTER_CUBIC : return String("cubic");
      default               : return String("<invalid>");
   }
}

//------------------------------------------------------------------------------
//!
String Gfx::toStr( const TexClamp c )
{
   switch(c)
   {
      case TEX_CLAMP_WRAP             : return String("wrap");
      case TEX_CLAMP_MIRROR           : return String("mirror");
      case TEX_CLAMP_LAST             : return String("last");
      case TEX_CLAMP_BORDER           : return String("border");
      case TEX_CLAMP_MIRRORONCE_LAST  : return String("mirroronce_last");
      case TEX_CLAMP_MIRRORONCE_BORDER: return String("mirroronce_border");
      default                         : return String("<invalid>");
   }
}
