/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_BOX_H
#define FUSION_BOX_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/WidgetContainer.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Box
==============================================================================*/

//!
//!
//! Attributes list for WM:
//!
//!   read/write:
//!     gap:  Space between to consecutive widget.
//!     orient:  Orient of the children packing. 0 = Horizontal packing and
//!         1 = Vertical packing.

class Box
   : public WidgetContainer
{

public:

   /*----- types and enumerations ----*/

   enum Orientation
   {
      HORIZONTAL = 0,
      VERTICAL   = 1,
      OVERLAY    = 2
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Box();

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   FUSION_DLL_API virtual ~Box();

   Vec4f scrollRatio() const;

   FUSION_DLL_API virtual void render( const RCP<Gfx::RenderNode>& );

   FUSION_DLL_API virtual void onPointerScroll( const Event& );

   FUSION_DLL_API virtual Vec2f performComputeBaseSize();
   FUSION_DLL_API virtual void performSetGeometry();
   FUSION_DLL_API virtual void performSetPosition();

   FUSION_DLL_API virtual bool isAttribute( const char* ) const;

   /*----- data members -----*/

   // Members are not privates to allow override of the values
   // in a derived constructor.

   float _gap;
   int   _orient;

private:

   /*----- data members -----*/

   Vec2f _maxSize;
   Vec2f _offset;
   Vec2f _minOffset;
   Vec2f _maxOffset;
   bool  _scissored;
   Vec2f _scPos;
   Vec2f _scSize;
};

NAMESPACE_END

#endif

