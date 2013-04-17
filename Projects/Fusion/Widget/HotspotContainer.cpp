/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/HotspotContainer.h>

#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/StringMap.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char* _hotspotContainer_str_ = "hotspots";

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_ADD_WIDGET,
   ATTRIB_REMOVE_WIDGET,
   ATTRIB_REMOVE_WIDGET_AT,
   ATTRIB_REMOVE_ALL_WIDGETS,
   ATTRIB_REMOVE_ALL_WIDGETS_FROM,
   ATTRIB_SET_WIDGET_HOTSPOT,
   ATTRIB_UNSET_WIDGET_HOTSPOT
};

StringMap _attributes(
   "addWidget"           ,  ATTRIB_ADD_WIDGET,
   "removeWidget"        ,  ATTRIB_REMOVE_WIDGET,
   "removeWidgetAt"      ,  ATTRIB_REMOVE_WIDGET_AT,
   "removeAllWidgets"    ,  ATTRIB_REMOVE_ALL_WIDGETS,
   "removeAllWidgetsFrom",  ATTRIB_REMOVE_ALL_WIDGETS_FROM,
   "setWidgetHotspot"    ,  ATTRIB_SET_WIDGET_HOTSPOT,
   "unsetWidgetHotspot"  ,  ATTRIB_UNSET_WIDGET_HOTSPOT,
   ""
);

const VM::EnumReg _enumHotspot[] = {
   { "TOP_LEFT"     ,  HotspotContainer::TOP_LEFT      },
   { "TOP_CENTER"   ,  HotspotContainer::TOP_CENTER    },
   { "TOP_RIGHT"    ,  HotspotContainer::TOP_RIGHT     },
   { "MIDDLE_LEFT"  ,  HotspotContainer::MIDDLE_LEFT   },
   { "MIDDLE_CENTER",  HotspotContainer::MIDDLE_CENTER },
   { "MIDDLE_RIGHT" ,  HotspotContainer::MIDDLE_RIGHT  },
   { "BOTTOM_LEFT"  ,  HotspotContainer::BOTTOM_LEFT   },
   { "BOTTOM_CENTER",  HotspotContainer::BOTTOM_CENTER },
   { "BOTTOM_RIGHT" ,  HotspotContainer::BOTTOM_RIGHT  },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.Hotspot", _enumHotspot );
}

//------------------------------------------------------------------------------
//!
bool setWidgetHotspotByTableVM( VMState* vm, HotspotContainer* c, int idx )
{
   if( !VM::isTable( vm, idx ) )
   {
      StdErr << "setWidgetHotspotByTableVM() - Did not receive a table." << nl;
      return false;
   }

   // 1st parameter.
   if( !VM::geti( vm, idx, 1 ) )
   {
      StdErr << "setWidgetHotspotByTableVM() - Missing widget." << nl;
      return false;
   }
   Widget* w = (Widget*)VM::toProxy( vm, -1 );
   VM::pop( vm );
   CHECK( w->parent() == c );

   // 2nd parameter.
   if( !VM::geti( vm, idx, 2 ) )
   {
      StdErr << "setWidgetHotspotByTableVM() - Missing 2nd parameter." << nl;
      return false;
   }
   if( VM::isNumber( vm, -1 ) )
   {
      // 2nd parameter: hotspot.
      HotspotContainer::Hotspot hs = (HotspotContainer::Hotspot)VM::toUInt( vm, -1 );
      VM::pop( vm );
      // 3rd parameter: offset.
      if( !VM::geti( vm, idx, 3 ) )
      {
         c->setWidgetHotspot( w, hs );
         return true;
      }
      else
      {
         Vec2f offset = VM::toVec2f( vm, -1 );
         VM::pop( vm );
         c->setWidgetHotspot( w, hs, offset );
         return true;
      }
   }
   else
   {
      // 2nd parameter: fParent.
      Vec2f fParent = VM::toVec2f( vm, -1 );
      VM::pop( vm );
      if( !VM::geti( vm, idx, 3 ) )
      {
         StdErr << "setWidgetHotspotByTableVM() - Missing fChild." << nl;
         return false;
      }
      // 3rd parameter: fChild.
      Vec2f fChild = VM::toVec2f( vm, -1 );
      VM::pop( vm );
      // 4th parameter: offset.
      if( !VM::geti( vm, idx, 4 ) )
      {
         c->setWidgetHotspot( w, fParent, fChild );
         return true;
      }
      else
      {
         Vec2f offset = VM::toVec2f( vm, -1 );
         VM::pop( vm );
         c->setWidgetHotspot( w, fParent, fChild, offset );
         return true;
      }
   }
}

//------------------------------------------------------------------------------
//!
int addWidgetVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   if( VM::isTable( vm, -1 ) )
   {
      RCP<Widget> w;
      if( VM::getiWidget( vm, -1, 1, w ) )
      {
         c->addWidget( w ); // Need to add the widget for the following call to work.
         setWidgetHotspotByTableVM( vm, c, -1 );
      }
      else
      {
         StdErr << "HotspotContainer::addWidgetVM() - First parameter of table not a widget." << nl;
      }
   }
   else
   {
      c->addWidget( (Widget*)VM::toProxy(vm, -1) );
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int removeWidgetVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   c->removeWidget( (Widget*)VM::toProxy( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int removeWidgetAtVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   c->removeWidget( VM::toUInt( vm, 1 ) );
   return 0;
}

#if 0
//------------------------------------------------------------------------------
//!
int removeWidgetsVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   if( VM::isTable( vm, -1 ) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push( vm );
      while( VM::next( vm, -2 ) )
      {
         Widget* w = (Widget*)VM::toProxy( vm, -1 );
         c->removeWidget( w );
         VM::pop( vm, 1 ); // Pop the value but keep the key.
      }
   }
   else
   {
      printf("Argument to HotspotContainer's removeWidgets isn't a table\n");
   }

   return 0;
}
#endif

//------------------------------------------------------------------------------
//!
int removeAllWidgetsVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   c->removeAllWidgets();
   return 0;
}


//------------------------------------------------------------------------------
//!
int removeAllWidgetsFromVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   c->removeAllWidgetsFrom( VM::toUInt( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int setWidgetHotspotVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   setWidgetHotspotByTableVM( vm, c, 1 );
   return 0;
}

//------------------------------------------------------------------------------
//!
int unsetWidgetHotspotVM( VMState* vm )
{
   HotspotContainer* c = (HotspotContainer*)VM::thisPtr( vm );
   Widget*           w = (Widget*)VM::toProxy( vm, 1 );
   c->unsetWidgetHotspot( w );
   return 0;
}

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
const char*
HotspotContainer::meta() const
{
   return _hotspotContainer_str_;
}

//------------------------------------------------------------------------------
//!
void
HotspotContainer::initialize()
{
   VMRegistry::add( initVM, VM_CAT_APP );
   VMObjectPool::registerObject( "UI", _hotspotContainer_str_, stdCreateVM<HotspotContainer>, stdGetVM<HotspotContainer>, stdSetVM<HotspotContainer> );
}

//------------------------------------------------------------------------------
//!
HotspotContainer::HotspotContainer()
{
   anchor( ANCHOR_BOTTOM_LEFT );
}

//------------------------------------------------------------------------------
//!
HotspotContainer::~HotspotContainer()
{
}

//------------------------------------------------------------------------------
//!
void
HotspotContainer::setWidgetHotspot( Widget* widget, Hotspot hs, const Vec2f& offset )
{
   HotspotInfo& hi = _hotspots[widget];
   static const Vec2f  parentChild[] = {
      Vec2f( 0.0f, 1.0f ), Vec2f( 0.5f, 1.0f ), Vec2f( 1.0f, 1.0f ),
      Vec2f( 0.0f, 0.5f ), Vec2f( 0.5f, 0.5f ), Vec2f( 1.0f, 0.5f ),
      Vec2f( 0.0f, 0.0f ), Vec2f( 0.5f, 0.0f ), Vec2f( 1.0f, 0.0f )
   };
   hi._fParent = hi._fChild = parentChild[hs];
   hi._offset  = offset;
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
HotspotContainer::setWidgetHotspot( Widget* widget, const Vec2f& fParent, const Vec2f& fChild, const Vec2f& offset )
{
   HotspotInfo& hi = _hotspots[widget];
   hi._fParent = fParent;
   hi._fChild  = fChild;
   hi._offset  = offset;
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
bool
HotspotContainer::isAttribute( const char* name ) const
{
   if( _attributes[name] != StringMap::INVALID ) return true;

   return WidgetContainer::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
void
HotspotContainer::performSetGeometry()
{
   Vec2f pSize = actualSize();
   for( auto cur = _widgets.begin(); cur != _widgets.end(); ++cur )
   {
      Widget* w = (*cur).ptr();
      if( !w->hidden() )
      {
         Vec2f p;
         Vec2f s = w->actualBaseSize();
         auto hsit = _hotspots.find( w );
         if( hsit != _hotspots.end() )
         {
            const HotspotInfo& hi = (*hsit).second;
            p  = hi._fParent * pSize;
            p -= hi._fChild * s;
            p += hi._offset;
         }
         else
         {
            p = w->localPosition();
            Vec2f flexSize = CGM::round( pSize * Vec2f( w->flexibilityH(), w->flexibilityV() ) );
            s = CGM::max( w->actualBaseSize(), flexSize );
         }
         w->geometry( globalPosition(), p, s );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
HotspotContainer::init( VMState* vm )
{
   if( VM::isTable( vm, 1 ) )
   {
      for( uint i = 1; VM::geti( vm, 1, i ); ++i )
      {
         if( VM::isTable( vm, -1 ) )
         {
            RCP<Widget> w;
            if( VM::getiWidget( vm, -1, 1, w ) )
            {
               addWidget( w ); // Need to add the widget for the following call to work.
               setWidgetHotspotByTableVM( vm, this, -1 );
            }
            else
            {
               StdErr << "HotspotContainer::init() - First parameter of table not a widget." << nl;
            }
         }
         else
         {
            addWidget( (Widget*)VM::toProxy(vm, -1) );
         }
         VM::pop( vm );
      }
      // Base class init.
      WidgetContainer::initParams( vm );
   }
}

//------------------------------------------------------------------------------
//!
bool
HotspotContainer::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ADD_WIDGET:
         VM::push( vm, this, addWidgetVM );
         return true;
      case ATTRIB_REMOVE_WIDGET:
         VM::push( vm, this, removeWidgetVM );
         return true;
      case ATTRIB_REMOVE_WIDGET_AT:
         VM::push( vm, this, removeWidgetAtVM );
         return true;
      case ATTRIB_REMOVE_ALL_WIDGETS:
         VM::push( vm, this, removeAllWidgetsVM );
         return true;
      case ATTRIB_REMOVE_ALL_WIDGETS_FROM:
         VM::push( vm, this, removeAllWidgetsFromVM );
         return true;
      case ATTRIB_SET_WIDGET_HOTSPOT:
         VM::push( vm, this, setWidgetHotspotVM );
         return true;
      case ATTRIB_UNSET_WIDGET_HOTSPOT:
         VM::push( vm, this, unsetWidgetHotspotVM );
         return true;
      default: break;
   }

   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
HotspotContainer::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ADD_WIDGET:
      case ATTRIB_REMOVE_WIDGET:
      case ATTRIB_REMOVE_WIDGET_AT:
      case ATTRIB_REMOVE_ALL_WIDGETS:
      case ATTRIB_REMOVE_ALL_WIDGETS_FROM:
      case ATTRIB_SET_WIDGET_HOTSPOT:
      case ATTRIB_UNSET_WIDGET_HOTSPOT:
         // Read-only.
         return true;
      default: break;
   }

   return WidgetContainer::performSet( vm );
}
