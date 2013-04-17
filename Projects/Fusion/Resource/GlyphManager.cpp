/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/GlyphManager.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Core/TaskEvent.h>
#include <Fusion/Drawable/Text.h>
#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/Resource/Font.h>
#include <Fusion/Resource/ResManager.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/Set.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/MT/Atomic.h>
#include <Base/MT/Lock.h>
#include <Base/MT/Task.h>
#include <Base/MT/TaskQueue.h>
#include <Base/MT/Thread.h>
#include <Base/Util/Platform.h>
#include <Base/Util/Timer.h>
#include <Base/Util/UnicodeIterator.h>

#include <algorithm>

#ifndef FUSION_LA_TEXTURE
#define FUSION_LA_TEXTURE 0
#endif

#ifndef FUSION_PROFILE_GLYPHS
// 0: Disabled, 1: Font timings, 2: Glyph timings
#define FUSION_PROFILE_GLYPHS 0
#endif

#ifndef FUSION_USE_LIB_FREETYPE
#if defined(__ANDROID__)
#define FUSION_USE_LIB_FREETYPE 0  // Default for Android
#elif PLAT_MACOSX
#define FUSION_USE_LIB_FREETYPE 0  // Default for Mac
#else
#define FUSION_USE_LIB_FREETYPE 0  // Default for other platforms
#endif
#endif

#ifndef FUSION_USE_STB_TRUETYPE
#define FUSION_USE_STB_TRUETYPE 1
#endif

// Overrides.
//#define FUSION_USE_LIB_FREETYPE 1
//#define FUSION_USE_STB_TRUETYPE 0

#if FUSION_USE_LIB_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftsnames.h>
#include <freetype/ttnameid.h>
#endif

#if FUSION_USE_STB_TRUETYPE
#include <cstdlib>

inline void* mallocSTB( size_t s, void* )
{
   return malloc( s );
}

inline void freeSTB( void* ptr, void* )
{
   free( ptr );
}

#define STBTT_malloc(x,u)    mallocSTB(x,u)
#define STBTT_free(x,u)      freeSTB(x,u)
#define STB_TRUETYPE_IMPLEMENTATION  // force the following include to generate implementation
#include "stb_truetype.h"
#include <Base/IO/FileSystem.h>
#endif


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_font, "Font" );

/*----- data types -----*/

typedef Pair< RCP<Font>, int >         FontPair;
typedef Set< char32_t >                CodeList;
typedef Map< FontPair, CodeList >      FontCodeList;
typedef Pair< char32_t, Character >    CodeChar;
typedef Vector< CodeChar >             CodeCharList;
typedef Map< FontPair, CodeCharList >  FontCodeCharList;

// Prototypes.
Vector<char32_t>  defaultGlyphs();


//==================
// STATIC VARIABLES
//==================

Vec2i _defaultTexSize             = Vec2i( 1024 );
int   _defaultTexBorder           =    1;
bool  _defaultScalable            = true;
int   _defaultScalableFontSize    =   32;
int   _defaultScalableMaxDistance =    8;

Vector<char32_t>  _defaultGlyphs  = defaultGlyphs();
Vector<Text*>     _textDrawables;
Lock              _lock;
FontCodeList      _fonts;
AtomicInt32       _taskRunning;

// Default glyph maker type.
#if   FUSION_USE_LIB_FREETYPE
GlyphManager::GlyphMakerType  _defaultGlyphMakerType = GlyphManager::LIB_FREETYPE;
#elif FUSION_USE_STB_TRUETYPE
GlyphManager::GlyphMakerType  _defaultGlyphMakerType = GlyphManager::STB_TRUETYPE;
#else
#error Neither FUSION_USE_FREETYPE nor FUSION_USE_STB_TRUETYPE are set.
#endif


//================
// IMPLEMENTATION
//================

/*==============================================================================
  CLASS GlyphUpdateEvent
==============================================================================*/
class GlyphUpdateEvent:
   public TaskEvent
{
public:

   /*----- methods -----*/

   //GlyphUpdateEvent();
   //virtual ~GlyphUpdateEvent();

   virtual void  execute();

   inline CodeCharList&  font( Font& font, int fontSize ) { return _fonts[FontPair(&font, fontSize)]; }

   void  print( TextStream& os = StdErr ) const;

protected:

   /*----- data members -----*/

   FontCodeCharList  _fonts;

private:
}; //class GlyphUpdateEvent

//-----------------------------------------------------------------------------
//!
void
GlyphUpdateEvent::execute()
{
   for( FontCodeCharList::ConstIterator curF = _fonts.begin(),
                                        endF = _fonts.end();
        curF != endF;
        ++curF )
   {
      const FontPair& fontPair = ((*curF).first);
      Font*         font = fontPair.first.ptr();
      FontData* fontData = font->data( fontPair.second );

      // Register the Characters in every font.
      for( CodeCharList::ConstIterator curCC = (*curF).second.begin(),
                                       endCC = (*curF).second.end();
           curCC != endCC;
           ++curCC )
      {
         fontData->set( (*curCC).first, (*curCC).second );
      }

      // Refresh touched slices' textures.
      GlyphManager::updateGfx( *font, fontPair.second );

      // Update Text drawables using touched font slices.
      GlyphManager::updateTextDrawables( font );
   }

   CHECK( _taskRunning == 1 );
   --_taskRunning; // Allow another task to be created (OK, since the outstanding Characters are all registered).
}

//-----------------------------------------------------------------------------
//!
void
GlyphUpdateEvent::print( TextStream& os ) const
{
   for( FontCodeCharList::ConstIterator curF = _fonts.begin(),
                                        endF = _fonts.end();
        curF != endF;
        ++curF )
   {
      const FontPair& fontPair = (*curF).first;
      os << fontPair.first->getInfo() << ":" << nl;
      for( CodeCharList::ConstIterator curCC = (*curF).second.begin(),
                                       endCC = (*curF).second.end();
           curCC != endCC;
           ++curCC )
      {
         os << "  " << (*curCC).first << ": " << (*curCC).second.toStr() << nl;
      }
   }
}


/*==============================================================================
  CLASS GlyphMakingTask
==============================================================================*/
class GlyphMakingTask:
   public Task
{
public:

   /*----- methods -----*/

   GlyphMakingTask() {}
   ~GlyphMakingTask() {}

   virtual void execute();

protected:

   /*----- data members -----*/

   /* members... */

   /*----- methods -----*/

   /* methods... */

private:
}; //class GlyphMakingTask

//-----------------------------------------------------------------------------
//!
void
GlyphMakingTask::execute()
{
   //Thread::sleep(1.0); // TEMP: Add some fake latency.
   //Thread::sleep(1.0/128.0); // TEMP: Add some fake latency.

   FontCodeList  fonts;

   // Atomically retrieve all of the glyphs to create.
   {
      LockGuard guard( _lock );
      _fonts.swap( fonts );
   }

   if( fonts.empty() )  return;

   // Create every glyph, one after the other.
   RCP<GlyphUpdateEvent> event = new GlyphUpdateEvent();
   for( FontCodeList::ConstIterator curF = fonts.begin(),
                                    endF = fonts.end();
        curF != endF;
        ++curF )
   {
      const FontPair&  fontPair = (*curF).first;
      const RCP<Font>&     font = fontPair.first;
      FontData*        fontData = font->data( fontPair.second );
      const CodeList&     codes = (*curF).second;
      RCP<GlyphMaker>     maker = GlyphManager::createGlyphMaker( *font, *fontData, (fontData->fontSize() != fontPair.second) );
      if( maker.isNull() )
      {
         StdErr << "ERROR - Could not create glyph manager." << nl;
         continue;
      }
      CodeCharList&  codeChar = event->font( *font, fontPair.second );
      for( CodeList::ConstIterator curG = codes.begin(),
                                   endG = codes.end();
           curG != endG;
           ++curG )
      {
         char32_t codepoint = *curG;
         codeChar.pushBack( CodeChar() );
         codeChar.back().first = codepoint;
         if( !maker->make( codepoint, codeChar.back().second ) )
         {
            StdErr << "ERROR - Could not create glyph " << codepoint << " for font " << font->getInfo() << nl;
            continue;
         }
      }
      maker = NULL;
   }

   Core::submitTaskEvent( event );

   CHECK( _taskRunning == 1 );
   //--_taskRunning; // Allow another task to be created (need to wait for the characters to be registered).
}


#if FUSION_USE_LIB_FREETYPE

//-----------------------------------------------------------------------------
//!
inline float conv_26_6( FT_Pos v )
{
   float tmp = float(v);
   tmp *= (1.0f/64.0f);
   return tmp;
}

//-----------------------------------------------------------------------------
//!
//inline float conv_16_16( FT_Pos v )
//{
//   float tmp = float(v);
//   tmp *= (1.0f/65536.0f);
//   return tmp;
//}

//-----------------------------------------------------------------------------
//!
inline int  round_26_6( FT_Pos v )
{
   return int( (v + (1<<5)) >> 6 );
}

//-----------------------------------------------------------------------------
//!
inline int  round_16_16( FT_Pos v )
{
   return int( (v + (1<<15)) >> 16 );
}

#if 0
//-----------------------------------------------------------------------------
//!
inline int  floor_26_6( FT_Pos v )
{
   return int( v >> 6 );
}

//-----------------------------------------------------------------------------
//!
inline int  floor_16_16( FT_Pos v )
{
   return int( v >> 16 );
}

#endif

/*==============================================================================
  CLASS GlyphMakerLibFreetype
==============================================================================*/
class GlyphMakerLibFreetype:
   public GlyphMaker
{
public:

   /*----- methods -----*/

   GlyphMakerLibFreetype( Font& font, FontData& fontData, bool scalable );
   virtual ~GlyphMakerLibFreetype();

   virtual String  getFontName();
   virtual   bool  getFontMetrics( float& maxAscender, float& maxDescender, float& lineGap );
   virtual   bool  make( const char32_t* codepoints, size_t n, bool tight = false );
   virtual   bool  make( char32_t codepoint, Character& character );

protected:

   /*----- data members -----*/

   FT_Library  _library;
   FT_Face     _face;

   /*----- methods -----*/

   bool   init();
   bool   term();
   Vec2i  getGlyphSize( char32_t codepoint );
   void   drawGlyph( int x, int y, const FT_Bitmap& src, Bitmap& dst );
   bool   createGlyph( char32_t codepoint, Character& character );
   bool   createGlyph( char32_t codepoint, Character& character, Bitmap& dst, IncrementalRectPacker& packer );

private:
}; //class GlyphMakerLibFreetype

//-----------------------------------------------------------------------------
//!
GlyphMakerLibFreetype::GlyphMakerLibFreetype( Font& font, FontData& fontData, bool scalable ):
   GlyphMaker( font, fontData, scalable )
{
   if( !init() )
   {
      CHECK( false );
   }
}

//-----------------------------------------------------------------------------
//!
GlyphMakerLibFreetype::~GlyphMakerLibFreetype()
{
   if( !term() )
   {
      CHECK( false );
   }
}

//-----------------------------------------------------------------------------
//!
String
GlyphMakerLibFreetype::getFontName()
{
   FT_SfntName name;
   FT_UInt n = FT_Get_Sfnt_Name_Count( _face );
   for( FT_UInt i = 0; i < n; ++i )
   {
      FT_Error error = FT_Get_Sfnt_Name( _face, 1, &name );
      if( error )  continue;
      if( name.platform_id == TT_PLATFORM_APPLE_UNICODE &&
          name.encoding_id == TT_MAC_ID_ROMAN           &&
          name.language_id == TT_MAC_LANGID_ENGLISH     &&
          name.name_id == 1 )
      {
         // Convert to UTF8.
         String tmp = String( (const char*)name.string, name.string_len, UTF16BE );
         return tmp;
      }
      else
      if( name.platform_id == TT_PLATFORM_MACINTOSH &&
          name.encoding_id == TT_MAC_ID_ROMAN       &&
          name.language_id == TT_MAC_LANGID_ENGLISH &&
          name.name_id == 1 )
      {
         // TODO: Convert from Latin1 to UTF8.
         return String( (const char*)name.string, name.string_len );
      }
   }
   return String();
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerLibFreetype::getFontMetrics( float& maxAscender, float& maxDescender, float& lineGap )
{
   const FT_Size_Metrics sizeMetrics = _face->size->metrics;
   maxAscender  = conv_26_6( sizeMetrics.ascender  );
   maxDescender = conv_26_6( sizeMetrics.descender );
   lineGap      = conv_26_6( sizeMetrics.height    );
   return true;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerLibFreetype::make( const char32_t* codepoints, size_t n, bool tight )
{
   bool ok = true;

   int b2 = _fontData.scalableDistance()*2;

   if( tight )
   {
      // Determine required Bitmap size.
      IncrementalRectPacker tmpPacker( _defaultTexSize.x, _defaultTexBorder );
      for( size_t i = 0; i < n; ++i )
      {
         char32_t codepoint = codepoints[i];
         Vec2i s = getGlyphSize( codepoint );
         if( s.x >= 0 )  tmpPacker.add( s + b2 );
      }

      // Allocate memory for destination and temporaries.
      _fontData.createSlice( tmpPacker.dimension() );
   }

   // Create all of the glyphs, and store them in the bitmap.
   for( size_t i = 0; i < n; ++i )
   {
      char32_t codepoint = codepoints[i];
      Vec2i s = getGlyphSize( codepoint );
      Character character;
      character.texSize( s + b2 );
      if( s.x >= 0 )
      {
         if( !_fontData.createCharacter( character ) )
         {
            ok = false;
            _fontData.set( codepoint, *_fontData.get(0) ); // TODO: Defer assignment to Gfx thread.
            continue;
         }
      }
      if( !createGlyph( codepoint, character ) )
      {
         ok = false;
         _fontData.set( codepoint, *_fontData.get(0) ); // TODO: Defer assignment to Gfx thread.
         continue;
      }
      _fontData.set( codepoint, character ); // TODO: Defer assignment to Gfx thread.
   }

   //if( true )
   //{
   //   //StdErr << "Dumping..." << nl;
   //   String fmt = ResManager::getFontName(_font.ptr()) + "_%02d.png";
   //   for( uint i = 0; i < _font->numSlices(); ++i )
   //   {
   //      _font->slice(i)._bitmap->saveFile( String().format(fmt.cstr(), i) );
   //   }
   //}

   return ok;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerLibFreetype::make( char32_t codepoint, Character& character )
{
   // Set texSize.
   character.texSize( getGlyphSize( codepoint ) + _fontData.scalableDistance()*2 );

   // Set (and reserve) slice and texPosition.
   if( !_fontData.createCharacter( character ) )  return false;

   return createGlyph( codepoint, character );
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerLibFreetype::init()
{
   int error;

   // Initialize the font library
   error = FT_Init_FreeType( &_library );
   if( error )
   {
      StdErr << "ERROR - FT_Init_Freetype failed." << nl;
      return false;
   }

   // Retrieve the font's face.
   error = FT_New_Face( _library, _font->path().cstr(), 0, &_face );
   if( error )
   {
      StdErr << "ERROR - FT_New_Face failed." << nl;
      return false;
   }

   DBG_MSG( os_font, "#faces: " << (int)_face->num_faces );
   DBG_MSG( os_font, "#glyphs: " << (int)_face->num_glyphs );
   DBG_MSG( os_font, "#fixed sizes: " << _face->num_fixed_sizes );

   // Define the font size.
   error = FT_Set_Pixel_Sizes( _face, 0, _fontData.fontSize() );
   if( error )
   {
      StdErr << "ERROR - FT_Set_Pixel_Sizes failed." << nl;
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerLibFreetype::term()
{
   int err1 = FT_Done_Face( _face );
   if( err1 )
   {
      StdErr << "ERROR - FT_Done_Face failed." << nl;
   }

   int err2 = FT_Done_FreeType( _library );
   if( err2 )
   {
      StdErr << "ERROR - FT_Done_FreeType failed." << nl;
   }

   return (err1 + err2) == 0;
}

//-----------------------------------------------------------------------------
//!
Vec2i
GlyphMakerLibFreetype::getGlyphSize( char32_t codepoint )
{
   FT_UInt gi = FT_Get_Char_Index( _face, codepoint );
   if( gi == 0 && codepoint != 0 )  return Vec2i( -1 );

   int error = FT_Load_Glyph( _face, gi, FT_LOAD_DEFAULT );
   if( error )  return Vec2i( -1 );

   const FT_Glyph_Metrics& glyphMetrics = _face->glyph->metrics;
   return Vec2i( round_26_6(glyphMetrics.width), round_26_6(glyphMetrics.height) );
}

//------------------------------------------------------------------------------
//!
void
GlyphMakerLibFreetype::drawGlyph( int x, int y, const FT_Bitmap& src, Bitmap& dst )
{
   CHECK( 0 <= x && x < dst.width()  );
   CHECK( 0 <= y && y < dst.height() );
   CHECK( 0 <= (x+src.width) && (x+src.width) <= dst.width()  );
   CHECK( 0 <= (y+src.rows ) && (y+src.rows ) <= dst.height() );
   // Copy every row, bottom up.
   for( int row = 0; row < src.rows; ++row )
   {
      uchar* curDst = dst.pixel( Vec2i( x, y + row ) );
      uchar* curSrc = src.buffer + (src.rows-row-1)*src.pitch;
      for( int col = 0; col < src.width; ++col )
      {
         // Luminance
         *curDst++ = *curSrc;  //Storing pre-multiplied alpha.
#if !FUSION_LA_TEXTURE
         *curDst++ = *curSrc;
         *curDst++ = *curSrc;
#endif
         // Alpha
         *curDst++ = *curSrc++;
      }
   }
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerLibFreetype::createGlyph( char32_t codepoint, Character& character )
{
   FT_UInt gi = FT_Get_Char_Index( _face, codepoint );
   if( gi == 0 && codepoint != 0 )
   {
      // Reuse the same texture data as codepoint == 0.
      character = *_fontData.get( char32_t(0) );
      return true;
   }

   int error = FT_Load_Glyph( _face, gi, FT_LOAD_DEFAULT );
   if( error )  return false;

   // Render the glyph (unless the font is already a bitmap font).
   if( _face->glyph->format != ft_glyph_format_bitmap )
   {
      error = FT_Render_Glyph( _face->glyph, ft_render_mode_normal );
      if( error )  return false;
   }

   CHECK( (character.glyphWidth()  - _fontData.scalableDistance()*2) == _face->glyph->bitmap.width ); // If it fails, we might need to ceil
   CHECK( (character.glyphHeight() - _fontData.scalableDistance()*2) == _face->glyph->bitmap.rows  ); // the width/height rather than round.

   // Copy into the Bitmap.
   FontData::Slice& slice = _fontData.slice(character.slice());
   CHECK( slice._bitmap.isValid() );
   Vec2i pos = character.texPosition();
   pos += _fontData.scalableDistance();  // Leave some space bottom and left.
   drawGlyph(
      pos.x, pos.y,
      _face->glyph->bitmap,
      *(slice._bitmap)
   );

   // Optionally convert to distance field.
   if( _scalable )
   {
#if FUSION_PROFILE_GLYPHS >= 2
      Timer timer;
#endif
      //StdErr << "Converting char=" << codepoint << " '" << toStr(codepoint) << "' pos=" << character.texPosition() << " size=" << character.texSize() << nl;
      if( !toDistanceField( *(slice._bitmap), character.texPosition(), character.texSize(), _fontData.scalableDistance() ) )
      {
         StdErr << "ERROR converting distance field for 0x" << toHex(uint32_t(codepoint)) << "." << nl;
      }
#if FUSION_PROFILE_GLYPHS >= 2
      StdErr << _font->getInfo() << ":" << toStr(codepoint) << ": " << timer.elapsed() << nl;
#endif
   }

   // Set the horizontal advance.
   const FT_Glyph_Metrics& glyphMetrics = _face->glyph->metrics;
   int xMin = round_26_6( glyphMetrics.horiBearingX );
   int yMin = round_26_6( glyphMetrics.horiBearingY - glyphMetrics.height );
   // See commit:ee7e5fbe107069ff694a714540a665a0e12880b2 for the reason.
   int advH = isdigit(codepoint) ? round_16_16( _face->glyph->linearHoriAdvance ) : round_26_6( glyphMetrics.horiAdvance );
   xMin -= _fontData.scalableDistance();
   yMin -= _fontData.scalableDistance();
   character.setH( xMin, yMin, advH );
   //StdErr << "glyph: '" << char(c) << "' pos=" << pos << " size=" << Vec2i(gw, gh)
   //       << " xya=" << xMin << " " << yMin << " " << advH << nl;
   return true;
}

#endif //FUSION_USE_LIB_FREETYPE


#if FUSION_USE_STB_TRUETYPE

/*==============================================================================
  CLASS GlyphMakerSTBTruetype
==============================================================================*/
class GlyphMakerSTBTruetype:
   public GlyphMaker
{
public:

   /*----- methods -----*/

   GlyphMakerSTBTruetype( Font& font, FontData& fontData, bool scalable );
   virtual ~GlyphMakerSTBTruetype();

   virtual String  getFontName();
   virtual   bool  getFontMetrics( float& maxAscender, float& maxDescender, float& lineGap );
   virtual   bool  make( const char32_t* codepoints, size_t n, bool tight = false );
   virtual   bool  make( char32_t codepoint, Character& character );

protected:

   /*----- data members -----*/

   Vector<uchar>   _tmpFont;   //!< The bytes of the font file.
   stbtt_fontinfo  _fi;        //!< STB Trutype's font info structure.
   float           _scale;     //!< A scale factor, used in a few routines.
   Vector<uchar>   _tmpGlyph; //!< A temporary buffer to hold a current glyph.

   /*----- methods -----*/

   bool   init();
   bool   term();
   Vec2i  getGlyphSize( char32_t codepoint );
   void   drawGlyph( const uchar* src, const int w, const int h, Bitmap& dst, int x, int y );
   bool   createGlyph( char32_t codepoint, Character& character );

private:
}; //class GlyphMakerSTBTruetype

//-----------------------------------------------------------------------------
//!
GlyphMakerSTBTruetype::GlyphMakerSTBTruetype( Font& font, FontData& fontData, bool scalable ):
   GlyphMaker( font, fontData, scalable )
{
   if( !init() )
   {
      CHECK( false );
   }
}

//-----------------------------------------------------------------------------
//!
GlyphMakerSTBTruetype::~GlyphMakerSTBTruetype()
{
   if( !term() )
   {
      CHECK( false );
   }
}

//-----------------------------------------------------------------------------
//!
String
GlyphMakerSTBTruetype::getFontName()
{
   int len;
   const char* str = stbtt_GetFontNameString( &_fi, &len, STBTT_PLATFORM_ID_MAC, STBTT_MAC_EID_ROMAN, STBTT_MAC_LANG_ENGLISH, 1 );
   if( str != nullptr )
   {
      return String( str, len );
   }
   else
   {
      return String();
   }
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerSTBTruetype::getFontMetrics( float& maxAscender, float& maxDescender, float& lineGap )
{
   // Retrieve the metrics values.
   int a, d, g;
   stbtt_GetFontVMetrics( &_fi, &a, &d, &g );

   maxAscender  = _scale * float(a);
   maxDescender = _scale * float(d);
   lineGap      = _scale * float(g);
   if( lineGap == 0.0f )  lineGap = maxAscender - maxDescender;

   return true;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerSTBTruetype::make( const char32_t* codepoints, size_t n, bool tight )
{
   bool ok = true;

   int d2 = _fontData.scalableDistance()*2;

   if( tight )
   {
      // Determine required Bitmap size.
      IncrementalRectPacker tmpPacker( _defaultTexSize.x, _defaultTexBorder );
      for( size_t i = 0; i < n; ++i )
      {
         char32_t codepoint = codepoints[i];
         Vec2i s = getGlyphSize( codepoint );
         if( s.x >= 0 )  tmpPacker.add( s + d2 );
      }

      // Allocate memory for destination and temporaries.
      _fontData.createSlice( tmpPacker.dimension() );
   }

   // Create all of the glyphs, and store them in the bitmap.
   for( size_t i = 0; i < n; ++i )
   {
      char32_t codepoint = codepoints[i];
      Vec2i s = getGlyphSize( codepoint );
      Character character;
      character.texSize( s + d2 );
      if( s.x >= 0 )
      {
         if( !_fontData.createCharacter( character ) )
         {
            ok = false;
            _fontData.set( codepoint, *_fontData.get(0) ); // TODO: Defer assignment to Gfx thread.
            continue;
         }
      }
      if( !createGlyph( codepoint, character ) )
      {
         ok = false;
         _fontData.set( codepoint, *_fontData.get(0) ); // TODO: Defer assignment to Gfx thread.
         continue;
      }
      _fontData.set( codepoint, character ); // TODO: Defer assignment to Gfx thread.
   }

#if 0
   if( true && _scalable )
   {
      //StdErr << "Dumping..." << nl;
      String fmt = ResManager::getFontName(_font.ptr()) + "_%02d.png";
      for( uint i = 0; i < _fontData.numSlices(); ++i )
      {
         _fontData.slice(i)._bitmap->saveFile( String().format(fmt.cstr(), i) );
      }
   }
#endif

   return ok;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerSTBTruetype::make( char32_t codepoint, Character& character )
{
   // Set texSize.
   character.texSize( getGlyphSize(codepoint) + _fontData.scalableDistance()*2 );

   // Set (and reserve) slice and texPosition.
   if( !_fontData.createCharacter( character ) )  return false;

   return createGlyph( codepoint, character );
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerSTBTruetype::init()
{
   FS::Entry  entry( _font->path() );
   if( !entry.exists() )
   {
      DBG_MSG( os_font, "Font file '" << _font->path().string() << "' not found." );
      return false;
   }

   DBG_MSG( os_font, "Allocating " << entry.size() << " bytes to store the font." );
   _tmpFont.grow( entry.size() );

   if( _tmpFont.size() != entry.size() )
   {
      DBG_MSG( os_font, "Could not allocated " << entry.size() << " bytes for font." );
      return false;
   }

   DBG_MSG( os_font, "Reading the font file." );
   FILE* fd = fopen( entry.path().cstr(), "rb" );
   fread( _tmpFont.data(), 1, entry.size(), fd );
   fclose( fd );

   stbtt_InitFont( &_fi, _tmpFont.data(), 0 );

   DBG_MSG( os_font, "#glyphs: " << _fi.numGlyphs );
#if 1
   int maxAscender, maxDescender, lineGap;
   stbtt_GetFontVMetrics( &_fi, &maxAscender, &maxDescender, &lineGap );
   float ad_a = float( maxAscender - maxDescender ) / float( maxAscender );
   float size = _fontData.fontSize() * ad_a;
   //size = _font->fontSize();
#else
   float size = _fontData.fontSize();
#endif
   _scale = stbtt_ScaleForPixelHeight( &_fi, size );
   DBG_MSG( os_font, "scale: " << _scale << " (fontSize=" << _fontData.fontSize() << " >> " << size << ")" );

   return true;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerSTBTruetype::term()
{
   return true;
}

//-----------------------------------------------------------------------------
//!
Vec2i
GlyphMakerSTBTruetype::getGlyphSize( char32_t codepoint )
{
   int gi = stbtt_FindGlyphIndex( &_fi, codepoint );
   if( gi == 0 && codepoint != 0 )  return Vec2i( -1 );

   int x0, x1, y0, y1;
   stbtt_GetGlyphBitmapBox( &_fi, gi, _scale, _scale, &x0, &y0, &x1, &y1 );

   return Vec2i( x1-x0, y1-y0 );
}

/*
// Converts a grayscale value into an ascii character for display (ASCII-art style).
inline char g2a( uchar v )
{
   //return " .:ioVM@"[v>>5]; // Sean Barrett's palette.
   return " .,;icosgCUOAWRB"[v>>4];
}
*/

//------------------------------------------------------------------------------
//!
void
GlyphMakerSTBTruetype::drawGlyph( const uchar* src, const int w, const int h, Bitmap& dst, int x, int y )
{
   //putchar('\n');
   CHECK( 0 <= x && x < dst.width()  );
   CHECK( 0 <= y && y < dst.height() );
   CHECK( 0 <= (x+w) && (x+w) <= dst.width()  );
   CHECK( 0 <= (y+h) && (y+h) <= dst.height() );
   for( int j = 0; j < h; ++j )
   {
      const uchar* srcTexel = src + w*(h-j-1);
      uchar* dstTexel = dst.pixel( Vec2i(x, y+j) );
      for( int i = 0; i < w; ++i )
      {
         //putchar( g2a(*srcTexel) ); // Also uncomment routine above.
         *dstTexel++ = *srcTexel; // Storing pre-multiplied alpha.
#if !FUSION_LA_TEXTURE
         *dstTexel++ = *srcTexel;
         *dstTexel++ = *srcTexel;
#endif
         *dstTexel++ = *srcTexel++;
      }
      //putchar('\n');
   }
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMakerSTBTruetype::createGlyph( char32_t codepoint, Character& character )
{
   int gi = stbtt_FindGlyphIndex( &_fi, codepoint );
   if( gi == 0 && codepoint != 0 )
   {
      // Reuse the same texture data as codepoint == 0.
      character = *_fontData.get( char32_t(0) );
      return true;
   }

   // Generate the glyph.
   int x0, x1, y0, y1;
   stbtt_GetGlyphBitmapBox( &_fi, gi, _scale, _scale, &x0, &y0, &x1, &y1 );
   int gw = x1 - x0;
   int gh = y1 - y0;
   _tmpGlyph.grow( gw*gh );
   stbtt_MakeGlyphBitmap( &_fi, _tmpGlyph.data(), gw, gh, gw, _scale, _scale, gi );

   // Copy into the bitmap.
   FontData::Slice& slice = _fontData.slice(character.slice());
   CHECK( slice._bitmap.isValid() );
   Vec2i pos = character.texPosition();
   pos += _fontData.scalableDistance();  // Leave some space bottom and left.
   drawGlyph( _tmpGlyph.data(), gw, gh, *(slice._bitmap), pos.x, pos.y );

   // Optionally convert to distance field.
   if( _scalable )
   {
#if FUSION_PROFILE_GLYPHS >= 2
      Timer timer;
#endif
      if( !toDistanceField( *(slice._bitmap), character.texPosition(), character.texSize(), _fontData.scalableDistance() ) )
      {
         StdErr << "ERROR converting distance field for 0x" << toHex(uint32_t(codepoint)) << "." << nl;
      }
#if FUSION_PROFILE_GLYPHS >= 2
      StdErr << _font->getInfo() << ":" << toStr(codepoint) << ": " << timer.elapsed() << nl;
#endif
   }

   // Set horizontal metrics.
   int advanceWidth, leftSideBearing;
   stbtt_GetGlyphHMetrics( &_fi, gi, &advanceWidth, &leftSideBearing );
   int advance = int(advanceWidth*_scale + 0.5f);
   x0 -= _fontData.scalableDistance();
   y1 += _fontData.scalableDistance();
   character.setH( x0, -y1, advance ); // -y1 because stbtt has origin top-left.

   return true;
}

#endif //FUSION_USE_STB_TRUETYPE


//-----------------------------------------------------------------------------
//! Returns a vector of default glyphs to make (slightly extended ASCII subset).
//! Returning a Vector<char32_t> forces a full copy, but this way, the final
//! _defaultGlyphs should be tight.
Vector<char32_t>  defaultGlyphs()
{
   const char32_t  _sCharRanges[] = {
       0,   0, // Make it the first character.
      32, 126,
   };

   const char* _sCharSets[] = {
      "àâäéèêëíìîïóòôöõøúùûüŭñç", // Common lowercase accentuated characters.
      "ÀÂÄÉÈÊËÍÌÎÏÓÒÔÖÕØÚÙÛÜŬÑÇ", // Common uppercase accentuated characters.
      "©¶•°“”",                   // Some special punctuation.
      "¢£¥₨€¤",                   // Currency signs.
   };

   Vector<char32_t>  dst;
   for( size_t i = 0; i < sizeof(_sCharRanges)/sizeof(_sCharRanges[0]); i += 2 )
   {
      char32_t firstChar = _sCharRanges[i];
      char32_t lastChar  = _sCharRanges[i+1];
      for( char32_t codepoint = firstChar; codepoint <= lastChar; ++codepoint )
      {
         dst.pushBack( codepoint );
      }
   }
   for( size_t i = 0; i < sizeof(_sCharSets)/sizeof(_sCharSets[0]); ++i )
   {
      for( UTF8Iterator it = _sCharSets[i]; it(); ++it )
      {
         dst.pushBack( *it );
      }
   }

   return dst;
}

UNNAMESPACE_END


/*==============================================================================
  CLASS GlyphMaker
==============================================================================*/

//-----------------------------------------------------------------------------
//!
GlyphMaker::GlyphMaker( Font& font, FontData& fontData, bool scalable ):
   _font( &font ),
   _fontData( fontData ),
   _scalable( scalable ),
   _scalableFactor( 1.0f / fontData.scalableDistance() )
   //_texSize( texSize )
{
   //if( _texSize.x*_texSize.y == 0 )  _texSize = _defaultTexSize;
}

//-----------------------------------------------------------------------------
//!
GlyphMaker::~GlyphMaker()
{
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMaker::makeDefaultGlyphs()
{
#if FUSION_PROFILE_GLYPHS >= 1
   Timer timer;
   bool ok = make( _defaultGlyphs.data(), _defaultGlyphs.size(), true );
   StdErr << "Default glyphs for " << _font->getInfo() << " took " << timer.elapsed() << nl;
   return ok;
#else
   // Populate with default glyphs.
   return make( _defaultGlyphs.data(), _defaultGlyphs.size(), true );
#endif
}

//-----------------------------------------------------------------------------
//!
bool
GlyphMaker::toDistanceField( Bitmap& bmp, const Vec2i& pos, const Vec2i& dim, int maxDist )
{
   CHECK( bmp.pixelType() == Bitmap::BYTE );
#if 0
   RCP<Bitmap> dist  = new Bitmap( dim, Bitmap::FLOAT, 1 );
   float* distP      = (float*)dist->pixels();
   for( int y = 0; y < dim.y; ++y )
   {
      uchar* bmpP = bmp.pixel( Vec2i( pos.x, pos.y+y ) );
      for( int x = 0; x < dim.x; ++x, bmpP += bmp.pixelSize(), ++distP )
      {
         *distP = float(bmpP[0])*(1.0f/255.0f);
      }
   }
   //dist->print();
   RCP<Bitmap>  fg = new Bitmap( dim, Bitmap::FLOAT, 1 );
   RCP<Bitmap>  bg = new Bitmap( dim, Bitmap::FLOAT, 1 );
   RCP<Bitmap> dst = new Bitmap( dim, Bitmap::FLOAT, 1 );
   BitmapManipulator::toDistanceField( *dist, *fg, *bg, *dst );
   //fg->print();
   //bg->print();
   //dst->print();
   distP = (float*)dst->pixels();
#else
   RCP<Bitmap> dist = BitmapManipulator::grayscaleToDistanceField( bmp, pos, dim, maxDist );
   float* distP = (float*)dist->pixels();
#endif
   for( int y = 0; y < dim.y; ++y )
   {
      uchar* bmpP = bmp.pixel( Vec2i( pos.x, pos.y+y ) );
      for( int x = 0; x < dim.x; ++x, bmpP += bmp.pixelSize(), ++distP )
      {
         float v = distP[0] * _scalableFactor;
         v = CGM::clamp( v, -1.0f, 1.0f );  // Range [-1.0f,   1.0f].
         v = v*-0.5f + 0.5f;                // Range [ 0.0f,   1.0f] (with a flip for >0.5 is inside).
         v *= 255.0f;                       // Range [ 0.0f, 255.0f].
         v += 0.5f;                         // Rounding.
#if FUSION_LA_TEXTURE
         bmpP[0] = 255;
         bmpP[1] = uchar( v );
#else
         bmpP[0] = bmpP[1] = bmpP[2] = 255;
         bmpP[3] = uchar( v );
#endif
      }
   }
   return true;
}


/*==============================================================================
  CLASS GlyphManager
==============================================================================*/

//-----------------------------------------------------------------------------
//! Generates the glyph for the specified codepoint in the background.
//! Automatically updates Text drawables using that Font.
void
GlyphManager::createGlyph( Font& font, int fontSize, char32_t codepoint )
{
   LockGuard guard( _lock );
   CHECK( codepoint < 0x1FFFFF );
   _fonts[FontPair(&font,fontSize)].add( codepoint ); // If it becomes a bottleneck, could use atomic lists.
}

//-----------------------------------------------------------------------------
//! Launches a background task to generate missing glyphs (if any).
//! If no glyphs are to be created, or if a task already exists, nothing happens.
void
GlyphManager::generateMissingGlyphs()
{
   if( !_fonts.empty() )
   {
      if( _taskRunning.CAS(0, 1) )
      {
         //StdErr << "Creating task." << nl;
         ResManager::dispatchQueue()->post( new GlyphMakingTask() );
      }
      else
      {
         //StdErr << "Preventing needless task to be created." << nl;
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::updateGfx( Font& font, int fontSize )
{
   uint n = font.data(fontSize)->numSlices();
   for( uint i = 0; i < n; ++i )
   {
      updateGfx( font, fontSize, i );
   }
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::updateGfx( Font& font, int fontSize, uint sliceID )
{
   FontData* fontData = font.data( fontSize );
   CHECK( sliceID < fontData->numSlices() );

   FontData::Slice& slice = fontData->slice( sliceID ); // Using the non-const retrieval method.
   if( slice._bitmap.isNull() )  return;

   if( slice._texture.isNull() )
   {
      // Create the texture.
      Vec2i dimNP2 = CGM::nextPow2( slice._bitmap->dimension()(0,1) );
#if FUSION_LA_TEXTURE
      slice._texture = Core::gfx()->create2DTexture(
         dimNP2.x, dimNP2.y, Gfx::TEX_FMT_8_8, Gfx::TEX_CHANS_LA, Gfx::TEX_FLAGS_NONE
      );
#else
      slice._texture = Core::gfx()->create2DTexture(
         dimNP2.x, dimNP2.y, Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_NONE
      );
#endif

      // Create the sampler.
      Gfx::TextureState texState;
      texState.magFilter( Gfx::TEX_FILTER_LINEAR );
      texState.minFilter( Gfx::TEX_FILTER_LINEAR );
      texState.mipFilter( Gfx::TEX_FILTER_NONE );
      texState.clampX( Gfx::TEX_CLAMP_LAST );
      texState.clampY( Gfx::TEX_CLAMP_LAST );
      slice._sampler = new Gfx::SamplerList();
      slice._sampler->addSampler( "colorTex", slice._texture, texState );
   }

   // Fill in the data.
   Core::gfx()->setData( slice._texture, 0, 0, 0, slice._bitmap->width(), slice._bitmap->height(), slice._bitmap->pixels() );
   //Core::gfx()->generateMipmaps( slice._texture );
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::registerTextDrawable( Text* text )
{
   _textDrawables.pushBack( text );
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::unregisterTextDrawable( Text* text )
{
   _textDrawables.removeSwap( text );
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::updateTextDrawables( Font* font )
{
   for( Vector<Text*>::Iterator curT = _textDrawables.begin(),
                                endT = _textDrawables.end();
        curT != endT;
        ++curT )
   {
      Text& text = *(*curT);
      // TODO: better filter.
      if( text.font() == font )
      {
         text.update();
      }
   }
}

//-----------------------------------------------------------------------------
//!
RCP<GlyphMaker>
GlyphManager::createGlyphMaker( Font& font, FontData& fontData, bool scalable, GlyphMakerType type )
{
   // Collapse DEFAULT to the actual value.
   type = (type == DEFAULT) ? _defaultGlyphMakerType : type;

   switch( type )
   {
      case DEFAULT:
         // The first line above should have collapsed it correctly.
         return NULL;

      case LIB_FREETYPE:
#if FUSION_USE_LIB_FREETYPE
         return new GlyphMakerLibFreetype( font, fontData, scalable );
#else
         return NULL;
#endif

      case STB_TRUETYPE:
#if FUSION_USE_STB_TRUETYPE
         return new GlyphMakerSTBTruetype( font, fontData, scalable );
#else
         return NULL;
#endif

      default:
         return NULL;
   }
}

//-----------------------------------------------------------------------------
//!
RCP<Bitmap>
GlyphManager::createFontBitmap( const Vec2i& s )
{
   Vec2i d = s;
   if( (d.x * d.y) == 0 )  d = _defaultTexSize;

   Bitmap* bitmap;
#if FUSION_LA_TEXTURE
   bitmap = new Bitmap( d, Bitmap::BYTE, 2 );
#else
   bitmap = new Bitmap( d, Bitmap::BYTE, 4 );
#endif
   memset( bitmap->pixels(), 0, bitmap->size() );

   return bitmap;
}

//-----------------------------------------------------------------------------
//!
GlyphManager::GlyphMakerType
GlyphManager::defaultGlyphMakerType()
{
   return _defaultGlyphMakerType;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::defaultGlyphMakerType( const GlyphManager::GlyphMakerType type )
{
   _defaultGlyphMakerType = type;
}

//-----------------------------------------------------------------------------
//!
const Vec2i&
GlyphManager::defaultTexSize()
{
   return _defaultTexSize;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::defaultTexSize( const Vec2i& s )
{
   _defaultTexSize = s;
}

//-----------------------------------------------------------------------------
//!
int
GlyphManager::defaultTexBorder()
{
   return _defaultTexBorder;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::defaultTexBorder( int b )
{
   _defaultTexBorder = b;
}

//-----------------------------------------------------------------------------
//!
bool
GlyphManager::defaultScalable()
{
   return _defaultScalable;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::defaultScalable( bool v )
{
   _defaultScalable = v;
}

//-----------------------------------------------------------------------------
//!
int
GlyphManager::defaultScalableFontSize()
{
   return _defaultScalableFontSize;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::defaultScalableFontSize( int s )
{
   _defaultScalableFontSize = s;
}

//-----------------------------------------------------------------------------
//!
int
GlyphManager::defaultScalableMaxDistance()
{
   return _defaultScalableMaxDistance;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::defaultScalableMaxDistance( int d )
{
   _defaultScalableMaxDistance = d;
}

//-----------------------------------------------------------------------------
//!
void
GlyphManager::printInfo( TextStream& os )
{
#if FUSION_USE_LIB_FREETYPE
   os << " Freetype";
   FT_Library ftlib;
   if( !FT_Init_FreeType( &ftlib ) )
   {
      int major, minor, patch;
      FT_Library_Version(ftlib, &major, &minor, &patch);
      os << '(' << major << '.' << minor << '.' << patch << ')';
      FT_Done_FreeType(ftlib);
   }
   else
   {
      os << "(\?\?\?)";
   }
   if( defaultGlyphMakerType() == LIB_FREETYPE )  os << "*";
   os << nl;
#endif

#if FUSION_USE_STB_TRUETYPE
   os << " STB_Truetype(>=0.3)";
   if( defaultGlyphMakerType() == STB_TRUETYPE )  os << "*";
   os << nl;
#endif
}
