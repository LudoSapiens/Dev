/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Desktop.h>

#include <Fusion/Core/Core.h>
#include <Fusion/VM/VMObjectPool.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_COLOR,
   ATTRIB_MODAL,
   ATTRIB_POPUP,
   ATTRIB_CLOSE_POPUPS_AFTER
};

StringMap _attributes(
   "color",            ATTRIB_COLOR,
   "modal",            ATTRIB_MODAL,
   "popup",            ATTRIB_POPUP,
   "closePopupsAfter", ATTRIB_CLOSE_POPUPS_AFTER,
   ""
);

//------------------------------------------------------------------------------
//!
int
modalVM( VMState* vm )
{
   Desktop* desktop   = (Desktop*)VM::thisPtr( vm );
   RCP<Widget> widget = (Widget*)VM::toProxy( vm, 1 );
   Vec2f pos          = VM::toVec2f( vm, 2 );

   desktop->modal( widget, pos );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
popupVM( VMState* vm )
{
   Desktop* desktop   = (Desktop*)VM::thisPtr( vm );
   RCP<Widget> widget = (Widget*)VM::toProxy( vm, 1 );
   Vec2f pos          = VM::toVec2f( vm, 2 );

   Widget* prevWidget = NULL;

   // Read other widget
   if( VM::getTop( vm ) > 2 )
   {
      prevWidget = (Widget*)VM::toProxy( vm, 3 );
   }

   desktop->popup( widget, pos, prevWidget );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
closePopupsAfterVM( VMState* vm )
{
   Desktop* desktop   = (Desktop*)VM::thisPtr( vm );
   RCP<Widget> widget = (Widget*)VM::toProxy( vm, 1 );

   desktop->closePopupsAfter( widget );

   return 0;
}

//------------------------------------------------------------------------------
//!
const char* _desktop_str_ = "desktop";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Desktop
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Desktop::initialize()
{
   VMObjectPool::registerObject( "UI", _desktop_str_, stdCreateVM<Desktop>, stdGetVM<Desktop>, stdSetVM<Desktop> );
}

//------------------------------------------------------------------------------
//!
Desktop::Desktop()
   : WidgetContainer(), _color( 0.0f, 0.0f, 0.0f, 1.0f )
{
   intangible( false );
}

//------------------------------------------------------------------------------
//!
Desktop::~Desktop()
{}

//------------------------------------------------------------------------------
//!
void
Desktop::color( const Vec4f& val )
{
   _color = val;
}

//------------------------------------------------------------------------------
//!
void
Desktop::modal( const RCP<Widget>& widget, const Vec2f& pos, uint pointerID )
{

   if( (!Core::grabbedKeyboard() && !Core::pointer(pointerID).grabbed()) || widget->parent() == this )
   {
      if( widget->parent() != this )
      {
         if( !Core::grabKeyboard( widget.ptr() ) )
         {
            printf("Failed to grab keyboard...  Aborting.\n");
            return;
         }

         if( !Core::grabPointer(pointerID, widget.ptr()) )
         {
            printf("Failed to grab pointer...  Aborting.\n");
            Core::releaseKeyboard( widget.ptr() );
            return;
         }
      }

      // Add to Desktop
      addWidget( widget );

      // Corrects position
      Vec2f size   = widget->actualBaseSize();
      Vec2f newPos = pos;

      // First correct y positionning.
      if( newPos.y < globalPosition().y )
      {
         newPos.y = globalPosition().y;
      }
      else if( newPos.y + size.y > globalPosition().y + actualSize().y )
      {
         newPos.y = globalPosition().y + actualSize().y - size.y;
      }

      // Second, corrects x positionning.
      if( newPos.x < globalPosition().x )
      {
         newPos.x = globalPosition().x;
      }
      else if( newPos.x + size.x > globalPosition().x + actualSize().x )
      {
         newPos.x = globalPosition().x + actualSize().x - size.x;
      }

      widget->localPosition( newPos );
   }
}

//------------------------------------------------------------------------------
//!
void
Desktop::popup(
   const RCP<Widget>& widget,
   const Vec2f&       pos,
   const Widget*      prevWidget
)
{
   widget->popup( true );

   // Add to Desktop
   addWidget( widget );

   // Corrects position
   Vec2f size   = widget->actualBaseSize();
   Vec2f newPos = pos;

   // First correct y positionning.
   if( newPos.y < globalPosition().y )
   {
      newPos.y = globalPosition().y;
   }
   else if( newPos.y + size.y > globalPosition().y + actualSize().y )
   {
      newPos.y = globalPosition().y + actualSize().y - size.y;
   }

   // Second, corrects x positionning.
   if( newPos.x < globalPosition().x )
   {
      newPos.x = globalPosition().x;
   }
   else if( newPos.x + size.x > globalPosition().x + actualSize().x )
   {
      if( prevWidget )
      {
         newPos.x = newPos.x - size.x - prevWidget->actualSize().x;
      }
      else
      {
         newPos.x = globalPosition().x + actualSize().x - size.x;
      }
   }

   widget->localPosition( newPos );
}

//------------------------------------------------------------------------------
//!
void
Desktop::closePopups()
{
   Container::Iterator it  = _widgets.begin();
   Container::Iterator end = _widgets.end();

   for( ; it != end; ++it )
   {
      if( (*it)->isPopup() )
      {
         (*it)->popup( false );
         (*it)->parent( NULL );
         (*it) = NULL;
      }
   }

   // remove every null widget from list
   _widgets.removeAll( RCP<Widget>(0) );

   markForUpdate();
}

//------------------------------------------------------------------------------
//!
void
Desktop::closePopupsAfter( const RCP<Widget>& widget )
{
   // Find popup widget.
   Widget* popup = widget.ptr();

   while( popup && !popup->isPopup() )
   {
      popup = popup->parent();
   }

   // The widget is not contained in any popup, so close all popups.
   if( !popup )
   {
      closePopups();
      return;
   }

   Container::Iterator it  = _widgets.begin();
   Container::Iterator end = _widgets.end();

   bool found = false;

   for( ; it != end; ++it )
   {
      if( found )
      {
         if( (*it)->isPopup() )
         {
            (*it)->popup( false );
            (*it)->parent( NULL );
            (*it) = NULL;
         }
      }
      else
      {
         if( (*it) == popup ) found = true;
      }

   }

   // remove every null widget from list
   _widgets.removeAll( RCP<Widget>(0) );
   markForUpdate();
}

//------------------------------------------------------------------------------
//!
void
Desktop::render( const RCP<Gfx::RenderNode>& rn )
{
   if( true ) // For now we always scissor...
   {
      const int* sc = rn->current()->addScissor(
         (int)globalPosition().x, (int)globalPosition().y,
         (int)actualSize().x, (int)actualSize().y
      );
      WidgetContainer::render( rn );
      rn->current()->setScissor( sc[0], sc[1], sc[2], sc[3] );
   }
   else
   {
      WidgetContainer::render( rn );
   }
}

//------------------------------------------------------------------------------
//!
Vec2f
Desktop::localToAbsPosition( const Widget& child ) const
{
   Vec2f absPos = child.localPosition();

   if( anchor() & INVERT_X )
   {
      switch( child.alignH() )
      {
         case START:  absPos.x = actualSize().x - child.actualSize().x - absPos.x; break;
         case MIDDLE: absPos.x = (actualSize().x - child.actualSize().x)/2.0f - absPos.x; break;
      }
   }
   else
   {
     switch( child.alignH() )
      {
         case MIDDLE: absPos.x = (actualSize().x - child.actualSize().x)/2.0f + absPos.x; break;
         case END:    absPos.x = actualSize().x - child.actualSize().x - absPos.x; break;
      }
   }

   if( anchor() & INVERT_Y )
   {
      switch( child.alignV() )
      {
         case START:  absPos.y = actualSize().y - child.actualSize().y - absPos.y; break;
         case MIDDLE: absPos.y = (actualSize().y - child.actualSize().y)/2.0f - absPos.y; break;
      }
   }
   else
   {
     switch( child.alignV() )
      {
         case MIDDLE: absPos.y = (actualSize().y - child.actualSize().y)/2.0f + absPos.y; break;
         case END:    absPos.y = actualSize().y - child.actualSize().y - absPos.y; break;
      }
   }

   return absPos;
}

//------------------------------------------------------------------------------
//!
Vec2f
Desktop::absToLocalPosition( const Widget& child, const Vec2f& absPos ) const
{
   Vec2f localPos = absPos;

   if( anchor() & INVERT_X )
   {
      switch( child.alignH() )
      {
         case START:  localPos.x = actualSize().x - child.actualSize().x - localPos.x; break;
         case MIDDLE: localPos.x = (actualSize().x - child.actualSize().x)/2.0f - localPos.x; break;
      }
   }
   else
   {
     switch( child.alignH() )
      {
         case MIDDLE: localPos.x = -(actualSize().x - child.actualSize().x)/2.0f + localPos.x; break;
         case END:    localPos.x = actualSize().x - child.actualSize().x - localPos.x; break;
      }
   }

   if( anchor() & INVERT_Y )
   {
      switch( child.alignV() )
      {
         case START:  localPos.y = actualSize().y - child.actualSize().y - localPos.y; break;
         case MIDDLE: localPos.y = (actualSize().y - child.actualSize().y)/2.0f - localPos.y; break;
      }
   }
   else
   {
     switch( child.alignV() )
      {
         case MIDDLE: localPos.y = -(actualSize().y - child.actualSize().y)/2.0f + localPos.y; break;
         case END:    localPos.y = actualSize().y - child.actualSize().y - localPos.y; break;
      }
   }

   return localPos;
}

//------------------------------------------------------------------------------
//!
Vec2f
Desktop::performComputeBaseSize()
{
   return Vec2f( 0.0f, 0.0f );
}

//------------------------------------------------------------------------------
//!
void
Desktop::performSetGeometry()
{
   Container::ConstIterator it  = _widgets.begin();
   Container::ConstIterator end = _widgets.end();

   for( ; it != end; ++it )
   {
      if( (*it)->hidden() ) continue;

      bool maximize = (*it)->flexibility() > 0;
      Vec2f pos = maximize ? Vec2f(0.0f,0.0f) : (*it)->localPosition();

      if( (*it)->needUpdate() || maximize )
      {
         Vec2f size = maximize ? actualSize() : (*it)->actualBaseSize();
         (*it)->geometry( globalPosition(), pos, size );
      }
      else
      {
         (*it)->position( globalPosition(), pos );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
Desktop::performSetPosition()
{
   WidgetContainer::performSetPosition();
}

//------------------------------------------------------------------------------
//!
const char*
Desktop::meta() const
{
   return _desktop_str_;
}

//------------------------------------------------------------------------------
//!
void
Desktop::init( VMState* vm )
{
   VM::get( vm, 1, "color", _color );

   // Base class init.
   WidgetContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Desktop::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_COLOR:
         VM::push( vm, _color );
         return true;
      case ATTRIB_MODAL:
         VM::push( vm, this, modalVM );
         return true;
      case ATTRIB_POPUP:
         VM::push( vm, this, popupVM );
         return true;
      case ATTRIB_CLOSE_POPUPS_AFTER:
         VM::push( vm, this, closePopupsAfterVM );
         return true;
      default: break;
   }

   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Desktop::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_COLOR:
         _color = VM::toVec4f( vm, 3 );
         return true;
      case ATTRIB_MODAL: // Read only.
         return true;
      case ATTRIB_POPUP: // Read only.
         return true;
      case ATTRIB_CLOSE_POPUPS_AFTER: // Read only.
         return true;
      default: break;
   }

   return WidgetContainer::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Desktop::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return WidgetContainer::isAttribute( name );
}

NAMESPACE_END
