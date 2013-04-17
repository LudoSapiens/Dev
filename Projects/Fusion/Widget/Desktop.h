/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_DESKTOP_H
#define FUSION_DESKTOP_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/WidgetContainer.h>

#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Desktop
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read only:
//!     popup( widget, pos, prevWidget ): Pop-up widget at position pos if
//!         possible. If not (near window border), ajust the position taking
//!         prevWidget into account, if given.
//!                                    
//!     closePopupsAfter( widget ): Close all popup widgets that have been open
//!         after widget.
//!
//!   read/write:
//!     color: Background color. Used with glClearColor.

class Desktop:
   public WidgetContainer
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Desktop();
   
   FUSION_DLL_API void color( const Vec4f& val );
   inline const Vec4f& color() const;
   
   FUSION_DLL_API void modal( const RCP<Widget>& widget, const Vec2f& pos, uint pointerID = 0 );

   FUSION_DLL_API void popup(
      const RCP<Widget>& widget, 
      const Vec2f&       pos, 
      const Widget*      prevWidget
   );
   FUSION_DLL_API void closePopups();
   FUSION_DLL_API void closePopupsAfter( const RCP<Widget>& );
      
   FUSION_DLL_API virtual void render( const RCP<Gfx::RenderNode>& );

   FUSION_DLL_API virtual Vec2f localToAbsPosition( const Widget& child ) const;
   FUSION_DLL_API virtual Vec2f absToLocalPosition( const Widget& child, const Vec2f& absPos ) const;

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/
   
   virtual ~Desktop();

   virtual Vec2f performComputeBaseSize();
   virtual void performSetGeometry();
   virtual void performSetPosition();

   virtual bool isAttribute( const char* ) const;

private:

   /*----- data members -----*/

   Vec4f _color;

};

//------------------------------------------------------------------------------
//!
inline const Vec4f&
Desktop::color() const
{
   return _color;
}

NAMESPACE_END

#endif
