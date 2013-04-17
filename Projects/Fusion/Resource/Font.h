/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_FONT_H
#define FUSION_FONT_H

#include <Fusion/StdDefs.h>

#include <Fusion/Resource/RectPacker.h>

#include <CGMath/Vec2.h>

#include <Gfx/Tex/Sampler.h>

#include <Base/ADT/HashTable.h>
#include <Base/ADT/Map.h>
#include <Base/IO/Path.h>
#include <Base/Util/Packed.h>
#include <Base/Util/RCP.h>
#include <Base/Util/Unicode.h>

NAMESPACE_BEGIN

class Font;
class Texture;

/*==============================================================================
  CLASS Character
==============================================================================*/

//! Class representing a character

class Character
{

public:

   /*----- methods -----*/

   inline int       slice()       const { return _texSizeSlice.z(); }
   inline Vec2i     texSize()     const { return Vec2i(_texSizeSlice.xs(), _texSizeSlice.ys()); }
   inline Vec2i     texPosition() const { return Vec2i(_texPosition.x, _texPosition.y); }
   inline Vec2i     offsetH()     const { return Vec2i(_hOffsetAdvance.xs(), _hOffsetAdvance.ys()); }

   inline void  slice( int v )                { _texSizeSlice.z(v); }
   inline void  texSize( const Vec2i& v )     { _texSizeSlice.x(v.x); _texSizeSlice.y(v.y); }
   inline void  texPosition( const Vec2i& v ) { _texPosition = v; }

   inline void  setRect( int x, int y, int w, int h );
   inline void  setH( int dx, int dy, int adv );
   inline void  setV( int dx, int dy, int adv );

   // For glyph metrics nomenclature, see:
   //   http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-3.html
   inline Vec2i     glyphSize()     const { return texSize(); }
   inline int       glyphWidth()    const { return _texSizeSlice.xs(); }
   inline int       glyphHeight()   const { return _texSizeSlice.ys(); }

   inline int       glyphAdvanceH()  const { return _hOffsetAdvance.zs(); }
   inline int       glyphAscentH()   const { return _hOffsetAdvance.ys() + _texSizeSlice.ys(); }
   inline int       glyphDescentH()  const { return _hOffsetAdvance.ys(); }

   //inline int       glyphAdvanceV()  const { return _vOffsetAdvance.zs(); }
   //inline int       glyphAscentV()   const { return _vOffsetAdvance.ys() + _texSizeSlice.ys(); }
   //inline int       glyphDescentV()  const { return _vOffsetAdvance.ys(); }

   inline String  toStr() const;

private:

   /*----- data types -----*/

   typedef Vec2<int16_t>   Position;

   /*----- data members -----*/

   X11Y11Z10 _texSizeSlice;   //!< Size (x, y) and texture slice (z) of the glyph.
   Position  _texPosition;    //!< Bottom left corner of the glyph in the texture.
   X11Y11Z10 _hOffsetAdvance; //!< Offset (x, y) and advance (z) when writing horizontally.
   //X11Y11Z10 _vOffsetAdvance; //!< Offset (x, y) and advance (z) when writing vertically.
};

//-----------------------------------------------------------------------------
//!
void
Character::setRect( int x, int y, int w, int h )
{
   _texPosition = Position( x, y );
   _texSizeSlice.xy( w, h );
}

//-----------------------------------------------------------------------------
//!
void
Character::setH( int dx, int dy, int adv )
{
   _hOffsetAdvance.set( dx, dy, adv );
}

//-----------------------------------------------------------------------------
//!
String
Character::toStr() const
{
   return String().format("(%dx%d)@(%d,%d)[%d] h={%d,%d,%d}",
      glyphWidth(), glyphHeight(),
      _texPosition.x, _texPosition.y,
      slice(),
      _hOffsetAdvance.xs(), _hOffsetAdvance.ys(), _hOffsetAdvance.zs()
   );
}


/*==============================================================================
  CLASS CharInfo
==============================================================================*/
class CharInfo
{
public:
   inline const Character*  get( char ) const;
   inline const Character*  get( char32_t codepoint ) const;

   inline bool  has( char32_t codepoint ) const { return _char.has(codepoint); }

protected:
   friend class Font;
   friend class FontData;

   /*----- methods -----*/

   inline Character&  create( char32_t codepoint ) { return _char[codepoint]; }
   inline void  set( char32_t codepoint, const Character& ch ) { _char[codepoint] = ch; }

   /*----- types -----*/

   typedef HashTable<char32_t, Character>  CharacterTable;

   /*----- members -----*/

   CharacterTable  _char;

}; //class CharInfo

//------------------------------------------------------------------------------
//!
inline const Character*
CharInfo::get( char ch ) const
{
   return get( char32_t(ch&0xFF) );
}

//------------------------------------------------------------------------------
//!
inline const Character*
CharInfo::get( char32_t codepoint ) const
{
   auto cur = _char.find( codepoint );
   if( cur != _char.end() )
   {
      return &(*cur).second;
   }
   else
   {
      return nullptr;
   }
}


/*==============================================================================
  CLASS FontData
==============================================================================*/
class FontData:
   public RCObject
{
public:

   /*----- types -----*/

   enum
   {
      MAX_SLICES = 4
   };

   struct Slice
   {
      RCP<Bitmap>            _bitmap;
      RCP<Gfx::Texture>      _texture;
      RCP<Gfx::SamplerList>  _sampler;
   };

   /*----- methods -----*/

   FontData();
   FontData( int size );

   inline int   fontSize() const { return _fontSize; }
   //inline void  fontSize( int v );

   inline int  scalableDistance() const { return _scalableDistance; }

   // Font metrics.
   inline float  maxHeight()    const { return _maxHeight;    }
   inline float  maxAscender()  const { return _maxAscender;  }
   inline float  maxDescender() const { return _maxDescender; }
   inline float  lineGap()      const { return _lineGap;      }

   inline const CharInfo&  charInfo() const { return _charInfo; }

   inline       Character&  createCharacter( char32_t c ) { return _charInfo.create(c); }
                      bool  createCharacter( Character& dst );
   inline             void  set( char32_t codepoint, const Character& c ) { /*CHECK( !_charInfo.has(codepoint) );*/ _charInfo.set( codepoint, c ); }
   inline const Character*  get( char32_t codepoint ) const { return _charInfo.get(codepoint); }

   inline                         uint  numSlices()        const { return (uint)_slice.size(); }
   inline                 const Slice&  slice( uint i )    const { CHECK( i < numSlices()); return _slice[i]; }
   inline     const RCP<Gfx::Texture>&  texture( uint i )  const { return _slice[i]._texture; }
   inline const RCP<Gfx::SamplerList>&  samplers( uint i ) const { return _slice[i]._sampler; }
   inline                       Slice&  slice( uint i )          { CHECK( i < numSlices()); return _slice[i]; }
                                Slice&  createSlice( const Vec2i& s = Vec2i(0) );

   inline IncrementalRectPacker&  packer() { return _packer; }

protected:

   friend class CharInfo;
   friend class Font;

   /*----- data members -----*/

   int                    _fontSize;
   int                    _scalableDistance;
   float                  _maxHeight;
   float                  _maxAscender;
   float                  _maxDescender;
   float                  _lineGap;
   CharInfo               _charInfo;
   Vector<Slice>          _slice;
   IncrementalRectPacker  _packer;

   /*----- methods -----*/

   void  fontMetrics( float a, float d, float g );
   void  scalableDistance( int d );

   void  forceNewSlice();

private:
}; //class FontData


/*==============================================================================
  CLASS Font
==============================================================================*/

//! Font class

class Font:
   public RCObject
{

public:

   /*----- static methods -----*/

   static FUSION_DLL_API void  printInfo( TextStream& os );

   /*----- methods -----*/

   FUSION_DLL_API Font( const Path& path );

   FUSION_DLL_API bool  initFixed( int fontSize );
   FUSION_DLL_API bool  initScalable( int fontSize = -1, int maxDistance = -1 );

   inline const Path&  path() const { return _path; }

   // Data.
   inline       FontData*  data( int fontSize );
   inline const FontData*  data( int fontSize ) const;

   inline const CharInfo&  charInfo( int fontSize ) const { return data(fontSize)->charInfo(); }

   FUSION_DLL_API String  getInfo() const;

protected:

   /*----- methods -----*/

   virtual ~Font();

   void createGlyph( int fontSize, char32_t c ) const;

private:

   friend class FontCharInfo;

   /*----- data members -----*/
   typedef Map< int, RCP<FontData> >  DataTable;

   Path       _path;
   String     _name;
   DataTable  _data;
};

//-----------------------------------------------------------------------------
//!
inline FontData*
Font::data( int fontSize )
{
   auto it = _data.find( fontSize );
   return ( it != _data.end() ) ? (*it).second.ptr() : nullptr;
}

//-----------------------------------------------------------------------------
//!
inline const FontData*
Font::data( int fontSize ) const
{
   const auto it = _data.find( fontSize );
   return ( it != _data.end() ) ? (*it).second.ptr() : nullptr;
}


/*==============================================================================
  CLASS FontCharInfo
==============================================================================*/
class FontCharInfo
{
public:

   /*----- methods -----*/

   FUSION_DLL_API FontCharInfo( Font& font, int fontSize, bool scalable );

   inline const Character&  get( char32_t codepoint ) const
   {
      const Character* c = _char->get( codepoint );
      if( c == nullptr )
      {
         _font.createGlyph( _scalable ? 0 : _fontSize, codepoint ); // Ask to create the glyph.
         c = _char->get( char32_t(0) ); // Should always exist.
         CHECK( c );
      }
      return *c;
   }

   inline float  factor()       const { return _factor; }

   inline float  maxHeight()    const { return _factor*_fontData->maxHeight();    }
   inline float  maxAscender()  const { return _factor*_fontData->maxAscender();  }
   inline float  maxDescender() const { return _factor*_fontData->maxDescender(); }
   inline float  lineGap()      const { return _factor*_fontData->lineGap();      }

   // Utilities.
   FUSION_DLL_API float  getWidth( const char* str ) const;
   FUSION_DLL_API float  getHeight( const char* str, float& minY ) const;
   FUSION_DLL_API Vec2f  getSize( const char* str, float& minY ) const;
   //FUSION_DLL_API float  getWidthV( const char* str ) const;
   //FUSION_DLL_API float  getHeightV( const char* str ) const;


protected:

   /*----- data members -----*/

   const Font&      _font;
   int              _fontSize;
   bool             _scalable;
   float            _factor;
   const FontData*  _fontData;
   const CharInfo*  _char;

private:
}; //class FontCharInfo


NAMESPACE_END

#endif //FUSION_FONT_H
