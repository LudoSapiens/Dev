/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/WidgetContainer.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Core.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

#include <algorithm>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_wc, "WidgetContainer" );

const VM::EnumReg _enumsWidgetContainerAnchor[] = {
   { "INVERT_X"    ,  WidgetContainer::INVERT_X            },
   { "INVERT_Y"    ,  WidgetContainer::INVERT_Y            },
   { "BOTTOM_LEFT" ,  WidgetContainer::ANCHOR_BOTTOM_LEFT  },
   { "BOTTOM_RIGHT",  WidgetContainer::ANCHOR_BOTTOM_RIGHT },
   { "TOP_LEFT"    ,  WidgetContainer::ANCHOR_TOP_LEFT     },
   { "TOP_RIGHT"   ,  WidgetContainer::ANCHOR_TOP_RIGHT    },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.Anchor", _enumsWidgetContainerAnchor );
}

//------------------------------------------------------------------------------
//!
int
numWidgetsVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   uint numWidgets = container->numWidgets();
   VM::push( vm, numWidgets );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
addWidgetVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   RCP<Widget> widget         = (Widget*)VM::toProxy( vm, 1 );

   container->addWidget( widget );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
addWidgetAtVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   RCP<Widget> widget         = (Widget*)VM::toProxy( vm, 1 );
   uint idx                   = VM::toUInt( vm, 2 ) - 1;

   container->addWidget( widget, idx );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
addWidgetBeforeVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   RCP<Widget> widget         = (Widget*)VM::toProxy( vm, 1 );
   RCP<Widget> other          = (Widget*)VM::toProxy( vm, 2 );

   container->addWidgetBefore( widget, other );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
addWidgetsVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );

   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         RCP<Widget> widget = (Widget*)VM::toProxy( vm, -1 );
         if( widget.isValid() )
         {
            container->addWidget( widget );
         }
         else
         {
            printf("Error reading widget\n");
         }

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }
   }
   else
   {
      printf("Argument to WidgetContainer's addWidgets isn't a table\n");
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int
replaceWidgetVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   RCP<Widget> from           = (Widget*)VM::toProxy( vm, 1 );
   RCP<Widget> to             = (Widget*)VM::toProxy( vm, 2 );

   container->replaceWidget( from, to );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeWidgetVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   RCP<Widget> widget         = (Widget*)VM::toProxy( vm, 1 );

   container->removeWidget( widget );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeWidgetAtVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   container->removeWidget( VM::toUInt( vm, 1 ) - 1 );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeWidgetsVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );

   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         RCP<Widget> widget = (Widget*)VM::toProxy( vm, -1 );
         if( widget.isValid() )
         {
            container->removeWidget( widget );
         }
         else
         {
            printf("Error reading widget\n");
         }

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }
   }
   else
   {
      printf("Argument to WidgetContainer's removeWidgets isn't a table\n");
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeAllWidgetsVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   container->removeAllWidgets();

   return 0;
}


//------------------------------------------------------------------------------
//!
int
removeAllWidgetsFromVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   container->removeAllWidgetsFrom( VM::toUInt( vm, 1 ) );

   return 0;
}


//------------------------------------------------------------------------------
//!
int
moveToFrontVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   container->moveToFront( (Widget*)VM::toProxy( vm, 1 ) );

   return 0;
}


//------------------------------------------------------------------------------
//!
int
moveToBackVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   container->moveToBack( (Widget*)VM::toProxy( vm, 1 ) );

   return 0;
}


//------------------------------------------------------------------------------
//!
int
findWidgetVM( VMState* vm )
{
   WidgetContainer* container = (WidgetContainer*)VM::thisPtr( vm );
   String id                  = VM::toString( vm, 1 );

   VM::pushProxy( vm, container->findWidget( id ) );

   return 1;
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_BORDER,
   ATTRIB_NUM_WIDGETS,
   ATTRIB_ADD_WIDGET,
   ATTRIB_ADD_WIDGET_AT,
   ATTRIB_ADD_WIDGET_BEFORE,
   ATTRIB_ADD_WIDGETS,
   ATTRIB_REPLACE_WIDGET,
   ATTRIB_REMOVE_WIDGET,
   ATTRIB_REMOVE_WIDGETS,
   ATTRIB_REMOVE_WIDGET_AT,
   ATTRIB_REMOVE_ALL_WIDGETS,
   ATTRIB_REMOVE_ALL_WIDGETS_FROM,
   ATTRIB_MOVE_TO_FRONT,
   ATTRIB_MOVE_TO_BACK,
   ATTRIB_FIND_WIDGET,
   ATTRIB_ANCHOR,
   ATTRIB_DESIRED_SIZE,
   ATTRIB_INTANGIBLE
};

StringMap _attributes(
   "border",              ATTRIB_BORDER,
   "numWidgets",          ATTRIB_NUM_WIDGETS,
   "addWidget",           ATTRIB_ADD_WIDGET,
   "addWidgetAt",         ATTRIB_ADD_WIDGET_AT,
   "addWidgetBefore",     ATTRIB_ADD_WIDGET_BEFORE,
   "addWidgets",          ATTRIB_ADD_WIDGETS,
   "replaceWidget",       ATTRIB_REPLACE_WIDGET,
   "removeWidget",        ATTRIB_REMOVE_WIDGET,
   "removeWidgetAt",      ATTRIB_REMOVE_WIDGET_AT,
   "removeWidgets",       ATTRIB_REMOVE_WIDGETS,
   "removeAllWidgets",    ATTRIB_REMOVE_ALL_WIDGETS,
   "removeAllWidgetsFrom",ATTRIB_REMOVE_ALL_WIDGETS_FROM,
   "moveToFront",         ATTRIB_MOVE_TO_FRONT,
   "moveToBack",          ATTRIB_MOVE_TO_BACK,
   "findWidget",          ATTRIB_FIND_WIDGET,
   "anchor",              ATTRIB_ANCHOR,
   "desiredSize",         ATTRIB_DESIRED_SIZE,
   "intangible",          ATTRIB_INTANGIBLE,
   ""
);

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS WidgetContainer
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
WidgetContainer::initialize()
{
   VMRegistry::add( initVM, VM_CAT_APP );
}

//------------------------------------------------------------------------------
//!
WidgetContainer::WidgetContainer()
   : Widget(),
     _maxWidgets( 10000 ), // Arbitrary big number.
     _border( 0.0f, 0.0f, 0.0f, 0.0f ),
     _containerState( (1<<2) | ANCHOR_TOP_LEFT )
{
}

//------------------------------------------------------------------------------
//!
WidgetContainer::~WidgetContainer()
{
   removeAllWidgets();
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::init( VMState* vm )
{
   DBG_BLOCK( os_wc, "WidgetContainer::init" );

   // Read all widgets.
   RCP<Widget> widget;
   for( uint i = 1; i <= _maxWidgets; ++i )
   {
      if( VM::getiWidget( vm, 1, i, widget ) )
      {
         _widgets.pushBack( widget );
         widget->parent( this );
      }
      else
      {
         break;
      }
   }

   initParams( vm );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::initParams( VMState* vm )
{
   VM::get( vm, 1, "border", _border );

   int i;
   if( VM::get( vm, 1, "anchor", i ) ) anchor( i );

   bool b;
   if( VM::get( vm, 1, "intangible", b ) ) intangible( b );

   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::render( const RCP<Gfx::RenderNode>& rn )
{
   // Render the widget container.
   Widget::render( rn );

   // Render the contained widgets.
   Container::ConstIterator it  = _widgets.begin();
   Container::ConstIterator end = _widgets.end();

   for( ; it != end; ++it ) if( !(*it)->hidden() ) (*it)->render( rn );
}


//------------------------------------------------------------------------------
//!
Widget*
WidgetContainer::getWidgetAt( const Vec2f& pos )
{
   Container::ReverseIterator it  = _widgets.rbegin();
   Container::ReverseIterator end = _widgets.rend();
   for( ; it != end; ++it )
   {
      Widget* cur = (*it).ptr();
      if( !cur->hidden() && cur->eventsEnabled() && cur->isInside( pos ) )
      {
         Widget* w = cur->getWidgetAt( pos );
         if( w )  return w;
      }
   }

   return this->intangible() ? NULL : this;
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::getWidgetsAt( const Vec2f& pos, Vector<Widget*>& w )
{
   Container::ReverseIterator it  = _widgets.rbegin();
   Container::ReverseIterator end = _widgets.rend();
   for( ; it != end; ++it )
   {
      if( !(*it)->hidden() && (*it)->eventsEnabled() && (*it)->isInside( pos ) )
      {
         (*it)->getWidgetsAt( pos, w );
      }
   }

   if( !this->intangible() ) w.pushBack( this );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::addWidget( const RCP<Widget>& widget )
{
   if( widget.isNull() )  return;

   if( widget->parent() == this ) return;

   if( _widgets.size() >= _maxWidgets ) return;

   if( widget->parent() != 0 ) widget->parent()->removeWidget( widget );

   widget->parent( this );
   _widgets.pushBack( widget );

   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::addWidget( const RCP<Widget>& widget, uint index )
{
   if( widget.isNull() )  return;
   if( widget->parent() == this ) return;
   if( _widgets.size() >= _maxWidgets ) return;

   if( widget->parent() != 0 ) widget->parent()->removeWidget( widget );

   widget->parent( this );

   if( index >= _widgets.size() )
   {
      _widgets.pushBack( widget );
   }
   else
   {
      _widgets.insert( _widgets.begin()+index, widget );
   }

   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::addWidgetBefore( const RCP<Widget>& widget, const RCP<Widget>& other )
{
   size_t idx = _widgets.size();
   if( other->parent() == this )
   {
      for( size_t i = 0; i < _widgets.size(); ++i )
      {
         if( _widgets[i] == other )
         {
            idx = i;
            break;
         }
      }
   }
   addWidget( widget, uint(idx) );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::replaceWidget( const RCP<Widget>& from, const RCP<Widget>& by )
{
   if( from->parent() != this ) return;

   for( uint i = 0; i < _widgets.size(); ++i )
   {
      if( _widgets[i] == from )
      {
         _widgets[i]->parent( 0 );
         if( by->parent() != 0 ) by->parent()->removeWidget( by );
         by->parent( this );
         _widgets[i] = by;
         markForUpdate( true );
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::removeWidget( const RCP<Widget>& widget )
{
   if( _widgets.remove( widget ) )
   {
      widget->parent( 0 );
      markForUpdate( true );
   }
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::removeWidget( uint index )
{
   removeWidget( _widgets[index] );
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::removeAllWidgetsFrom( uint index )
{
   Container::Iterator it  = _widgets.begin() + index;
   Container::Iterator end = _widgets.end();

   for( ; it < end; ++it ) (*it)->parent( 0 );

   _widgets.erase( _widgets.begin() + index, end );
   markForUpdate( true );
}


//------------------------------------------------------------------------------
//!
void
WidgetContainer::moveToFront( const RCP<const Widget>& widget )
{
   Container::Iterator match = std::find( _widgets.begin(), _widgets.end(), widget );

   // Not found or already at the right place.
   if( match == _widgets.end() || match == _widgets.end()-1 ) return;

   std::rotate( match, match + 1, _widgets.end() );
   markForUpdate();
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::moveToBack( const RCP<const Widget>& widget )
{
   Container::Iterator match = std::find( _widgets.begin(), _widgets.end(), widget );

   // Not found or already at the right place.
   if( match == _widgets.end() || match == _widgets.begin() ) return;

   std::rotate( _widgets.begin(), match, match + 1 );
   markForUpdate();
}


//------------------------------------------------------------------------------
//!
Widget*
WidgetContainer::findWidget( const String& wid )
{
   if( wid == id() ) return this;

   Container::ReverseIterator it  = _widgets.rbegin();
   Container::ReverseIterator end = _widgets.rend();

   for( ; it != end; ++it )
   {
      Widget* widget = (*it)->findWidget( wid );
      if( widget ) return widget;
   }
   return NULL;
}


//------------------------------------------------------------------------------
//!
void
WidgetContainer::sendChildMessage( Widget* widget, int message )
{
   Container::Iterator it  = _widgets.begin();
   Container::Iterator end = _widgets.end();

   for( ; it != end; ++it )
   {
      (*it)->sendChildMessage( widget, message );
   }
}

//------------------------------------------------------------------------------
//!
Vec2f
WidgetContainer::localToAbsPosition( const Widget& child ) const
{
   Vec2f absPos = child.localPosition();

   if( anchor() & INVERT_X )
   {
      absPos.x = actualSize().x - absPos.x - child.actualSize().x;
   }

   if( anchor() & INVERT_Y )
   {
      absPos.y = actualSize().y - absPos.y - child.actualSize().y;
   }

   return absPos;
}

//------------------------------------------------------------------------------
//!
Vec2f
WidgetContainer::absToLocalPosition( const Widget& child, const Vec2f& absPos ) const
{
   Vec2f localPos = absPos;

   if( anchor() & INVERT_X )
   {
      localPos.x = actualSize().x - localPos.x - child.actualSize().x;
   }

   if( anchor() & INVERT_Y )
   {
      localPos.y = actualSize().y - localPos.y - child.actualSize().y;
   }

   return localPos;
}

//------------------------------------------------------------------------------
//!
void
WidgetContainer::performSetPosition()
{
   for( auto it = _widgets.begin(); it != _widgets.end(); ++it )
   {
      if( !(*it)->hidden() )
         (*it)->position( globalPosition(), (*it)->localPosition() );
   }
}

//------------------------------------------------------------------------------
//!
bool
WidgetContainer::performGet( VMState* vm )
{
   DBG_BLOCK( os_wc, "WidgetContainer::performGet" );

   // Is the requested attribute an index into widgets container?
   if( VM::isNumber( vm, 2 ) )
   {
      int idx = VM::toInt( vm, 2 )-1;
      if( idx < 0 || idx >= (int)_widgets.size() ) return false;
      DBG_MSG( os_wc, "Is widget at index " << idx );
      VM::pushProxy( vm, _widgets[idx].ptr() );
      return true;
   }

   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_BORDER:
         VM::push( vm, _border );
         return true;
      case ATTRIB_NUM_WIDGETS:
         VM::push( vm, this, numWidgetsVM );
         return true;
      case ATTRIB_ADD_WIDGET:
         VM::push( vm, this, addWidgetVM );
         return true;
      case ATTRIB_ADD_WIDGET_AT:
         VM::push( vm, this, addWidgetAtVM );
         return true;
      case ATTRIB_ADD_WIDGET_BEFORE:
         VM::push( vm, this, addWidgetBeforeVM );
         return true;
      case ATTRIB_ADD_WIDGETS:
         VM::push( vm, this, addWidgetsVM );
         return true;
      case ATTRIB_REPLACE_WIDGET:
         VM::push( vm, this, replaceWidgetVM );
         return true;
      case ATTRIB_REMOVE_WIDGET:
         VM::push( vm, this, removeWidgetVM );
         return true;
      case ATTRIB_REMOVE_WIDGETS:
         VM::push( vm, this, removeWidgetsVM );
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
      case ATTRIB_MOVE_TO_FRONT:
         VM::push( vm, this, moveToFrontVM );
         return true;
      case ATTRIB_MOVE_TO_BACK:
         VM::push( vm, this, moveToBackVM );
         return true;
      case ATTRIB_FIND_WIDGET:
         VM::push( vm, this, findWidgetVM );
         return true;
      case ATTRIB_ANCHOR:
         VM::push( vm, anchor() );
         return true;
      case ATTRIB_DESIRED_SIZE:
         VM::push( vm, desiredSize() );
         return true;
      case ATTRIB_INTANGIBLE:
         VM::push( vm, intangible() );
         return true;
      default: break;
   }
   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
WidgetContainer::performSet( VMState* vm )
{
   DBG_BLOCK( os_wc, "WidgetContainer::performSet(" << VM::toCString( vm, 2 ) << ")" );
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_BORDER:
         _border = VM::toVec4f( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_NUM_WIDGETS:
      case ATTRIB_ADD_WIDGET:
      case ATTRIB_ADD_WIDGET_AT:
      case ATTRIB_ADD_WIDGET_BEFORE:
      case ATTRIB_ADD_WIDGETS:
      case ATTRIB_REPLACE_WIDGET:
      case ATTRIB_REMOVE_WIDGET:
      case ATTRIB_REMOVE_WIDGETS:
      case ATTRIB_REMOVE_WIDGET_AT:
      case ATTRIB_REMOVE_ALL_WIDGETS:
      case ATTRIB_REMOVE_ALL_WIDGETS_FROM:
      case ATTRIB_MOVE_TO_FRONT:
      case ATTRIB_MOVE_TO_BACK:
      case ATTRIB_FIND_WIDGET:
         return true; // Read only.
      case ATTRIB_ANCHOR:
         anchor( VM::toInt( vm, 3 ) );
         return true;
      case ATTRIB_DESIRED_SIZE: // Read only.
         return true;
      case ATTRIB_INTANGIBLE:
         intangible( VM::toBoolean( vm, 3 ) );
         return true;
      default: break;
   }

   DBG_MSG( os_wc, "... not an index, so calling widget..." );
   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
WidgetContainer::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Widget::isAttribute( name );
}


NAMESPACE_END
