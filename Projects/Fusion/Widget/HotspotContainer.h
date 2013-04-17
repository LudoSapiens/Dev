/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_WIDGET_HOTSPOT_CONTAINER
#define FUSION_WIDGET_HOTSPOT_CONTAINER

#include <Fusion/StdDefs.h>

#include <Fusion/Widget/WidgetContainer.h>

#include <Base/ADT/Map.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS HotspotContainer
==============================================================================*/
class HotspotContainer:
   public WidgetContainer
{
public:

   /*----- types -----*/
   enum Hotspot
   {
      TOP_LEFT,
      TOP_CENTER,
      TOP_RIGHT,
      MIDDLE_LEFT,
      MIDDLE_CENTER,
      MIDDLE_RIGHT,
      BOTTOM_LEFT,
      BOTTOM_CENTER,
      BOTTOM_RIGHT
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API HotspotContainer();
   FUSION_DLL_API virtual ~HotspotContainer();

   using WidgetContainer::addWidget;
   inline void addWidget( Widget* widget, Hotspot hs, const Vec2f& offset = Vec2f(0.0f) );
   inline void addWidget( Widget* widget, const Vec2f& fParent, const Vec2f& fChild, const Vec2f& offset = Vec2f(0.0f) );

   FUSION_DLL_API void setWidgetHotspot( Widget* widget, Hotspot hs, const Vec2f& offset = Vec2f(0.0f) );
   FUSION_DLL_API void setWidgetHotspot( Widget* widget, const Vec2f& fParent, const Vec2f& fChild, const Vec2f& offset = Vec2f(0.0f) );

   inline void unsetWidgetHotspot( Widget* widget );

   inline void removeWidget( Widget* widget );
   inline void removeWidget( uint index );
   inline void removeAllWidgets();
   inline void removeAllWidgetsFrom( uint index );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:


   /*----- methods -----*/

   FUSION_DLL_API virtual bool isAttribute( const char* ) const;

   FUSION_DLL_API virtual void performSetGeometry();

   /*----- types -----*/
   struct HotspotInfo
   {
      Vec2f    _fParent;
      Vec2f    _fChild;
      Vec2f    _offset;
   };

   typedef Map<Widget*, HotspotInfo>  HotspotInfoContainer;

   /*----- data members -----*/

   HotspotInfoContainer  _hotspots;

private:
}; //class HotspotContainer

//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::addWidget( Widget* widget, Hotspot hs, const Vec2f& offset )
{
   addWidget( widget );
   setWidgetHotspot( widget, hs, offset );
}

//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::addWidget( Widget* widget, const Vec2f& fParent, const Vec2f& fChild, const Vec2f& offset )
{
   addWidget( widget );
   setWidgetHotspot( widget, fParent, fChild, offset );
}

//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::unsetWidgetHotspot( Widget* widget )
{
   _hotspots.erase( widget );
}

//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::removeWidget( Widget* widget )
{
   _hotspots.erase( widget );
   WidgetContainer::removeWidget( widget );
}

//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::removeWidget( uint index )
{
   const RCP<Widget>& w = widget(index);
   _hotspots.erase( w.ptr() );
   WidgetContainer::removeWidget( w );
}
//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::removeAllWidgets()
{
   _hotspots.clear();
   WidgetContainer::removeAllWidgets();
}

//------------------------------------------------------------------------------
//!
inline void
HotspotContainer::removeAllWidgetsFrom( uint index )
{
   for( uint i = 0; i < numWidgets(); ++i )
   {
      _hotspots.erase( widget(i).ptr() );
   }
   WidgetContainer::removeAllWidgetsFrom( index );
}

NAMESPACE_END

#endif //FUSION_WIDGET_HOTSPOT_CONTAINER
