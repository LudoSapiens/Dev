/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_IMAGE_H
#define FUSION_IMAGE_H

#include <Fusion/StdDefs.h>

#include <Fusion/Resource/Bitmap.h>

#include <Gfx/Tex/Texture.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS Image
==============================================================================*/

//! This class is not thread safe. If you call texture from another thread than
//! the main one, you risk some problem (crashing).
//! Also for now, after the texture is created, it's not kept up to date with
//! changes made to bitmap.

class Image:
   public RCObject
{
public:

   /*----- methods -----*/

   FUSION_DLL_API Image( const String& path = String() );
   FUSION_DLL_API Image( Gfx::Texture* t );
   FUSION_DLL_API Image( Bitmap* b );

   FUSION_DLL_API void init(
      const Vec2i&            dim,
      const Bitmap::PixelType type,
      const int               numChannels
   );
   FUSION_DLL_API void load();
   FUSION_DLL_API void load( const String& path );
   FUSION_DLL_API void loadCubemap( const String& path );
   FUSION_DLL_API void bitmap( Bitmap* bitmap );

   inline       Bitmap* bitmap()       { return _bitmap.ptr(); }
   inline const Bitmap* bitmap() const { return _bitmap.ptr(); }

   inline bool cubemap() const { return _bitmap.isValid() ? _bitmap->dimType() == Bitmap::DIM_CUBEMAP : false; }

   FUSION_DLL_API Gfx::Texture* texture();
   FUSION_DLL_API void invalidateTexture();

   inline const String&  sourcePath() const { return _sourcePath; }

protected:

   /*----- methods -----*/

   virtual ~Image();

   void createTexture();

   /*----- data members -----*/

   bool              _needUpdate;
   RCP<Bitmap>       _bitmap;
   RCP<Gfx::Texture> _texture;
   String            _sourcePath;
};

NAMESPACE_END

#endif
