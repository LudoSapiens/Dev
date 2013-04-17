/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_TEXT_H
#define FUSION_TEXT_H

#include <Fusion/StdDefs.h>
#include <Fusion/Drawable/Drawable.h>
#include <Fusion/Resource/Font.h>

#include <CGMath/Vec4.h>

#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Text
==============================================================================*/

//!

class Text
   : public Drawable
{

public:

   /*----- types and enumerations ----*/

   enum Alignment
   {
      START     = 0,
      MIDDLE    = 1,
      END       = 2,
      BASELINE  = 3,
      MIDASCENT = 4,
   };

   enum Orientation
   {
      HORIZONTAL = 0,
      VERTICAL   = 1
   };

   /*----- static methods -----*/

   static void initialize();
   static void terminate();

   /*----- methods -----*/

   FUSION_DLL_API Text();

   FUSION_DLL_API virtual void draw( const RCP<Gfx::RenderNode>& ) const;

   FUSION_DLL_API void text( const String& );
   FUSION_DLL_API const String& text() const;
   
   FUSION_DLL_API void color( const Vec4f& );
   FUSION_DLL_API const Vec4f& color() const;
   
   FUSION_DLL_API void position( const Vec2f& );
   FUSION_DLL_API const Vec2f& position() const;

   FUSION_DLL_API void size( const Vec2f& );
   FUSION_DLL_API const Vec2f& size() const;

   FUSION_DLL_API void offset( const Vec2f& );
   FUSION_DLL_API const Vec2f& offset() const;
   
   FUSION_DLL_API void alignment( Alignment, Alignment );
   FUSION_DLL_API int alignmentH() const;
   FUSION_DLL_API int alignmentV() const;
   
   FUSION_DLL_API void orientation( Orientation );
   FUSION_DLL_API int orientation() const;
   
   FUSION_DLL_API void font( const RCP<Font>& font, int fontSize, bool scalable );
   inline const RCP<Font>& font()         const { return _font;         }
   inline              int fontSize()     const { return _fontSize;     }
   inline             bool fontScalable() const { return _fontScalable; }

   FUSION_DLL_API Vec2f charPosition( uint pos ) const;
   FUSION_DLL_API uint charIndex( const Vec2f& pos ) const;

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

   void update();

protected:

   /*----- methods -----*/
   
   virtual ~Text();

private:

   /*----- methods -----*/

   Vec2f displacement( Alignment, Alignment ) const;

   /*----- data members -----*/

   Vec2f                    _position;
   Vec2f                    _size;
   Vec2f                    _textSize;
   Vec2f                    _offset;
   RCP<Font>                _font;
   int                      _fontSize;
   bool                     _fontScalable;
   String                   _text;
   int                      _alignH;
   int                      _alignV;
   int                      _orient;
   Vec4f                    _color;
   bool                     _scissored;

   Mat4f                    _mat;
   RCP<Gfx::ConstantBuffer> _constants;
   RCP<Gfx::ConstantList>   _cl;
   RCP<Gfx::Geometry>       _geom;
   uint                     _sliceSize[FontData::MAX_SLICES]; //!< Tells the number of characters in each slice.
};

NAMESPACE_END

#endif
