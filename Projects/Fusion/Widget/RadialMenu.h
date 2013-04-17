/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_RADIALMENU_H
#define FUSION_RADIALMENU_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/WidgetContainer.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RadialMenu
==============================================================================*/

//!
//!
//! Attributes list for WM:
//!
//!   read/write:
//!

class RadialMenu
   : public WidgetContainer
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API RadialMenu();

   FUSION_DLL_API virtual void  highlight( const uint widgetIndex );
   FUSION_DLL_API virtual void  select( const uint widgetIndex );

   // Event handlers
   //FUSION_DLL_API virtual void onClick( const Event& );
   FUSION_DLL_API virtual void onPointerPress( const Event& );
   FUSION_DLL_API virtual void onPointerRelease( const Event& );
   FUSION_DLL_API virtual void onPointerMove( const Event& );
   //FUSION_DLL_API virtual void onPointerEnter( const Event& );
   //FUSION_DLL_API virtual void onPointerLeave( const Event& );
   FUSION_DLL_API virtual void onKeyPress( const Event& );
   FUSION_DLL_API virtual void onKeyRelease( const Event& );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~RadialMenu();

   virtual void render( const RCP<Gfx::RenderNode>& );

   virtual Vec2f performComputeBaseSize();
   virtual void performSetGeometry();
   virtual void performSetPosition();
   
   virtual bool isAttribute( const char* ) const;

private:

   /*----- data members -----*/

   float   _radius;
   Vec2f   _center;
   bool    _pressed;
   Widget* _currentWidget;
};

NAMESPACE_END

#endif

