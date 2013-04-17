/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_TQUAD_H
#define FUSION_TQUAD_H

#include <Fusion/StdDefs.h>
#include <Fusion/Drawable/Drawable.h>

#include <CGMath/Vec4.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

class Image;

/*==============================================================================
  CLASS TQuad
==============================================================================*/

//!

class TQuad
   : public Drawable
{

public:

   /*----- types and enumerations ----*/

   enum Type
   {
      NORMAL         = 0,
      HORIZONTAL     = 1,
      VERTICAL       = 2,
      GRID           = 3,
      ADJUST_BORDERS = 4,
      CENTERED       = 5,
      CONTOUR        = 6
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API TQuad();

   FUSION_DLL_API virtual void draw( const RCP<Gfx::RenderNode>& ) const;

   inline         Type type() const       { return (Type)_type; }
   FUSION_DLL_API void type( Type );

   inline const Vec4f& u() const          { return _u;          }
   FUSION_DLL_API void u( const Vec4f& );

   inline const Vec4f& v() const          { return _v;          }
   FUSION_DLL_API void v( const Vec4f& );

   inline const Vec4f& color() const      { return _color;      }
   FUSION_DLL_API void color( const Vec4f& );

   inline const Vec2f& size() const       { return _size;       }
   FUSION_DLL_API void size( const Vec2f& );

   FUSION_DLL_API Vec2f position() const;
   FUSION_DLL_API void  position( const Vec2f& );

   inline const RCP<Image>& image() const { return _img;        }
   FUSION_DLL_API      void image( Image* img );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~TQuad();

   void update();

private:

   /*----- data members -----*/

   Vec2f                    _size;
   int                      _type;
   Vec4f                    _u;
   Vec4f                    _v;
   Vec4f                    _color;
   Mat4f                    _mat;
   RCP<Image>               _img;
   RCP<Gfx::Geometry>       _geom;
   RCP<Gfx::SamplerList>    _samplers;
   RCP<Gfx::ConstantBuffer> _constants;
   RCP<Gfx::ConstantList>   _cl;
};

NAMESPACE_END

#endif
