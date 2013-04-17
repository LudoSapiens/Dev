/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/Font.h>
#include <Fusion/Resource/Bitmap.h>
#include <Fusion/Resource/GlyphManager.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/Core/Core.h>

#include <CGMath/CGMath.h>
#include <CGMath/Math.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/MT/Task.h>
#include <Base/Util/UnicodeIterator.h>


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_font, "Font" );

UNNAMESPACE_END


NAMESPACE_BEGIN


/*==============================================================================
  CLASS CharInfo
==============================================================================*/



/*==============================================================================
  CLASS FontData
==============================================================================*/

//-----------------------------------------------------------------------------
//!
FontData::FontData( int size ):
   _fontSize( size ),
   _scalableDistance( 0 ),
   _packer( GlyphManager::defaultTexSize().x, GlyphManager::defaultTexBorder() )
{
}

//-----------------------------------------------------------------------------
//!
FontData::FontData():
   _fontSize( -1 ),
   _scalableDistance( 0 ),
   _packer( GlyphManager::defaultTexSize().x, GlyphManager::defaultTexBorder() )
{
}

//-----------------------------------------------------------------------------
//!
void
FontData::fontMetrics( float a, float d, float g )
{
   _maxAscender  = a;
   _maxDescender = d;
   _lineGap      = g;
   _maxHeight    = _maxAscender - _maxDescender;
}

//-----------------------------------------------------------------------------
//!
void
FontData::scalableDistance( int d )
{
   _scalableDistance = d;
}

//-----------------------------------------------------------------------------
//! Uses the size of the character to allocate new storage into the current slice.
//! If no space is left, a new slice is created, and the character is placed there.
//! This function replaces the slice and texPosition values in the character.
bool
FontData::createCharacter( Character& c )
{
   Vec2i p = _packer.add( c.texSize() );

   // Check is the specified character fits in the last slice.
   uint lastSliceHeight = 0;
   if( !_slice.empty() )
   {
      CHECK( _slice.back()._bitmap.isValid() );
      lastSliceHeight = _slice.back()._bitmap->height();
   }

   // Create a new slice if necessary.
   if( _packer.height() > lastSliceHeight )
   {
      Slice& slice = createSlice();
      _packer.reset( slice._bitmap->width() );
      p = _packer.add( c.texSize() );
      if( _packer.height() > uint(slice._bitmap->height()) )
      {
         // Means the character is bigger than the whole slice.
         StdErr << "ERROR - Character of size " << c.texSize() << " doesn't seem to fit texture of " << slice._bitmap->dimension() << nl;
         CHECK( false );
         return false;
      }
   }

   // Store the newly generated info.
   c.slice( numSlices() - 1 );
   c.texPosition( p );

   return true;
}

//-----------------------------------------------------------------------------
//!
FontData::Slice&
FontData::createSlice( const Vec2i& s )
{
   CHECK( numSlices() < MAX_SLICES );
   _slice.pushBack( Slice() );
   _slice.back()._bitmap = GlyphManager::createFontBitmap( s );
   return _slice.back();
}

//-----------------------------------------------------------------------------
//!
void
FontData::forceNewSlice()
{
   _packer.add( _slice.back()._bitmap->dimension()(0,1) );
}


/*==============================================================================
  CLASS Font
==============================================================================*/

//------------------------------------------------------------------------------
//!
void Font::printInfo( TextStream& os )
{
   GlyphManager::printInfo( os );
}

//------------------------------------------------------------------------------
//!
Font::Font( const Path& path ):
   _path( path )
{
   DBG_BLOCK( os_font, "Font::Font(" << path.string() << ")" );
}

//------------------------------------------------------------------------------
//!
Font::~Font()
{
}

//-----------------------------------------------------------------------------
//!
bool
Font::initFixed( int fontSize )
{
   DBG_BLOCK( os_font, "Font::initFixed()" );

   RCP<FontData>&  fontData = _data[fontSize];
   if( fontData.isValid() )  return false;

   fontData = new FontData( fontSize );

   bool ok = true;

   RCP<GlyphMaker> maker = GlyphManager::createGlyphMaker( *this, *fontData, false );

   if( _name.empty() )
   {
      _name = maker->getFontName();
   }

   // Set font metric values.
   float a, d, g;
   maker->getFontMetrics( a, d, g );
   fontData->fontMetrics( CGM::round(a), CGM::round(d), CGM::round(g) );
   //StdErr << getInfo() << "@" << fontSize << " FontMetrics(" << Vec3f(a,d,g) << ")" << nl;

   // Generate the default glyphs.
   ok &= maker->makeDefaultGlyphs();

   GlyphManager::updateGfx( *this, fontSize );

   fontData->forceNewSlice();  // Start subsequent glyphs in a new slice.

   return ok;
}

//-----------------------------------------------------------------------------
//!
bool
Font::initScalable( int fontSize, int maxDistance )
{
   DBG_BLOCK( os_font, "Font::initScalable()" );

   if( fontSize    <= 0 )  fontSize    = GlyphManager::defaultScalableFontSize();
   if( maxDistance <  0 )  maxDistance = GlyphManager::defaultScalableMaxDistance();

   RCP<FontData>&  fontData = _data[0];
   if( fontData.isValid() )
   {
      CHECK( fontSize == fontData->fontSize() );
      return false;
   }

   fontData = new FontData( fontSize );
   fontData->scalableDistance( maxDistance );

   bool ok = true;

   RCP<GlyphMaker> maker = GlyphManager::createGlyphMaker( *this, *fontData, true );

   if( _name.empty() )
   {
      _name = maker->getFontName();
   }

   // Set font metric values.
   float a, d, g;
   maker->getFontMetrics( a, d, g );
   fontData->fontMetrics( CGM::round(a), CGM::round(d), CGM::round(g) );
   //StdErr << getInfo() << "@" << fontSize << " FontMetrics(" << Vec3f(a,d,g) << ")" << nl;

   // Generate the default glyphs.
   ok &= maker->makeDefaultGlyphs();

   GlyphManager::updateGfx( *this, 0 );

   fontData->forceNewSlice();  // Start subsequent glyphs in a new slice.

   return ok;
}

//-----------------------------------------------------------------------------
//!
String
Font::getInfo() const
{
   return ResManager::getFontName( this );
}

//-----------------------------------------------------------------------------
//!
void
Font::createGlyph( int fontSize, char32_t codepoint ) const
{
   GlyphManager::createGlyph( *(const_cast<Font*>(this)), fontSize, codepoint );
}


/*==============================================================================
  CLASS FontCharInfo
==============================================================================*/

//------------------------------------------------------------------------------
//!
FontCharInfo::FontCharInfo( Font& font, int fontSize, bool scalable ):
   _font( font ),
   _fontSize( fontSize ),
   _scalable( scalable )
{
   if( scalable )
   {
      _fontData = _font.data( 0 );
      if( _fontData == nullptr )
      {
         font.initScalable();
         _fontData = _font.data( 0 );
         CHECK( _fontData != nullptr );
      }
      _factor = float(fontSize) / _fontData->fontSize();
   }
   else
   {
      _fontData = _font.data( fontSize );
      if( _fontData == nullptr )
      {
         font.initFixed( _fontSize );
         _fontData = _font.data( fontSize );
         CHECK( _fontData != nullptr );
      }
      _factor = 1.0f;
   }
   _char = &(_fontData->charInfo());
}

//-----------------------------------------------------------------------------
//!
float
FontCharInfo::getWidth( const char* str ) const
{
   float tmp = 0.0f;
   for( UTF8Iterator it = str; it(); ++it )
   {
      const Character& ch = get( *it );
      tmp += (float)ch.glyphAdvanceH();
   }
   tmp *= _factor;
   return tmp;
}

//-----------------------------------------------------------------------------
//!
float
FontCharInfo::getHeight( const char* str, float& minY ) const
{
   float maxY = -CGConstf::infinity();
         minY =  CGConstf::infinity();
   for( UTF8Iterator it = str; it(); ++it )
   {
      const Character& ch = get( *it );
      maxY = CGM::max( maxY, (float)ch.glyphAscentH()  );
      minY = CGM::min( minY, (float)ch.glyphDescentH() );
   }
   maxY *= _factor;
   minY *= _factor;
   return maxY-minY+1.0f; // TEMP: is the +1 required?
}

//-----------------------------------------------------------------------------
//!
Vec2f
FontCharInfo::getSize( const char* str, float& minY ) const
{
   float tmp  = 0.0f;
   float maxY = -CGConstf::infinity();
         minY =  CGConstf::infinity();
   for( UTF8Iterator it = str; it(); ++it )
   {
      const Character& ch = get( *it );
      tmp += (float)ch.glyphAdvanceH();
      maxY = CGM::max( maxY, (float)ch.glyphAscentH()  );
      minY = CGM::min( minY, (float)ch.glyphDescentH() );
   }
   tmp  *= _factor;
   maxY *= _factor;
   minY *= _factor;
   return Vec2f( tmp, maxY-minY+1.0f ); // TEMP: is the +1 required?
}


NAMESPACE_END
