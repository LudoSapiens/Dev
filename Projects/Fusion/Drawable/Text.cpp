/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Drawable/Text.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Resource/Font.h>
#include <Fusion/Resource/GlyphManager.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/Core/Core.h>

#include <CGMath/CGMath.h>
#include <CGMath/Range.h>

#include <Base/ADT/StringMap.h>
#include <Base/Util/UnicodeIterator.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
bool contains( const Vec2f& pos0, const Vec2f& size0, const Vec2f& pos1, const Vec2f& size1 )
{
   if( pos1.x >= pos0.x && pos1.y >= pos0.y &&
       pos1.x + size1.x <= pos0.x + size0.x &&
       pos1.y + size1.y <= pos0.y + size0.y )
   {
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
int
charPositionVM( VMState* vm )
{
   Text* text = (Text*)VM::thisPtr( vm );
   uint pos   = VM::toUInt( vm, 1 );

   VM::push( vm, text->charPosition( pos ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
charIndexVM( VMState* vm )
{
   Text* text = (Text*)VM::thisPtr( vm );
   Vec2f pos  = VM::toVec2f( vm, 1 );

   VM::push( vm, text->charIndex( pos ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_TEXT,
   ATTRIB_COLOR,
   ATTRIB_FONT,
   ATTRIB_ORIENTATION,
   ATTRIB_ALIGNH,
   ATTRIB_ALIGNV,
   ATTRIB_POSITION,
   ATTRIB_SIZE,
   ATTRIB_TEXT_SIZE,
   ATTRIB_OFFSET,
   ATTRIB_CHAR_POSITION,
   ATTRIB_CHAR_INDEX
};

StringMap _attributes(
   "text",         ATTRIB_TEXT,
   "color",        ATTRIB_COLOR,
   "font",         ATTRIB_FONT,
   "orientation",  ATTRIB_ORIENTATION,
   "alignH",       ATTRIB_ALIGNH,
   "alignV",       ATTRIB_ALIGNV,
   "position",     ATTRIB_POSITION,
   "size",         ATTRIB_SIZE,
   "textSize",     ATTRIB_TEXT_SIZE,
   "offset",       ATTRIB_OFFSET,
   "charPosition", ATTRIB_CHAR_POSITION,
   "charIndex",    ATTRIB_CHAR_INDEX,
   ""
);

const VM::EnumReg _enumsTextAlignment[] = {
   { "START",      Text::START     },
   { "MIDDLE",     Text::MIDDLE    },
   { "END",        Text::END       },
   { "BASELINE",   Text::BASELINE  },
   { "MIDASCENT",  Text::MIDASCENT },
   { 0, 0 }
};

const VM::EnumReg _enumsTextOrientation[] = {
   { "HORIZONTAL",  Text::HORIZONTAL },
   { "VERTICAL",    Text::VERTICAL   },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
const char* _text_str_ = "text";

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.TextAlignment", _enumsTextAlignment );
   VM::registerEnum( vm, "UI.TextOrientation", _enumsTextOrientation );
}

//------------------------------------------------------------------------------
//!
int
text_get( VMState* vm )
{
   Text* d = (Text*)VM::toProxy( vm, 1 );
   return d->performGet( vm ) ? 1 : 0;
}

//------------------------------------------------------------------------------
//!
int
text_set( VMState* vm )
{
   Text* d = (Text*)VM::toProxy( vm, 1 );
   d->performSet( vm );
   return 0;
}

RCP<Gfx::Program>  _scalableProg;

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Text
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Text::initialize()
{
   VMRegistry::add( _text_str_, NULL, text_get, text_set, VM_CAT_APP );
   VMRegistry::add( initVM, VM_CAT_APP );
}

//-----------------------------------------------------------------------------
//!
void
Text::terminate()
{
   _scalableProg = nullptr;
}

//------------------------------------------------------------------------------
//!
Text::Text()
   : Drawable(),
     _position( 0.0f, 0.0f ),
     _size( 0.0f, 0.0f ),
     _textSize( 0.0f, 0.0f ),
     _offset( 0.0f, 0.0f ),
     _fontSize( -1 ),
     _fontScalable( false ),
     _alignH( MIDDLE ),
     _alignV( MIDDLE ),
     _orient( HORIZONTAL ),
     _color( 1.0f, 1.0f, 1.0f, 1.0f ),
     _scissored( false )
{
   GlyphManager::registerTextDrawable( this );
   _geom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   _constants = Core::gfx()->createConstants( 16 );
   _constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
   _constants->setConstant( "color", _color.ptr() );
   _cl = Gfx::ConstantList::create( _constants );
   if( _scalableProg.isNull() )
   {
      // Lazy initialization.
      _scalableProg = data( ResManager::getProgram( "shader/program/distanceField" ) );
   }
}

//------------------------------------------------------------------------------
//!
Text::~Text()
{
   GlyphManager::unregisterTextDrawable( this );
}

//------------------------------------------------------------------------------
//!
void
Text::text( const String& val )
{
   if( _text != val )
   {
      _text = val;
      update();
   }
}

//------------------------------------------------------------------------------
//!
const String&
Text::text() const
{
   return _text;
}

//------------------------------------------------------------------------------
//!
void
Text::color( const Vec4f& val )
{
   _color = val;
   _constants->setConstant( "color", _color.ptr() );
}

//------------------------------------------------------------------------------
//!
const Vec4f&
Text::color() const
{
   return _color;
}

//------------------------------------------------------------------------------
//!
void
Text::position( const Vec2f& pos )
{
   _position = pos;
   _mat = Mat4f::translation( _position.x, _position.y, 0.0f );
}

//------------------------------------------------------------------------------
//!
const Vec2f&
Text::position() const
{
   return _position;
}

//------------------------------------------------------------------------------
//!
void
Text::size( const Vec2f& val )
{
   if( val != _size )
   {
      _size = val;
      CGM::clampMin( _size.x, 0.0f );
      CGM::clampMin( _size.y, 0.0f );
      update();
   }
}

//------------------------------------------------------------------------------
//!
const Vec2f&
Text::size() const
{
   return _size;
}

//------------------------------------------------------------------------------
//!
void
Text::offset( const Vec2f& val )
{
   if( _offset != val )
   {
      _offset = val;
      update();
   }
}

//------------------------------------------------------------------------------
//!
const Vec2f&
Text::offset() const
{
   return _offset;
}

//------------------------------------------------------------------------------
//!
void
Text::alignment( Alignment alignH, Alignment alignV )
{
   if( alignH != _alignH || alignV != _alignV )
   {
      _alignH = alignH;
      _alignV = alignV;
      update();
   }
}

//------------------------------------------------------------------------------
//!
int
Text::alignmentH() const
{
   return _alignH;
}

//------------------------------------------------------------------------------
//!
int
Text::alignmentV() const
{
   return _alignV;
}

//------------------------------------------------------------------------------
//!
void
Text::orientation( Orientation val )
{
   if( val != _orient )
   {
      _orient = val;
      update();
   }
}

//------------------------------------------------------------------------------
//!
int
Text::orientation() const
{
   return _orient;
}

//------------------------------------------------------------------------------
//!
void
Text::font( const RCP<Font>& font, int fontSize, bool scalable )
{
   bool different = (font != _font);
   different |= (fontSize != _fontSize);
   different |= (scalable != _fontScalable);
   if( different )
   {
      _font         = font;
      _fontSize     = fontSize;
      _fontScalable = scalable;
      if( _fontScalable )
      {
         _font->initScalable();
      }
      else
      {
         _font->initFixed( fontSize );
      }
      update();
   }
}

//------------------------------------------------------------------------------
//!
Vec2f
Text::charPosition( uint pos ) const
{
   Vec2f cpos( _position + _offset + displacement( (Alignment)_alignH, (Alignment)_alignV ) );
   CGM::clampMax( pos, (uint)_text.length() );

   FontCharInfo fci( *_font, _fontSize, _fontScalable );
   UTF8Iterator it = _text.cstr();
   float coff = 0.0f;
   for( uint i = 0; i < pos; ++i, ++it )
   {
      const Character& chr = fci.get( *it );
      coff += chr.glyphAdvanceH();
   }
   cpos.x += coff*fci.factor();
   return cpos;
}

//------------------------------------------------------------------------------
//!
uint
Text::charIndex( const Vec2f& pos ) const
{
   Vec2f cpos( _position + _offset + displacement( (Alignment)_alignH, (Alignment)_alignV ) );

   FontCharInfo fci( *_font, _fontSize, _fontScalable );
   uint i = 0;
   for( UTF8Iterator cur = _text.cstr(),
                     end = _text.cstr() + _text.size();
        cur < end;
        ++cur, ++i )
   {
      const Character& chr = fci.get( *cur );
      cpos.x += chr.glyphAdvanceH()*fci.factor();
      if( pos.x <= cpos.x )
      {
         // Adding the new character extends past the desired position.
         float diff = cpos.x - pos.x;
         if( diff <= (chr.glyphAdvanceH()*fci.factor()*0.5f) )
         {
            // Passed mid-letter.
            return i + 1;
         }
         else
         {
            // Not passed mid-letter.
            return i;
         }
      }
   }
   return i;
}

//------------------------------------------------------------------------------
//!
void
Text::draw( const RCP<Gfx::RenderNode>& rn ) const
{
   if( _text.empty() ) return;

   Gfx::Pass& pass = *(rn->current());

   const int* sc = NULL;
   if( _scissored )
   {
      sc = pass.addScissor( (int)_position.x, (int)_position.y, (int)_size.x, (int)_size.y );
   }

   pass.setWorldMatrixPtr( _mat.ptr() );
   pass.setConstants( _cl );
   uint range[2] = { 0, 0 };
   const FontData* fd;
   if( _fontScalable )
   {
      fd = _font->data( 0 );
      pass.setProgram( _scalableProg );
   }
   else
   {
      fd = _font->data( _fontSize );
      //pass.setProgram( Core::defaultProgram() );
   }
   for( uint i = 0; i < FontData::MAX_SLICES; ++i )
   {
      if( _sliceSize[i] != 0 )
      {
         range[0] += range[1];
         range[1] = _sliceSize[i];
         pass.setSamplers( fd->samplers(i) );
         pass.execRangeGeometry( _geom, range );
      }
   }

#if 0
   // Show descent, baseline, and ascent.
   Vec2f dis = displacement( (Alignment)_alignH, (Alignment)_alignV );
   FontCharInfo fci( *_font, _fontSize, _fontScalable );
   float y;

   RCP<Gfx::VertexBuffer> vertexBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F        , 0 );
   vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR   , Gfx::ATTRIB_FMT_32F_32F_32F_32F, 8 );

#define DRAW_LINE( X1, Y1, X2, Y2, R, G, B, A ) \
   vertices.pushBack(    X1   ); \
   vertices.pushBack(    Y1   ); \
   vertices.pushBack(    R    ); \
   vertices.pushBack(    G    ); \
   vertices.pushBack(    B    ); \
   vertices.pushBack(    A    ); \
   vertices.pushBack(    X2   ); \
   vertices.pushBack(    Y2   ); \
   vertices.pushBack(    R    ); \
   vertices.pushBack(    G    ); \
   vertices.pushBack(    B    ); \
   vertices.pushBack(    A    )

#define DRAW_HLINE( Y, R, G, B, A ) \
   vertices.pushBack(   0.0f  ); \
   vertices.pushBack(    Y    ); \
   vertices.pushBack(    R    ); \
   vertices.pushBack(    G    ); \
   vertices.pushBack(    B    ); \
   vertices.pushBack(    A    ); \
   vertices.pushBack( _size.x ); \
   vertices.pushBack(    Y    ); \
   vertices.pushBack(    R    ); \
   vertices.pushBack(    G    ); \
   vertices.pushBack(    B    ); \
   vertices.pushBack(    A    )

   Vector<float> vertices;  // X Y R G B A   X Y R G B A ...
   //DRAW_LINE( 0.0f, 0.0f, _size.x, _size.y, 1.0f, 1.0f, 0.0f, 0.7f );
   y = _offset.y + dis.y; // + fci.maxDescender();
   DRAW_HLINE( y, 1.0f, 0.0f, 1.0f, 0.5f );
   y -= fci.maxDescender();
   DRAW_HLINE( y, 1.0f, 0.0f, 1.0f, 0.5f );
   y += fci.maxAscender();
   DRAW_HLINE( y, 1.0f, 0.0f, 1.0f, 0.5f );
   y -= fci.maxAscender()*0.5;
   DRAW_HLINE( y, 0.4f, 0.4f, 0.4f, 0.2f );
   y += fci.maxDescender()*0.5;
   DRAW_HLINE( y, 0.4f, 0.4f, 0.4f, 0.2f );
   //y = _offset.y + dis.y + fci.maxHeight();
   //DRAW_HLINE( y, 1.0f, 1.0f, 1.0f, 0.2f ); // Overlaps maxAscender.


   Core::gfx()->setData( vertexBuffer, vertices.dataSize(), vertices.data() );

   RCP<Gfx::Geometry> geom = Core::gfx()->createGeometry( Gfx::PRIM_LINES );
   geom->addBuffer( vertexBuffer );

   RCP<Gfx::Program> prog = data( ResManager::getProgram( "shader/program/vertexColor" ) );
   pass.setProgram( prog );

   pass.execGeometry( geom );
   pass.setProgram( Core::defaultProgram() );

#undef DRAW_HLINE
#undef DRAW_LINE

#endif

   // Restore program.
   if( _fontScalable )  pass.setProgram( Core::defaultProgram() );

   if( _scissored )
   {
      pass.setScissor( sc[0], sc[1], sc[2], sc[3] );
   }
}


//------------------------------------------------------------------------------
//!
Vec2f
Text::displacement( Alignment alignH, Alignment alignV ) const
{
   Vec2f dis;

   switch( alignH )
   {
      case START:     dis.x = 0.0f;                           break;
      case MIDDLE:    dis.x = (_size.x - _textSize.x) * 0.5f; break;
      case END:       dis.x = _size.x - _textSize.x - 1.0f;   break;
      case BASELINE:  dis.x = 0.0f;                           break;
      case MIDASCENT: dis.x = 0.0f;                           break;
   }

   if( _orient == HORIZONTAL )
   {
      FontCharInfo fci( *_font, _fontSize, _fontScalable );
      float a = fci.maxAscender();
      float d = fci.maxDescender();
      switch( alignV )
      {
         case START:     dis.y = 0.0f;                           break;
         case MIDDLE:    dis.y = (_size.y - _textSize.y) * 0.5f; break;
         case END:       dis.y = _size.y - _textSize.y;          break;
         case BASELINE:  dis.y = d;                              break;
         case MIDASCENT: dis.y = (_size.y - a) * 0.5f + d;       break;
      }
   }
   else
   {
      switch( alignV )
      {
         case START:     dis.y = _textSize.y;                    break;
         case MIDDLE:    dis.y = (_size.y + _textSize.y) * 0.5f; break;
         case END:       dis.y = _size.y;                        break;
         case BASELINE:  dis.y = (_size.y + _textSize.y) * 0.5f; break;
         case MIDASCENT: dis.y = (_size.y + _textSize.y) * 0.5f; break;
      }
   }

   return dis;
}

//------------------------------------------------------------------------------
//!
void
Text::update()
{
   if( _text.empty() )
   {
      FontCharInfo fci( *_font, _fontSize, _fontScalable );
      _textSize = Vec2f( 0.0f, fci.maxHeight() );
      return;
   }

   size_t nChars = getUTF8Length( _text.cstr(), _text.size() );

   Vector<Vec4f>    vertices( 4 * nChars ); // 4 vertices per quad.
   Vector<uint16_t> indices( 6 * nChars );  // 2 triangles per quad.

   _scissored = false;

   const FontData* fontData = _font->data(_fontScalable ? 0 : _fontSize);

   // Compute text size.
   FontCharInfo fci( *_font, _fontSize, _fontScalable );
   _textSize = Vec2f( fci.getWidth(_text.cstr()), fci.maxHeight() );

   // create string
   Vec2f startPos( _offset + displacement( (Alignment)_alignH, (Alignment)_alignV ) );
   startPos.y -= fci.maxDescender(); // Place writing position on the baseline.

   Rangef yRange = Rangef::empty(); // To track for scissoring.
   float oto     = Core::gfx()->oneToOneOffset();

   Vec4f*      curV = vertices.data();
   uint16_t*   curI = indices.data();
   int curSlice = 0;
   int maxSlice = 0;
   while( curSlice <= maxSlice )
   {
      const Vec4f* startV = curV;
      const Gfx::Texture& tex = *(fontData->texture(curSlice));
      Vec2i dim( tex.width(), tex.height() );
      Vec2f oodim = Vec2f(1.0f)/dim;
      Vec2f pos = startPos;
      UTF8Iterator it = _text.cstr();
      for( size_t i = 0; i < nChars; ++i, ++it )
      {
         const Character* chr = fontData->get( *it );
         if( chr == NULL )  continue;

         if( chr->slice() == curSlice )
         {
            // Part of the current slice.
            Vec2f bl_pos, tr_pos;
            Vec2f bl_tex, tr_tex;

            Vec2f texSize = chr->texSize();
            Vec2f texPos  = chr->texPosition();
            if( _orient == HORIZONTAL )
            {
               Vec2f  offset = Vec2f(chr->offsetH()) * fci.factor();
               float advance = chr->glyphAdvanceH()  * fci.factor();
               // Snap character to pixel for now (non-scalable fonts only).
               bl_pos = _fontScalable  ? pos : CGM::round(pos + Vec2f(0.5f));
               bl_pos += offset + oto;
               tr_pos = bl_pos + texSize * fci.factor();
               bl_tex = texPos*oodim;
               tr_tex = (texPos+texSize)*oodim;
               pos.x += advance;
            }
            else
            {
               CHECK(false);  // Unsupported for now.
            }

            yRange |= bl_pos.y;
            yRange |= tr_pos.y;

            uint16_t curIdx = uint16_t(curV - vertices.data()); // 4 vertices per quad.

            curV[0] = Vec4f( bl_pos.x, bl_pos.y, bl_tex.x, bl_tex.y );
            curV[1] = Vec4f( tr_pos.x, bl_pos.y, tr_tex.x, bl_tex.y );
            curV[2] = Vec4f( tr_pos.x, tr_pos.y, tr_tex.x, tr_tex.y );
            curV[3] = Vec4f( bl_pos.x, tr_pos.y, bl_tex.x, tr_tex.y );

            curV += 4;

            curI[0] = curIdx+0;
            curI[1] = curIdx+1;
            curI[2] = curIdx+2;
            curI[3] = curIdx+0;
            curI[4] = curIdx+2;
            curI[5] = curIdx+3;
            curI += 6;
         }
         else
         {
            maxSlice = CGM::max( chr->slice(), maxSlice ); // Cheaper for now than a separate pass to determine.
            // Advance the position.
            if( _orient == HORIZONTAL )
            {
               int  advance = chr->glyphAdvanceH();
               pos += Vec2f( float(advance), 0.0f );
            }
            else
            {
               CHECK(false);  // Unsupported for now.
            }
         }
      }
      _sliceSize[curSlice] = (uint(curV - startV)>>2)*6; // Convert 4 vertices to 6 indices.
      ++curSlice;
   }
   // Kill range of the remaining slices.
   for( ; curSlice < FontData::MAX_SLICES; ++curSlice )
   {
      _sliceSize[curSlice] = 0;
   }

   RCP<Gfx::IndexBuffer> indexBuffer;
   RCP<Gfx::VertexBuffer> vertexBuffer;

   // Create buffers if not done yet.
   if( _geom->numBuffers() > 0 )
   {
      indexBuffer  = _geom->indexBuffer();
      vertexBuffer = _geom->buffers()[0];
   }
   else
   {
      indexBuffer  = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
      vertexBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
      vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F, 0 );
      vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 8 );

      _geom->indexBuffer( indexBuffer );
      _geom->addBuffer( vertexBuffer );
   }

   // Fill buffer.
   Core::gfx()->setData( indexBuffer, indices.dataSize(), indices.data() );
   Core::gfx()->setData( vertexBuffer, vertices.dataSize(), vertices.data() );

   int l = (int)vertices[0].x;
   int r = (int)vertices[vertices.size()-2].x;
   int b = (int)yRange.min();
   int t = (int)yRange.max();
   _scissored = !contains(
      Vec2i::zero(), _size,
      Vec2i( l, b ), Vec2i( r-l, t-b )
   );
}

//------------------------------------------------------------------------------
//!
const char*
Text::meta() const
{
   return _text_str_;
}

//------------------------------------------------------------------------------
//!
void
Text::init( VMState* vm )
{
   VM::get( vm, 1, "color", _color );
   VM::get( vm, 1, "text", _text );
   VM::get( vm, 1, "alignH", _alignH );
   VM::get( vm, 1, "alignV", _alignV );
   VM::get( vm, 1, "orientation", _orient );
   VM::get( vm, 1, "offset", _offset );

   // Font.
   VM::push( vm, "font" );
   VM::get( vm, 1 );
   if( !VM::isNil( vm, -1 ) )
   {
      // Read font name.
      String fontID;
      if( VM::geti( vm, -1, 1 ) )
      {
         fontID = VM::toString( vm, -1 );
         VM::pop( vm );
      }

      // Read font size.
      int fontSize = 0;
      if( VM::geti( vm, -1, 2 ) )
      {
         fontSize = VM::toInt( vm, -1 );
         VM::pop( vm );
      }

      // Read font scalable flag.
      bool scalable = GlyphManager::defaultScalable();
      if( VM::geti( vm, -1, 3 ) )
      {
         scalable = VM::toBoolean( vm, -1 );
         VM::pop( vm );
      }

      font( data( ResManager::getFont( fontID ) ), fontSize, scalable );
   }
   VM::setTop( vm, 0 );

   _constants->setConstant( "color", _color.ptr() );
   update();
}

//------------------------------------------------------------------------------
//!
bool
Text::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_TEXT: VM::push( vm, _text );
         return true;
      case ATTRIB_COLOR: VM::push( vm, _color );
         return true;
      case ATTRIB_FONT: /// @todo - We need to finish this case.
         return true;
      case ATTRIB_ORIENTATION: VM::push( vm, _orient );
         return true;
      case ATTRIB_ALIGNH: VM::push( vm, _alignH );
         return true;
      case ATTRIB_ALIGNV: VM::push( vm, _alignV );
         return true;
      case ATTRIB_POSITION: VM::push( vm, _position );
         return true;
      case ATTRIB_SIZE: VM::push( vm, _size );
         return true;
      case ATTRIB_TEXT_SIZE: VM::push( vm, _textSize );
         return true;
      case ATTRIB_OFFSET: VM::push( vm, _offset );
         return true;
      case ATTRIB_CHAR_POSITION: VM::push( vm, this, charPositionVM );
         return true;
      case ATTRIB_CHAR_INDEX: VM::push( vm, this, charIndexVM );
         return true;
      default: break;
   }
   return Drawable::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Text::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_TEXT: text( VM::toCString( vm, 3 ) );
         return true;
      case ATTRIB_COLOR: color( VM::toVec4f( vm, 3 ) );
         return true;
      case ATTRIB_FONT:
         {
            if( VM::isTable( vm, 3 ) )
            {
               // Read font name.
               if( !VM::geti( vm, 3, 1 ) )
               {
                  StdErr << "Text::performSet() - Error parsing font name." << nl;
                  return true;  // Don't touch the current font.
               }
               String fontID = VM::toString( vm, -1 );
               VM::pop( vm );

               // Read font size.
               if( !VM::geti( vm, 3, 2 ) )
               {
                  StdErr << "Text::performSet() - Error parsing font size." << nl;
                  return true;  // Don't touch the current font.
               }
               int fontSize = VM::toInt( vm, -1 );
               VM::pop( vm );

               bool scalable = GlyphManager::defaultScalable();
               if( VM::geti( vm, 3, 3 ) )
               {
                  scalable = VM::toBoolean( vm, -1 );
                  VM::pop( vm );
               }

               // Assign new font.
               font( data( ResManager::getFont( fontID ) ), fontSize, scalable );
            }
            else
            if( VM::isNil( vm, 3 ) )
            {
               font( NULL, 0, false );
            }
            else
            {
               StdErr << "Text::performSet() - Unexpected format for font." << nl;
               return true;  // Don't touch the current font.
            }
         }
         return true;
      case ATTRIB_ORIENTATION: orientation( (Orientation)VM::toInt( vm, 3 ) );
         return true;
      case ATTRIB_ALIGNH:
         alignment( (Alignment)VM::toInt( vm, 3 ), (Alignment)_alignV );
         return true;
      case ATTRIB_ALIGNV:
         alignment( (Alignment)_alignH, (Alignment)VM::toInt( vm, 3 ) );
         return true;
      case ATTRIB_POSITION: position( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_SIZE: size( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_TEXT_SIZE: // Read only.
         return true;
      case ATTRIB_OFFSET: offset( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_CHAR_POSITION: // Read only.
         return true;
      case ATTRIB_CHAR_INDEX: // Read only.
         return true;
      default: break;
   }

   return Drawable::performSet( vm );
}

NAMESPACE_END
