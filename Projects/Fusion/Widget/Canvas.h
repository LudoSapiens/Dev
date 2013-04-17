/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_CANVAS_H
#define FUSION_CANVAS_H

#include <Fusion/StdDefs.h>

#include <Fusion/Core/Event.h>
#include <Fusion/Widget/WidgetContainer.h>

#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Canvas
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read only:
//!     getCanvasRect():  Returns a vec4f containing the pos and size of the
//!       canvas.
//!
//!   read/write:
//!     offset: Relative position of canvas content.

class Canvas 
   : public WidgetContainer
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/
   
   FUSION_DLL_API Canvas();
  
   FUSION_DLL_API Vec4f canvasRect() const;
   FUSION_DLL_API void  offset( const Vec2f& val );
   
   FUSION_DLL_API void addOnModify( const Delegate1<const RCP<Widget>&>& );
   FUSION_DLL_API void removeOnModify( const Delegate1<const RCP<Widget>&>& );
 
   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~Canvas();

   virtual void render( const RCP<Gfx::RenderNode>& );
   virtual bool isAttribute( const char* ) const;

   virtual Vec2f performComputeBaseSize();
   virtual void performSetGeometry();
   virtual void performSetPosition();

   bool isScissored() const;

   void modified();
   
private:

   /*----- data members -----*/

   Delegate1List<const RCP<Widget>&> _onModify;
   VMRef                             _onModifyRef;
   
   Vec2f         _offset;
   
   Vec2f         _scPos;
   Vec2f         _scSize;

   mutable bool  _canvasRectCached;
   mutable Vec4f _canvasRect;
   
};

NAMESPACE_END

#endif
