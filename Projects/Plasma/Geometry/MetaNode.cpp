/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/MetaNode.h>
#include <Plasma/Geometry/MetaGeometry.h>

#include <Fusion/VM/VM.h>

#include <Base/ADT/MemoryPool.h>

#include <algorithm>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


UNNAMESPACE_END

/*==============================================================================
   CLASS MetaNode
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
MetaNode::print( TextStream& os ) const
{
   print( os, "" );
}

//------------------------------------------------------------------------------
//! 
void 
MetaNode::print( TextStream& /*stream*/, const String& /*tab*/ ) const
{
}

/*==============================================================================
   CLASS MetaBlocks
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void 
MetaBlocks::resetCount( uint countID )
{ 
   _countID  = countID; 
   _count    = 0;
   _backB    = false;
   _frontB   = false;
}

//------------------------------------------------------------------------------
//! 
void
MetaBlocks::addIntersection( float t, bool backFacing )
{
   if( backFacing )
   {
      if( t < 0.0001f )
      {
         if( !_backB )
         {
            _backB = true;
            --count();
         }
      }
      else
         --count();
   }
   else
   {
      if( t < 0.0001f )
      {
         if( !_frontB )
         {
            _frontB = true;
            ++count();
         }
      }
      else
         ++count();
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaBlocks::dump() const
{
   StdErr << "count: " << _count << "\n";
   StdErr << "back: " << _backB  << "\n";
   StdErr << "front: "<< _frontB << "\n";
}

//------------------------------------------------------------------------------
//! 
void 
MetaBlocks::print( TextStream& stream, const String& tab ) const
{
   stream << tab << "Blocks: " << this << "\n";
   stream << tab << "parent: " << _parent << "\n";
   stream << tab << "BBox: " << _box << "\n";
   stream << tab << _blocks.size() << " blocks\n";
   stream << tab << "Blocks end\n";
}

/*==============================================================================
   CLASS MetaCompositeBlocks
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
MetaCompositeBlocks::update()
{
   _blocks.clear();
   _attractions.clear();
   for( uint i = 0; i < _children.size(); ++i )
   {
      MetaBlocks* mb = _children[i];
      for( uint b = 0; b < mb->numBlocks(); ++b )
      {
         _blocks.pushBack( mb->block(b) );
         mb->block(b)->_group = this;
      }
      AttractionContainer::ConstIterator it;
      for( it = mb->attractions().begin(); it != mb->attractions().end(); ++it )
      {
         _attractions.add( *it );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaCompositeBlocks::print( TextStream& stream, const String& tab ) const
{
   stream << tab << "CompositeBlocks: " << this << "\n";
   stream << tab << "parent: " << _parent << "\n";
   stream << tab << "BBox: " << _box << "\n";
   stream << tab << _blocks.size() << " blocks\n";
   stream << tab << "CompositeBlocks end\n";
}

/*==============================================================================
   CLASS MetaInput
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void 
MetaInput::print( TextStream& stream, const String& tab ) const
{
   stream << tab << "Input: " << this << "\n";
   stream << tab << "parent: " << _parent << "\n";
   if( _child ) _child->print( stream, tab + "  " );
   stream << tab << "Input end\n";
}

/*==============================================================================
   CLASS MetaTransform
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
MetaTransform::print( TextStream& stream, const String& tab ) const
{
   stream << tab << "Transform: " << this << "\n";
   stream << tab << "parent: " << _parent << "\n";
   stream << tab << "ref: " << _transform << "\n";
   if( _child ) _child->print( stream, tab + "  " );
   stream << tab << "Transform end\n";
}

/*==============================================================================
   CLASS MetaOperation
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
MetaOperation::print( TextStream& stream, const String& tab ) const
{
   String str;
   switch( type() )
   {
      case META_UNION:        str = "Union";        break;
      case META_DIFFERENCE:   str = "Difference";   break;
      case META_INTERSECTION: str = "Intersection"; break;
      default:;
   }

   stream << tab << str << ": " << this << "\n";
   stream << tab << "parent: " << _parent << "\n";
   stream << tab << "BBox: " << _box << "\n";
   for( uint i = 0; i < _children.size(); ++i )
   {
      _children[i]->print( stream, tab + "  " );
   }
   stream << tab << str << " end\n";
}

/*==============================================================================
   CLASS MetaComposite
==============================================================================*/

//------------------------------------------------------------------------------
//! 
inline bool lt_comp( MetaNode* a, MetaNode* b )
{
   return a->layer() < b->layer();
}

//------------------------------------------------------------------------------
//! 
void
MetaComposite::connectComposites()
{
   _interiorNodes.clear();

   if( _children.empty() )
   {
      _mainChild = 0;
      return;
   }

   // First sorts children by their layers.
   std::stable_sort( _children.begin(), _children.end(), lt_comp );

   // Connect children node together.
   MetaUnion* unionNode = 0;
   MetaNode* lastNode   = child(0);
   for( uint i = 1; i < _children.size(); ++i )
   {
      if( child(i)->type() == META_COMPOSITE )
      {
         MetaComposite* comp = (MetaComposite*)child(i);
         if( comp->input() )
         {
            comp->input()->child( lastNode );
            lastNode  = comp;
            unionNode = 0;
            continue;
         }
      }
      if( !unionNode )
      {
         unionNode = new MetaUnion();
         unionNode->parent( this );
         unionNode->add( lastNode );
         lastNode  = unionNode;
         _interiorNodes.pushBack( unionNode );
      }
      unionNode->add( child(i) );
   }
   _mainChild = lastNode;
}

//------------------------------------------------------------------------------
//! 
void
MetaComposite::print( TextStream& stream, const String& tab ) const
{
   stream << tab << "Composite: " << this << "\n";
   stream << tab << "parent: " << _parent << "\n";
   stream << tab << "BBox: " << _box << "\n";
   stream << tab << "ref: " << _transform << "\n";
   if( _mainChild ) _mainChild->print( stream, tab + "  " );
   stream << tab << "Composite end\n";
}
