/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_GLYPH_MANAGER_H
#define FUSION_GLYPH_MANAGER_H

#include <Fusion/StdDefs.h>

#include <CGMath/Vec2.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/Unicode.h>

NAMESPACE_BEGIN

class Bitmap;
class Character;
class Font;
class FontData;
class Text;

/*==============================================================================
  CLASS GlyphMaker
==============================================================================*/
class GlyphMaker:
   public RCObject
{
public:

   /*----- methods -----*/

   FUSION_DLL_API GlyphMaker( Font& font, FontData& fontData, bool scalable );
   FUSION_DLL_API virtual ~GlyphMaker();

   FUSION_DLL_API virtual String  getFontName() = 0;
   FUSION_DLL_API virtual   bool  getFontMetrics( float& maxAscender, float& maxDescender, float& lineGap ) = 0;
   FUSION_DLL_API virtual   bool  make( const char32_t* codepoints, size_t n, bool tight = false ) = 0;
   FUSION_DLL_API virtual   bool  make( char32_t codepoint, Character& character ) = 0;

   FUSION_DLL_API           bool  makeDefaultGlyphs();

protected:

   /*----- data members -----*/

   RCP<Font>  _font;
   FontData&  _fontData;
   bool       _scalable;
   float      _scalableFactor;
   //Vec2i      _texSize;
   //int        _texBorder;

   /*----- methods -----*/

   bool  toDistanceField( Bitmap& bmp, const Vec2i& pos, const Vec2i& size, int maxDist );

private:
}; //class GlyphMaker


namespace GlyphManager
{

enum GlyphMakerType
{
   DEFAULT,
   LIB_FREETYPE,
   STB_TRUETYPE
};

FUSION_DLL_API            void  createGlyph( Font& font, int fontSize, char32_t codepoint );
FUSION_DLL_API            void  generateMissingGlyphs();

FUSION_DLL_API RCP<GlyphMaker>  createGlyphMaker( Font& font, FontData& fontData, bool scalable, GlyphMakerType type = DEFAULT );
FUSION_DLL_API            void  updateGfx( Font& font, int fontSize );
FUSION_DLL_API            void  updateGfx( Font& font, int fontSize, uint sliceID );

FUSION_DLL_API            void  registerTextDrawable( Text* text );
FUSION_DLL_API            void  unregisterTextDrawable( Text* text );
FUSION_DLL_API            void  updateTextDrawables( Font* font );

FUSION_DLL_API     RCP<Bitmap>  createFontBitmap( const Vec2i& size = Vec2i(0) );

FUSION_DLL_API  GlyphMakerType  defaultGlyphMakerType();
FUSION_DLL_API            void  defaultGlyphMakerType( const GlyphMakerType type );

FUSION_DLL_API    const Vec2i&  defaultTexSize();
FUSION_DLL_API            void  defaultTexSize( const Vec2i& s );
FUSION_DLL_API             int  defaultTexBorder();
FUSION_DLL_API            void  defaultTexBorder( int b );

FUSION_DLL_API            bool  defaultScalable();
FUSION_DLL_API            void  defaultScalable( bool v );
FUSION_DLL_API             int  defaultScalableFontSize();
FUSION_DLL_API            void  defaultScalableFontSize( int s );
FUSION_DLL_API             int  defaultScalableMaxDistance();
FUSION_DLL_API            void  defaultScalableMaxDistance( int d );

FUSION_DLL_API            void  printInfo( TextStream& os = StdErr );

} // namespace GlyphManager

NAMESPACE_END

#endif //FUSION_GLYPH_MANAGER_H
