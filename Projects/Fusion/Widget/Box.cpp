/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Box.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Event.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_GAP,
   ATTRIB_MAX_SIZE,
   ATTRIB_ORIENTATION,
   ATTRIB_SCROLL_RATIO
};

StringMap _attributes(
   "gap",         ATTRIB_GAP,
   "maxSize",     ATTRIB_MAX_SIZE,
   "orientation", ATTRIB_ORIENTATION,
   "scrollRatio", ATTRIB_SCROLL_RATIO,
   ""
);

const VM::EnumReg _enumsBoxOrientation[] = {
   { "HORIZONTAL",  Box::HORIZONTAL },
   { "VERTICAL",    Box::VERTICAL   },
   { "OVERLAY",     Box::OVERLAY    },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
const char* _box_str_ = "box";


//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.BoxOrientation", _enumsBoxOrientation );
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS Box
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Box::initialize()
{
   VMObjectPool::registerObject( "UI", _box_str_, stdCreateVM<Box>, stdGetVM<Box>, stdSetVM<Box> );
   VMRegistry::add( initVM, VM_CAT_APP );
}

//------------------------------------------------------------------------------
//!
Box::Box()
   : WidgetContainer(),
     _gap( 0.0f ),
     _orient( HORIZONTAL ),
     _maxSize( CGConstf::infinity() ),
     _offset( 0.0f ),
     _minOffset( 0.0f ),
     _maxOffset( 0.0f ),
     _scissored( false )
{}

//------------------------------------------------------------------------------
//!
Box::~Box()
{}

//------------------------------------------------------------------------------
//!
Vec4f
Box::scrollRatio() const
{
   if( !_scissored ) return Vec4f(0.0f);
   Vec2f interval = _maxOffset - _minOffset;
   Vec2f r        = CGM::abs(_offset) / interval;
   Vec4f sr(0.0f);
   if( interval.x > 0.0f )
   {
      sr.x = r.x;
      sr.y = 1.0f-r.x;
   }
   if( interval.y > 0.0f )
   {
      sr.z = r.y;
      sr.w = 1.0f-r.y;
   }
   return sr;
}

//------------------------------------------------------------------------------
//!
void
Box::render( const RCP<Gfx::RenderNode>& rn )
{
   if( _scissored )
   {
      // Render the widget container.
      Widget::render( rn );

      // Render the contained widgets.
      const int* sc = rn->current()->addScissor( CGM::floori(_scPos.x), CGM::floori(_scPos.y), CGM::ceili(_scSize.x), CGM::ceili(_scSize.y) );

      for( auto it  = _widgets.begin(); it != _widgets.end(); ++it )
      {
         if( !(*it)->hidden() ) (*it)->render( rn );
      }
      rn->current()->setScissor( sc[0], sc[1], sc[2], sc[3] );
   }
   else
   {
      WidgetContainer::render( rn );
   }
}

//------------------------------------------------------------------------------
//!
void
Box::onPointerScroll( const Event& ev )
{
   if( _scissored )
   {
      _offset -= ev.scrollValue() * 2.0f;
      _offset  = CGM::clamp( _offset, _minOffset, _maxOffset );
      for( auto it = _widgets.begin(); it != _widgets.end(); ++it )
      {
         if( !(*it)->hidden() )
            (*it)->position( globalPosition() + _offset, (*it)->localPosition() );
      }
      callShader();
   }

   WidgetContainer::onPointerScroll( ev );
}

//------------------------------------------------------------------------------
//!
Vec2f
Box::performComputeBaseSize()
{
   auto it  = _widgets.begin();
   auto end = _widgets.end();

   Vec2f newBaseSize( 0.0f, 0.0f );

   if( _orient == OVERLAY )
   {
      // Draw every widget on top of one another
      for( ; it != end; ++it )
      {
         if( !(*it)->hidden() )
         {
            Vec2f childSize = (*it)->actualBaseSize();
            CGM::clampMin( newBaseSize.x, childSize.x );
            CGM::clampMin( newBaseSize.y, childSize.y );
         }
      }
   }
   else
   {
      int axis1 = _orient == HORIZONTAL ? 0 : 1;
      int axis2 = _orient == VERTICAL   ? 0 : 1;

      int nbVisible = 0;

      for( ; it != end; ++it )
      {
         if( !(*it)->hidden() )
         {
            Vec2f childSize = (*it)->actualBaseSize();
            newBaseSize( axis1 ) += childSize( axis1 );
            CGM::clampMin( newBaseSize( axis2 ), childSize( axis2 ) );
            ++nbVisible;
         }
      }

      // Add gaps.
      if( nbVisible > 0  ) newBaseSize( axis1 ) += ( nbVisible - 1 ) * _gap;
   }

   // Add border.
   newBaseSize.x += border().x + border().z;
   newBaseSize.y += border().y + border().w;

   // Compute the boundaries for the offset movement.
   int a = anchor();
   if( a & INVERT_X )
   {
      _minOffset.x = 0.0f;
      _maxOffset.x = CGM::max( 0.0f, newBaseSize.x - _maxSize.x );
   }
   else
   {
      _minOffset.x = CGM::min( 0.0f, _maxSize.x - newBaseSize.x );
      _maxOffset.x = 0.0f;
   }
   if( a & INVERT_Y )
   {
      _minOffset.y = 0.0f;
      _maxOffset.y = CGM::max( 0.0f, newBaseSize.y - _maxSize.y );
   }
   else
   {
      _minOffset.y = CGM::max( 0.0f, _maxSize.y - newBaseSize.y );
      _maxOffset.y = 0.0f;
   }
   // Clamp to maximum size.
   newBaseSize = CGM::min( newBaseSize, _maxSize );

   return newBaseSize;
}

//------------------------------------------------------------------------------
//!
void
Box::performSetGeometry()
{
   // Set-up scissoring, if needed
   _scPos.x  = globalPosition().x + border().x;
   _scPos.y  = globalPosition().y + border().y;
   _scSize.x = actualSize().x - border().x - border().z;
   _scSize.y = actualSize().y - border().y - border().w;

   // Clamping the offset to the defined region.
   _offset   = CGM::clamp( _offset, _minOffset, _maxOffset );

   auto it   = _widgets.begin();
   auto end  = _widgets.end();

   if( _orient == OVERLAY )
   {
      // Draw every widget on top of one another

      // Figure out minimal size for children
      Vec2f size(0,0);

      for( ; it != end; ++it )
      {
         if( !(*it)->hidden() )
         {
            Vec2f childSize = (*it)->actualBaseSize();
            CGM::clampMin( size.x, childSize.x );
            CGM::clampMin( size.y, childSize.y );
         }
      }

      // Compute available space
      Vec2f availableSize = actualSize();
      availableSize.x -= border().x + border().z;
      availableSize.y -= border().y + border().w;

      // If minimal size is smaller than our size, scissor
      if( availableSize.x < size.x || availableSize.y < size.y )
      {
         _scissored = true;
      }
      else
      {
         _scissored = false;
      }

      // Set children geometry.
      it  = _widgets.begin();

      Vec2f currPos( border()( (anchor()&INVERT_X)?2:0 ),
                     border()( (anchor()&INVERT_Y)?3:1 ) );

      Vec2f newPos;
      Vec2f newSize;

      for( ; it != end; ++it )
      {
         if( (*it)->hidden() ) continue;

         Vec2f childSize = (*it)->actualBaseSize();

         // Set widget position and size along both axis.
         for( uint axis = 0; axis < 2; ++axis )
         {
            switch( (*it)->alignHV( axis ) )
            {
               case FLEX:
               {
                  newPos( axis )  = currPos( axis );
                  newSize( axis ) = CGM::max( availableSize( axis ), childSize( axis ) );
               } break;
               case START:
               {
                  newPos( axis )  = currPos( axis );
                  newSize( axis ) = childSize( axis );
               } break;
               case MIDDLE:
               {
                  newPos( axis )  = currPos( axis ) + (availableSize( axis ) - childSize( axis ))/2.0f;
                  newSize( axis ) = childSize( axis );
               } break;
               case END:
               {
                  newPos( axis )  = currPos( axis ) + availableSize( axis ) - childSize( axis );
                  newSize( axis ) = childSize( axis );
               } break;
            }
         }
         (*it)->geometry( globalPosition() + _offset, newPos, newSize );
      }
   }
   else
   {
      int axis1 = _orient == HORIZONTAL ? 0 : 1;
      int axis2 = _orient == VERTICAL   ? 0 : 1;

      // Compute total size and total flexibility.
      //   size(0) : total size along stacking axis
      //   size(1) : total size along opposite of stacking axis
      float flex = 0.0f;
      Vec2f size(0.0f);

      auto lastVisibleFlexible = _widgets.end();
      int nbVisible = 0;

      for( ; it != end; ++it )
      {
         if( !(*it)->hidden() )
         {
            float widgetFlex = (*it)->flexibilityHV( axis1 );
            if( widgetFlex > 0.0f ) lastVisibleFlexible = it;
            Vec2f childSize  = (*it)->actualBaseSize();
            size.x          += childSize( axis1 );
            flex            += widgetFlex;
            CGM::clampMin( size.y, childSize( axis2 ) );
            ++nbVisible;
         }
      }

      // Add gap.
      if( nbVisible > 0  ) size.x += ( nbVisible - 1 ) * _gap;

      // Compute available space
      Vec2f availableSize = actualSize();
      availableSize.x -= border().x + border().z;
      availableSize.y -= border().y + border().w;

      // Compute flexibility value, determine if scissoring is needed.
      float flexSizeLeft = availableSize( axis1 ) - size.x;
      if( flexSizeLeft < 0.0f )
      {
         flexSizeLeft = 0.0f;
         _scissored   = true;
      }
      else if( availableSize( axis2 ) < size.y )
      {
         _scissored = true;
      }
      else
      {
         _scissored = false;
      }
      if( flex > 0.0f ) flex = flexSizeLeft / flex;

      // Set children geometry.
      it  = _widgets.begin();

      Vec2f currPos( border()( (anchor()&INVERT_X)?2:0 ),
                     border()( (anchor()&INVERT_Y)?3:1 ) );

      Vec2f newPos;
      Vec2f newSize;

      for( ; it != end; ++it )
      {
         if( (*it)->hidden() ) continue;

         Vec2f childSize = (*it)->actualBaseSize();

         // Set widget position and size along stacking axis.
         newPos( axis1 )  = currPos( axis1 );
         newSize( axis1 ) = childSize( axis1 );

         // Add flexibility if needed.
         float widgetFlex = (*it)->flexibilityHV( axis1 );

         if( widgetFlex > 0.0f )
         {
            float newFlexSize = 0.0f;
            if( it == lastVisibleFlexible  )
            {
               newFlexSize = flexSizeLeft;
            }
            else
            {
               newFlexSize = flex * widgetFlex;
               CGM::clampMax( newFlexSize, flexSizeLeft );
            }

            newSize( axis1 ) += newFlexSize;
            flexSizeLeft     -= newFlexSize;
         }

         // Set widget position and size along non-stacking axis.
         switch( (*it)->alignHV( axis2 ) )
         {
            case FLEX:
            {
               newPos( axis2 )  = currPos( axis2 );
               newSize( axis2 ) = CGM::max( availableSize( axis2 ), childSize( axis2 ) );
            } break;
            case START:
            {
               newPos( axis2 )  = currPos( axis2 );
               newSize( axis2 ) = childSize( axis2 );
            } break;
            case MIDDLE:
            {
               newPos( axis2 )  = currPos( axis2 ) + (availableSize( axis2 ) - childSize( axis2 ))/2.0f;
               newSize( axis2 ) = childSize( axis2 );
            } break;
            case END:
            {
               newPos( axis2 )  = currPos( axis2 ) + availableSize( axis2 ) - childSize( axis2 );
               newSize( axis2 ) = childSize( axis2 );
            } break;
         }

         (*it)->geometry( globalPosition() + _offset, newPos, newSize );

         currPos( axis1 ) += newSize( axis1 ) + _gap;
      }
   }
}

//------------------------------------------------------------------------------
//!
void Box::performSetPosition()
{
   for( auto it = _widgets.begin(); it != _widgets.end(); ++it )
   {
      if( !(*it)->hidden() )
         (*it)->position( globalPosition() + _offset, (*it)->localPosition() );
   }

   _scPos.x  =  globalPosition().x + border().x;
   _scPos.y  =  globalPosition().y + border().y;
}

//------------------------------------------------------------------------------
//!
const char*
Box::meta() const
{
   return _box_str_;
}

//------------------------------------------------------------------------------
//!
void
Box::init( VMState* vm )
{
   VM::get( vm, 1, "gap", _gap );
   VM::get( vm, 1, "orientation", _orient );
   VM::get( vm, 1, "maxSize", _maxSize );

   // Base class init.
   WidgetContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Box::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GAP:
         VM::push( vm, _gap );
         return true;
      case ATTRIB_MAX_SIZE:
         VM::push( vm, _maxSize );
         return true;
      case ATTRIB_ORIENTATION:
         VM::push( vm, _orient );
         return true;
      case ATTRIB_SCROLL_RATIO:
         VM::push( vm, scrollRatio() );
         return true;
      default: break;
   }

   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Box::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GAP:
         _gap = VM::toFloat( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_MAX_SIZE:
         _maxSize = VM::toVec2f( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_ORIENTATION:
         _orient = VM::toInt( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_SCROLL_RATIO:
         return true;
      default: break;
   }

   return WidgetContainer::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Box::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return WidgetContainer::isAttribute( name );
}

NAMESPACE_END
