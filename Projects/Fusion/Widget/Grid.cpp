/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Grid.h>
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
   ATTRIB_GAP,
   ATTRIB_ROW_MAJOR,
   ATTRIB_DIM_MAJOR
};

StringMap _attributes(
   "gap",      ATTRIB_GAP,
   "rowMajor", ATTRIB_ROW_MAJOR,
   "dimMajor", ATTRIB_DIM_MAJOR,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _grid_str_ = "grid";

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS Grid
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Grid::initialize()
{
   VMObjectPool::registerObject( "UI", _grid_str_, stdCreateVM<Grid>, stdGetVM<Grid>, stdSetVM<Grid> );
}

//------------------------------------------------------------------------------
//!
Grid::Grid()
   : WidgetContainer(),
     _gap( 0.0f, 0.0f ),
     _rowMajor( true ),
     _dimMajor( 1 ),
     _scissored( false )
{}

//------------------------------------------------------------------------------
//!
Grid::~Grid()
{}

//------------------------------------------------------------------------------
//!
Vec2f
Grid::performComputeBaseSize()
{
   const int staticAllocSize(32);

   float majorSizeStack[ staticAllocSize ];
   float minorSizeStack[ staticAllocSize ];

   float *majorSize = majorSizeStack;
   float *minorSize = minorSizeStack;
   
   if( getDimMajor() > staticAllocSize ) majorSize = new float[ getDimMajor() ];
   if( getDimMinor() > staticAllocSize ) minorSize = new float[ getDimMinor() ];

   Vec2f baseSize( computeBaseRowColSize( majorSize, minorSize ) );

   if( getDimMajor() > staticAllocSize ) delete[] majorSize;
   if( getDimMinor() > staticAllocSize ) delete[] minorSize;
   
   return baseSize;
}

//------------------------------------------------------------------------------
//!
void
Grid::render( const RCP<Gfx::RenderNode>& rn )
{
   if( _scissored )
   {
      // Render the widget container.
      Widget::render( rn );
      
      // Render the contained widgets.
      const int* sc = rn->current()->addScissor( (int)_scPos.x, (int)_scPos.y, (int)_scSize.x, (int)_scSize.y );
      Container::ConstIterator it  = _widgets.begin();
      Container::ConstIterator end = _widgets.end();
      
      for( ; it != end; ++it )
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
bool
Grid::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return WidgetContainer::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
void
Grid::performSetGeometry()
{
   const int dimMaj = getDimMajor();
   const int dimMin = getDimMinor();

   const int staticAllocSize(32);
   
   float majorSizeStack[ staticAllocSize ];
   float majorFlexStack[ staticAllocSize ];
   float minorSizeStack[ staticAllocSize ];
   float minorFlexStack[ staticAllocSize ];

   float* majorSize = majorSizeStack;
   float* majorFlex = majorFlexStack;
   float* minorSize = minorSizeStack;
   float* minorFlex = minorFlexStack;
   
   if( dimMaj > staticAllocSize )
   {
      majorSize = new float[dimMaj];
      majorFlex = new float[dimMaj];
   }

   if( dimMin > staticAllocSize )
   {
      minorSize = new float[dimMin];
      minorFlex = new float[dimMin];
   }

   // Initializes majorFlex at 0
   int i;
   int j;
   for( i=0; i < dimMaj; ++i ) majorFlex[i] = 0.0f;
   for( j=0; j < dimMin; ++j ) minorFlex[j] = 0.0f;
   
   WidgetContainer::Container::ConstIterator it  = _widgets.begin();
   WidgetContainer::Container::ConstIterator end = _widgets.end();

   int axisMajor = _rowMajor ? 0 : 1;
   int axisMinor = 1 - axisMajor;

   for( i=0, j=0; it != end; ++it )
   {
      if( !(*it)->hidden() )
      {
         // TODO: Should we have a vertical and horizontal flexibility?
         CGM::clampMin( majorFlex[i], (*it)->flexibilityHV( axisMajor ) );
         CGM::clampMin( minorFlex[j], (*it)->flexibilityHV( axisMinor ) );
      }

      i++;

      if( i%_dimMajor == 0 )
      {
         // Starting the next minor (row / column)
         i = 0;
         j++;
      }
   }

   Vec2f flex( 0.0f, 0.0f );
   
   // Calculate flex
   for( i=0; i<dimMaj; ++i ) flex( axisMajor ) += majorFlex[i];
   for( j=0; j<dimMin; ++j ) flex( axisMinor ) += minorFlex[j];

   // Get the size
   Vec2f size( computeBaseRowColSize( majorSize, minorSize ) );

   // Compute flexibility value.
   Vec2f flexSizeLeft( actualSize() - size );

   // Check if we should scissor
   _scissored = false;
   if( flexSizeLeft.x < 0.0f )
   {
      flexSizeLeft.x = 0.0f;
      _scissored   = true;
   }
   if( flexSizeLeft.y < 0.0f )
   {
      flexSizeLeft.y = 0.0f;
      _scissored   = true;
   }

   // Compute the size to use for each flex unit
   if( flex.x > 0.0f ) flex.x = flexSizeLeft.x / flex.x;
   if( flex.y > 0.0f ) flex.y = flexSizeLeft.y / flex.y;

   // Compute scissor position
   _scPos.x  =  globalPosition().x + border().x;
   _scPos.y  =  globalPosition().y + border().y;
   _scSize.x =  actualSize().x - border().x - border().z;
   _scSize.y =  actualSize().y - border().y - border().w;

   // Find last visible and flexible row/column
   int lastMajorVisibleFlexible( -1 );
   int lastMinorVisibleFlexible( -1 );
   for( i=0; i<dimMaj; ++i )
   {
      if( majorSize[i] >= 0.0f && majorFlex[i] > 0.0f ) lastMajorVisibleFlexible = i;
   }

   for( j=0; j<dimMin; ++j )
   {
      if( minorSize[j] >= 0.0f && minorFlex[j] > 0.0f ) lastMinorVisibleFlexible = j;
   }
   
   // Distribute flex
   float sizeLeft = flexSizeLeft(axisMajor);
   //float residual = 0.0f;
   for( i=0; i<dimMaj; ++i )
   {
      if( majorSize[i] >= 0.0f )
      {
         float newFlexSize;

         if( i == lastMajorVisibleFlexible )
         {
            newFlexSize = sizeLeft;
         }
         else
         {
            /*
            float newFlexSizeFloat = flex(axisMajor) * majorFlex[i];
            newFlexSize = (int)newFlexSizeFloat;
            residual += newFlexSizeFloat - newFlexSize;
            int correction = (int)residual;
            newFlexSize += correction;
            residual -= correction;
            */
            newFlexSize = flex(axisMajor) * majorFlex[i];
            CGM::clampMax( newFlexSize, sizeLeft );
         }

         majorSize[i] += newFlexSize;
         sizeLeft -= newFlexSize;
      }
   }

   sizeLeft = flexSizeLeft(axisMinor);
   //residual = 0;
   for( j=0; j<dimMin; ++j )
   {
      if( minorSize[j] >= 0 )
      {
         float newFlexSize;
         
         if( j == lastMinorVisibleFlexible )
         {
            newFlexSize = sizeLeft;
         }
         else
         {
            /*
            float newFlexSizeFloat = flex(axisMinor) * minorFlex[j];
            newFlexSize = (int)newFlexSizeFloat;
            residual += newFlexSizeFloat - newFlexSize;
            int correction = (int)residual;
            newFlexSize += correction;
            residual -= correction;
            */
            newFlexSize = flex(axisMinor) * minorFlex[j];
            CGM::clampMax( newFlexSize, sizeLeft );           
         }
         
         minorSize[j] += newFlexSize;
         sizeLeft -= newFlexSize;
      }
   }
   
   // Set children geometry.
   it  = _widgets.begin();

   Vec2f currPos( border()( (anchor()&INVERT_X)?2:0 ),
                  border()( (anchor()&INVERT_Y)?3:1 ) );
   
   Vec2f newPos;
   Vec2f newSize;

   for( i=0, j=0; it != end; ++it )
   {
      if( minorSize[j] >= 0.0f )
      {      
         // Skip the whole row/column if all its widgets are hidden
         if( majorSize[i] >= 0 )
         {
            
            // Don't show hidden widget, but compute gaps since some widgets in the row are visible
            if( !(*it)->hidden() )
            {
               Vec2f childSize = (*it)->actualBaseSize();
               
               switch( (*it)->alignHV( axisMajor ) )
               {
                  case FLEX:
                  {
                     newPos( axisMajor )  = currPos( axisMajor );
                     newSize( axisMajor ) = CGM::max( majorSize[i], childSize( axisMajor ) );
                  } break;
                  case START:
                  {
                     newPos( axisMajor )  = currPos( axisMajor );
                     newSize( axisMajor ) = childSize( axisMajor );
                  } break;
                  case MIDDLE:
                  {
                     newPos( axisMajor )  = currPos( axisMajor ) + ( majorSize[i] - childSize( axisMajor ))/2;
                     newSize( axisMajor ) = childSize( axisMajor );
                  } break;
                  case END:
                  {
                     newPos( axisMajor )  = currPos( axisMajor ) + majorSize[i] - childSize( axisMajor );
                     newSize( axisMajor ) = childSize( axisMajor );
                  } break;
               }

               switch( (*it)->alignHV( axisMinor ) )
               {
                  case FLEX:
                  {
                     newPos( axisMinor )  = currPos( axisMinor );
                     newSize( axisMinor ) = CGM::max( minorSize[j], childSize( axisMinor ) );
                  } break;
                  case START:
                  {
                     newPos( axisMinor )  = currPos( axisMinor );
                     newSize( axisMinor ) = childSize( axisMinor );
                  } break;
                  case MIDDLE:
                  {
                     newPos( axisMinor )  = currPos( axisMinor ) + ( minorSize[j] - childSize( axisMinor ))/2;
                     newSize( axisMinor ) = childSize( axisMinor );
                  } break;
                  case END:
                  {
                     newPos( axisMinor )  = currPos( axisMinor ) + minorSize[j] - childSize( axisMinor );
                     newSize( axisMinor ) = childSize( axisMinor );
                  } break;
               }
               
               (*it)->geometry( globalPosition(), newPos, newSize );
            }

            currPos( axisMajor ) += majorSize[i] + _gap( axisMajor );
         }
      }

      i++;

      if( i%_dimMajor == 0 )
      {
         // Starting the next minor (row / column)
         if( _rowMajor )
         {
            currPos.x = border()( (anchor()&INVERT_X)?2:0 );
         }
         else
         {
            currPos.y = border()( (anchor()&INVERT_X)?3:1 );
         }
 
         if( minorSize[j] >= 0.0f )
         {
            currPos( axisMinor ) += minorSize[j] + _gap( axisMinor );
         }
         
         i = 0;
         j++;
      }
   }

   
   if( dimMaj > staticAllocSize )
   {
      delete[] majorSize;
      delete[] majorFlex;
   }

   if( dimMin > staticAllocSize )
   {
      delete[] minorSize;
      delete[] minorFlex;
   }

}

//------------------------------------------------------------------------------
//!
void Grid::performSetPosition()
{
   WidgetContainer::performSetPosition();
   _scPos.x  =  globalPosition().x + border().x;
   _scPos.y  =  globalPosition().y + border().y;
}


//------------------------------------------------------------------------------
//!
//! majorSize is the width of each column when in row major
//!           or the height of each row when in column major
//!           -1 means the column/row has no visible widget
//! minorSize is the opposite
//!
//! Make sure enough memory is allocated for both pointers
Vec2f
Grid::computeBaseRowColSize( float* majorSize, float* minorSize ) const
{
   // Initializes majorSize at -1
   int i;
   int j;
   for( i=0; i < getDimMajor(); ++i ) majorSize[i] = -1.0f;
   for( j=0; j < getDimMinor(); ++j ) minorSize[j] = -1.0f;

   WidgetContainer::Container::ConstIterator it  = _widgets.begin();
   WidgetContainer::Container::ConstIterator end = _widgets.end();

   int axisMajor = _rowMajor ? 0 : 1;
   int axisMinor = 1 - axisMajor;

   for( i=0, j=0; it != end; ++it )
   {
      if( !(*it)->hidden() )
      {
         if( majorSize[i] < 0.0f ) majorSize[i] = 0.0f;
         if( minorSize[j] < 0.0f ) minorSize[j] = 0.0f;
         
         Vec2f childSize = (*it)->actualBaseSize();
         CGM::clampMin( majorSize[i], childSize( axisMajor ) );
         CGM::clampMin( minorSize[j], childSize( axisMinor ) );
      }

      i++;

      if( i%_dimMajor == 0 )
      {
         // Starting the next minor (row / column)
         i = 0;
         j++;
      }
   }

   Vec2f newBaseSize( 0.0f, 0.0f );

   int nbMinorVisible = 0;
   int nbMajorVisible = 0;
   
   // Calculate newBaseSize
   for( i=0; i< getDimMajor(); ++i )
   {
      if( majorSize[i] >= 0.0f )
      {
         newBaseSize( axisMajor ) += majorSize[i];
         nbMajorVisible++;
      }
   }

   for( j=0; j < getDimMinor(); ++j )
   {
      if( minorSize[j] >= 0.0f )
      {
         newBaseSize( axisMinor ) += minorSize[j];
         nbMinorVisible++;
      }
   }
   
   // Add gaps.
   if( nbMinorVisible > 0  )
   {
      CHECK( nbMajorVisible > 0 );
      newBaseSize( axisMinor ) += ( nbMinorVisible - 1 ) * _gap( axisMinor );
      newBaseSize( axisMajor ) += ( nbMajorVisible - 1 ) * _gap( axisMajor );
   }
   
   // Add border.
   newBaseSize.x += border().x + border().z;
   newBaseSize.y += border().y + border().w;

   return newBaseSize;
}

//------------------------------------------------------------------------------
//!
const char*
Grid::meta() const
{
   return _grid_str_;
}

//------------------------------------------------------------------------------
//!
void
Grid::init( VMState* vm )
{
   VM::get( vm, 1, "gap", _gap );
   VM::get( vm, 1, "rowMajor", _rowMajor );
   VM::get( vm, 1, "dimMajor", _dimMajor );
   
   // Base class init.
   WidgetContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Grid::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GAP:
         VM::push( vm, _gap );
         return true;
      case ATTRIB_ROW_MAJOR:
         VM::push( vm, _rowMajor );
         return true;
      case ATTRIB_DIM_MAJOR:
         VM::push( vm, _dimMajor );
         return true;
         
      default: break;
   }

   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Grid::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GAP:
         _gap = VM::toVec2f( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_ROW_MAJOR:
         _rowMajor = VM::toBoolean( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_DIM_MAJOR:
         _dimMajor = VM::toInt( vm, 3 );
         markForUpdate();
         return true;
         
      default: break;
   }

   return WidgetContainer::performSet( vm );
}

NAMESPACE_END
