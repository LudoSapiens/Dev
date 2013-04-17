/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_SPLITTER_H
#define FUSION_SPLITTER_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/WidgetContainer.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Splitter
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read only:
//!     splitPos:  Absolute local position of the split (independant of anchor)
//!
//!   read/write:
//!     gap:  Space between the two widget.
//!     orient:  Orientation of the split. 0 = Horizontal and 1 = Vertical
//!         split.
//!     ratio:  Ratio of the space taken by the first widget. The ratio is
//!         defined between 0 and 1.

class Splitter
   : public WidgetContainer
{

public:

   /*----- types and enumerations ----*/

   enum Orientation
   {
      HORIZONTAL,
      VERTICAL
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Splitter();

   FUSION_DLL_API void ratio( float val );

   FUSION_DLL_API virtual void render( const RCP<Gfx::RenderNode>& );

   //! Event handlers
   FUSION_DLL_API virtual void onPointerMove( const Event& );
   FUSION_DLL_API virtual void onPointerPress( const Event& );
   FUSION_DLL_API virtual void onPointerRelease( const Event& );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~Splitter();

   virtual Vec2f performComputeBaseSize();
   virtual void performSetGeometry();

   virtual bool isAttribute( const char* ) const;

private:

   /*----- data members -----*/

   float _gap;
   int   _orient;
   float _ratio;
   Vec2f _splitPos;
   bool  _lock;
};

NAMESPACE_END

#endif

