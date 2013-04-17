/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Geom/Buffer.h>

#include <algorithm>

USING_NAMESPACE

using namespace Gfx;


UNNAMESPACE_BEGIN

/*==============================================================================
   CLASS AttributeTypesEqual
==============================================================================*/

//! A predicate class for STL's remove_if call
class AttributeTypesEqual
{
public:
   const AttributeType _type;
   AttributeTypesEqual( const AttributeType& type ): _type(type) { }

   bool operator()( VertexBuffer::AttributesContainer::const_reference attrib ) { return attrib->_type == _type; }
};

UNNAMESPACE_END

/*==============================================================================
   CLASS VertexBuffer
==============================================================================*/

//------------------------------------------------------------------------------
//!
Buffer::Buffer( const BufferFlags flags, const size_t sizeInBytes ):
   _flags(flags), _sizeInBytes(sizeInBytes)
{
}

//------------------------------------------------------------------------------
//!
Buffer::~Buffer
( void )
{
}



/*==============================================================================
   CLASS IndexBuffer
==============================================================================*/

//------------------------------------------------------------------------------
//!
IndexBuffer::IndexBuffer( const IndexFormat format, const BufferFlags flags ):
   Buffer(flags),
   _format(format)
{
}

//------------------------------------------------------------------------------
//!
IndexBuffer::~IndexBuffer
( void )
{
}


/*==============================================================================
   CLASS VertexBuffer
==============================================================================*/

//Static
const RCP<const VertexBuffer::Attribute>  VertexBuffer::kNullAttrib = NULL;

//------------------------------------------------------------------------------
//!
VertexBuffer::VertexBuffer( const BufferFlags flags ):
   Buffer(flags),
   _revision(1),
   _strideInBytes(0)
{
}

//------------------------------------------------------------------------------
//!
VertexBuffer::~VertexBuffer( void )
{

}

//------------------------------------------------------------------------------
//!
const RCP<const VertexBuffer::Attribute>&
VertexBuffer::getAttribute( AttributeType type )
const
{
   //Iterate over all of the attributes to find one with the specified type
   AttributesContainer::const_iterator curAttrib = _attributes.begin();
   AttributesContainer::const_iterator endAttrib = _attributes.end();
   for( ; curAttrib != endAttrib; ++curAttrib )
   {
      if( (*curAttrib)->_type == type )
      {
         return reinterpret_cast< const RCP<const Attribute>& >(*curAttrib);
      }
   }

   //Could not find it
   return kNullAttrib;
}

//------------------------------------------------------------------------------
//!
void
VertexBuffer::addAttribute( const RCP<Attribute>& attrib )
{
   _attributes.pushBack(attrib);
   growStride(attrib->requiredStride());
   ++_revision;
}

//------------------------------------------------------------------------------
//!
void
VertexBuffer::addAttribute(
   AttributeType   type,
   AttributeFormat format,
   size_t          offset
)
{
   addAttribute( RCP<Attribute>( new Attribute( type, format, offset) ) );
}

//------------------------------------------------------------------------------
//!
void
VertexBuffer::setAttribute( const RCP<Attribute>& attrib )
{
   //Iterate over all of the attributes to find one with the specified type
   AttributesContainer::iterator curAttrib = _attributes.begin();
   AttributesContainer::iterator endAttrib = _attributes.end();
   for( ; curAttrib != endAttrib; ++curAttrib )
   {
      if( (*curAttrib)->_type == attrib->_type )
      {
         size_t oldReqStride = (*curAttrib)->requiredStride();
         size_t newReqStride = attrib->requiredStride();
         (*curAttrib) = attrib;
         ++_revision;
         if( newReqStride < _strideInBytes )
         {
            if(oldReqStride == _strideInBytes)
            {
               //Means the attribute used to be the defining factor
               _strideInBytes = computeStride();
            }
            //else, someone else is the defining factor
         }
         else
         {
            _strideInBytes = newReqStride;
         }
         return;
      }
   }

   //Could not find it, just add it
   addAttribute(attrib);
}

//------------------------------------------------------------------------------
//!
bool
VertexBuffer::removeAttribute( AttributeType type )
{
   AttributeTypesEqual types_equal(type);
   AttributesContainer::Iterator newEnd = std::remove_if(
      _attributes.begin(), _attributes.end(), types_equal
   );

   bool removed = false;
   if( newEnd != _attributes.end() )
   {
      _attributes.erase( newEnd, _attributes.end() );
      ++_revision;
      removed = true;
      _strideInBytes = computeStride();
   }
   return removed;
}

//------------------------------------------------------------------------------
//!
size_t
VertexBuffer::computeStride() const
{
   size_t largest = 0;
   size_t current;

   AttributesContainer::const_iterator curAttrib = _attributes.begin();
   AttributesContainer::const_iterator endAttrib = _attributes.end();
   for( ; curAttrib != endAttrib; ++curAttrib )
   {
      current = (*curAttrib)->requiredStride();
      if( largest < current )  largest = current;
   }

   return largest;
}
