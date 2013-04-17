/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_WIDGETCONTAINER_H
#define FUSION_WIDGETCONTAINER_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <CGMath/Vec4.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS WidgetContainer
==============================================================================*/

//! Base class for Widget that can contains many widgets.
//!
//! Attributes list for WM:
//!
//!   read only:
//!     addWidget( Widget ):  Add a widget at the end to the container.
//!     removeWidget( Widget ):  Remove a widget from the container.
//!     removeAllWidgets      :  Remove all the widgets from the container.
//!     removeAllWidgetsFrom( idx ) : Remove all widgets starting with a given index
//!     findWidget( id ):  Search and return a widget contained in the children
//!         hierarchy. For now it's a depth first search.
//!
//!   read/write:
//!     border: Border around the contained children widgets, independant of anchor
//!          { x1, y1, x2, y2 }.
//!                x1 = left border
//!                y1 = bottom border
//!                x2 = right border
//!                y2 = top border
//!     anchor: Anchor point
//!                 0 = Bottom Left
//!                 1 = Bottom Right
//!                 2 = Top Left
//!                 3 = Top Right

class WidgetContainer
   : public Widget
{

public:

   /*----- types and enumerations ----*/

   enum {
      INVERT_X = 1,
      INVERT_Y = 2
   };

   enum {
      ANCHOR_BOTTOM_LEFT  = 0,
      ANCHOR_BOTTOM_RIGHT = INVERT_X,
      ANCHOR_TOP_LEFT     = INVERT_Y,
      ANCHOR_TOP_RIGHT    = INVERT_X | INVERT_Y
   };


   typedef Vector< RCP<Widget> > Container;

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   inline const Vec4f& border() const;
   // inline void border( const Vec4f& );

   inline void  anchor( int v );
   inline int   anchor() const;

   inline bool  intangible() const;
   inline void  intangible( bool v );

   //! See Widget
   //{
   FUSION_DLL_API virtual void render( const RCP<Gfx::RenderNode>& );
   FUSION_DLL_API virtual Widget* getWidgetAt( const Vec2f& pos );
   FUSION_DLL_API virtual void getWidgetsAt( const Vec2f& pos, Vector<Widget*>& );
   FUSION_DLL_API virtual Widget* findWidget( const String& id );
   //}

   FUSION_DLL_API void addWidget( const RCP<Widget>& widget );
   FUSION_DLL_API void addWidget( const RCP<Widget>& widget, uint index );
   FUSION_DLL_API void addWidgetBefore( const RCP<Widget>& widget, const RCP<Widget>& other );
   FUSION_DLL_API void replaceWidget( const RCP<Widget>& from, const RCP<Widget>& by );
   FUSION_DLL_API void removeWidget( const RCP<Widget>& widget );
   FUSION_DLL_API void removeWidget( uint index );
   inline void removeAllWidgets();
   FUSION_DLL_API void removeAllWidgetsFrom( uint index );
   FUSION_DLL_API void moveToFront( const RCP<const Widget>& widget );
   FUSION_DLL_API void moveToBack( const RCP<const Widget>& widget );

   FUSION_DLL_API virtual void sendChildMessage( Widget* widget, int message );

   inline uint  numWidgets() const;
   inline const Container& widgets() const;
   inline const RCP<Widget>& widget( uint ) const;

   FUSION_DLL_API virtual Vec2f localToAbsPosition( const Widget& child ) const;
   FUSION_DLL_API virtual Vec2f absToLocalPosition( const Widget& child, const Vec2f& absPos ) const;

   inline Vec2f desiredSize();

protected:

   /*----- methods -----*/

   FUSION_DLL_API WidgetContainer();
   FUSION_DLL_API virtual ~WidgetContainer();

   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API void initParams( VMState* vm );

   FUSION_DLL_API virtual void performSetPosition();

   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );
   FUSION_DLL_API virtual bool isAttribute( const char* ) const;

   /*----- data members -----*/

   Container  _widgets;
   uint       _maxWidgets;
   Vec4f      _border;
   //! NAME          SIZE   LOCATION        DESCRIPTION
   //! anchor        (2)   _fields[ 1: 0]
   //! intangible    (1)   _fields[ 2: 2]
   int        _containerState;

private:
};

//------------------------------------------------------------------------------
//!
inline const Vec4f&
WidgetContainer::border() const
{
   return _border;
}

//------------------------------------------------------------------------------
//!
inline int
WidgetContainer::anchor() const
{
   return getbits( _containerState, 0, 2 );
}

//------------------------------------------------------------------------------
//!
inline void
WidgetContainer::anchor( int v )
{
   _containerState = setbits( _containerState, 0, 2, v );
   markForUpdate();
}

//------------------------------------------------------------------------------
//!
inline bool
WidgetContainer::intangible() const
{
   return getbits( _containerState, 2, 1 ) != 0x0;
}

//------------------------------------------------------------------------------
//!
inline void
WidgetContainer::intangible( bool v )
{
   _containerState = setbits( _containerState, 2, 1, v?1:0 );
}

//------------------------------------------------------------------------------
//!
inline void
WidgetContainer::removeAllWidgets()
{
   removeAllWidgetsFrom( 0 );
}

//------------------------------------------------------------------------------
//!
inline uint
WidgetContainer::numWidgets() const
{
   return uint(_widgets.size());
}

//------------------------------------------------------------------------------
//!
inline const WidgetContainer::Container&
WidgetContainer::widgets() const
{
   return _widgets;
}

//------------------------------------------------------------------------------
//!
inline const RCP<Widget>&
WidgetContainer::widget( uint i ) const
{
   return _widgets[i];
}

//------------------------------------------------------------------------------
//!
inline Vec2f
WidgetContainer::desiredSize()
{
   return performComputeBaseSize();
}

NAMESPACE_END

#endif
