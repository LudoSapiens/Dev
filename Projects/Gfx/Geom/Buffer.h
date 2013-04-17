/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_BUFFER_H
#define GFX_BUFFER_H

#include <Gfx/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>

#include <cstddef>

NAMESPACE_BEGIN

namespace Gfx
{


/*----- types -----*/

typedef enum
{
   ATTRIB_FMT_INVALID,
   ATTRIB_FMT_8,
   ATTRIB_FMT_8_8,
   ATTRIB_FMT_8_8_8,     //Probably not supported
   ATTRIB_FMT_8_8_8_8,
   ATTRIB_FMT_16,
   ATTRIB_FMT_16_16,
   ATTRIB_FMT_16_16_16,  //Probably not supported
   ATTRIB_FMT_16_16_16_16,
   ATTRIB_FMT_16F,
   ATTRIB_FMT_16F_16F,
   ATTRIB_FMT_16F_16F_16F,  //Probably not supported
   ATTRIB_FMT_16F_16F_16F_16F,
   ATTRIB_FMT_32,
   ATTRIB_FMT_32_32,
   ATTRIB_FMT_32_32_32,
   ATTRIB_FMT_32_32_32_32,
   ATTRIB_FMT_32F,
   ATTRIB_FMT_32F_32F,
   ATTRIB_FMT_32F_32F_32F,
   ATTRIB_FMT_32F_32F_32F_32F
} AttributeFormat;

typedef enum
{
   ATTRIB_TYPE_UNSPECIFIED = 0x0000,
   ATTRIB_TYPE_POSITION    = 0x0001,
   ATTRIB_TYPE_COLOR       = 0x0002,
   ATTRIB_TYPE_TEXCOORD0   = 0x0004,
   ATTRIB_TYPE_TEXCOORD1   = 0x0008,
   ATTRIB_TYPE_TEXCOORD2   = 0x0010,
   ATTRIB_TYPE_TEXCOORD3   = 0x0020,
   ATTRIB_TYPE_TEXCOORD4   = 0x0040,
   ATTRIB_TYPE_TEXCOORD5   = 0x0080,
   ATTRIB_TYPE_TEXCOORD6   = 0x0100,
   ATTRIB_TYPE_TEXCOORD7   = 0x0200,
   ATTRIB_TYPE_NORMAL      = 0x0400,
   ATTRIB_TYPE_TANGENT     = 0x0800,
   ATTRIB_TYPE_BINORMAL    = 0x1000,
   ATTRIB_TYPE_ALL         = 0x1FFF,
} AttributeType;

typedef enum
{
   INDEX_FMT_INVALID,
   INDEX_FMT_8,
   INDEX_FMT_16,
   INDEX_FMT_32
} IndexFormat;

typedef enum
{
   BUFFER_FLAGS_NONE       = 0x00,
   BUFFER_FLAGS_STREAMABLE = 0x01
} BufferFlags;

inline BufferFlags  operator| ( BufferFlags f1, BufferFlags f2 )
{ return (BufferFlags)((int)f1 | (int)f2); }

inline bool  isStreamable( const BufferFlags fl )
{ return (fl & BUFFER_FLAGS_STREAMABLE) != 0; }


/*==============================================================================
   Utilities
==============================================================================*/

//------------------------------------------------------------------------------
//!
inline
uint
toNumChannels
( const AttributeFormat fmt )
{
   switch( fmt )
   {
      case ATTRIB_FMT_INVALID        : return  0;
      case ATTRIB_FMT_8              : return  1;
      case ATTRIB_FMT_8_8            : return  2;
      case ATTRIB_FMT_8_8_8          : return  3;
      case ATTRIB_FMT_8_8_8_8        : return  4;
      case ATTRIB_FMT_16             : return  1;
      case ATTRIB_FMT_16_16          : return  2;
      case ATTRIB_FMT_16_16_16       : return  3;
      case ATTRIB_FMT_16_16_16_16    : return  4;
      case ATTRIB_FMT_16F            : return  1;
      case ATTRIB_FMT_16F_16F        : return  2;
      case ATTRIB_FMT_16F_16F_16F    : return  3;
      case ATTRIB_FMT_16F_16F_16F_16F: return  4;
      case ATTRIB_FMT_32             : return  1;
      case ATTRIB_FMT_32_32          : return  2;
      case ATTRIB_FMT_32_32_32       : return  3;
      case ATTRIB_FMT_32_32_32_32    : return  4;
      case ATTRIB_FMT_32F            : return  1;
      case ATTRIB_FMT_32F_32F        : return  2;
      case ATTRIB_FMT_32F_32F_32F    : return  3;
      case ATTRIB_FMT_32F_32F_32F_32F: return  4;
      default                        : return  0;
   }
}

//------------------------------------------------------------------------------
//!
inline
uint
toBytes
( const AttributeFormat fmt )
{
   switch( fmt )
   {
      case ATTRIB_FMT_INVALID        : return  0;
      case ATTRIB_FMT_8              : return  1;
      case ATTRIB_FMT_8_8            : return  2;
      case ATTRIB_FMT_8_8_8          : return  3;
      case ATTRIB_FMT_8_8_8_8        : return  4;
      case ATTRIB_FMT_16             : return  2;
      case ATTRIB_FMT_16_16          : return  4;
      case ATTRIB_FMT_16_16_16       : return  6;
      case ATTRIB_FMT_16_16_16_16    : return  8;
      case ATTRIB_FMT_16F            : return  2;
      case ATTRIB_FMT_16F_16F        : return  4;
      case ATTRIB_FMT_16F_16F_16F    : return  6;
      case ATTRIB_FMT_16F_16F_16F_16F: return  8;
      case ATTRIB_FMT_32             : return  4;
      case ATTRIB_FMT_32_32          : return  8;
      case ATTRIB_FMT_32_32_32       : return 12;
      case ATTRIB_FMT_32_32_32_32    : return 16;
      case ATTRIB_FMT_32F            : return  4;
      case ATTRIB_FMT_32F_32F        : return  8;
      case ATTRIB_FMT_32F_32F_32F    : return 12;
      case ATTRIB_FMT_32F_32F_32F_32F: return 16;
      default                        : return  0;
   }
}

//------------------------------------------------------------------------------
//!
inline
uint
toBytes
( const IndexFormat fmt )
{
   switch( fmt )
   {
      case INDEX_FMT_INVALID: return  0;
      case INDEX_FMT_8      : return  1;
      case INDEX_FMT_16     : return  2;
      case INDEX_FMT_32     : return  4;
      default               : return  0;
   }
}


/*==============================================================================
  CLASS Buffer
==============================================================================*/

//------------------------------------------------------------------------------
//!
class Buffer:
   public RCObject
{
public:

   inline const BufferFlags&  flags() const { return _flags; }
   inline bool  isStreamable() const { return Gfx::isStreamable(_flags); }

   inline size_t  sizeInBytes() const { return _sizeInBytes; }

protected:

   /*----- methods -----*/

   // Only Managers can create this object.
   Buffer( const BufferFlags flags = BUFFER_FLAGS_NONE, size_t sizeInBytes = 0 );
   virtual ~Buffer();

   /*----- data members -----*/

   BufferFlags  _flags;        //!< Some flags giving further information
   size_t       _sizeInBytes;  //!< The total number of bytes that the buffer uses

private:
   GFX_MAKE_MANAGERS_FRIENDS();

};

/*==============================================================================
  CLASS IndexBuffer
==============================================================================*/

//------------------------------------------------------------------------------
//!
class IndexBuffer:
   public Buffer
{

public:

   /*----- methods -----*/

   inline IndexFormat  format() const { return _format; }

   inline size_t  numIndices() const { return _sizeInBytes / toBytes(_format); }

protected:

   /*----- methods -----*/

   // Only Managers can create this object.
   IndexBuffer( const IndexFormat format, const BufferFlags flags );
   virtual ~IndexBuffer();

   /*----- data members -----*/

   IndexFormat  _format;  //!< The format of every index in the buffer

private:
   GFX_MAKE_MANAGERS_FRIENDS();

};

/*==============================================================================
  CLASS VertexBuffer
==============================================================================*/

//------------------------------------------------------------------------------
//!
class VertexBuffer:
   public Buffer
{

public:

   /*----- classes -----*/

   class Attribute:
      public RCObject
   {
   public:
      AttributeType    _type;    //!< The type of information this attribute conveys
      AttributeFormat  _format;  //!< The format of the attribute (specifies both number of channels as well as size in bytes)
      size_t           _offset;  //!< The offset in bytes at which this attribute is found
      Attribute(
         AttributeType type     = ATTRIB_TYPE_UNSPECIFIED,
         AttributeFormat format = ATTRIB_FMT_INVALID,
         size_t offset          = 0
      ):
         _type(type), _format(format), _offset(offset) { }
      // Returns the minimum stride (in bytes) required by this attribute.
      inline size_t  requiredStride() const { return _offset + toBytes(_format); }
   };

   /*----- types and enumerations ----*/
   typedef Vector< RCP<Attribute> >  AttributesContainer;

   /*----- static data members -----*/

   static const RCP<const Attribute> kNullAttrib;  //to avoid warning about return temp reference for NULL

   /*----- methods -----*/

   size_t  strideInBytes() const { return _strideInBytes; }

   size_t  numAttributes() const { return _attributes.size(); }

   inline const RCP<const Attribute>&  getAttribute( size_t index ) const;
   GFX_DLL_API const RCP<const Attribute>&  getAttribute( AttributeType type ) const;

   // Adds an attribute, potentially creating a duplicate (which the user must prevent).
   GFX_DLL_API void  addAttribute( const RCP<Attribute>& attrib );
   GFX_DLL_API void  addAttribute(
      AttributeType type,
      AttributeFormat format,
      size_t offset
   );

   // Changes a previously-set attribute (or adds it if it is missing).
   GFX_DLL_API void  setAttribute( const RCP<Attribute>& attrib );
               void  setAttribute( const size_t index, const RCP<Attribute>& attrib )
   { _attributes[index] = attrib; ++_revision; }

   // Removes a previously set attribute.
   GFX_DLL_API bool  removeAttribute( AttributeType type );

   // Returns the number of vertices present in the current buffer.
   size_t  numVertices() const { return _sizeInBytes / _strideInBytes; }

   inline uint  revision() const { return _revision; }

protected:

   /*----- methods -----*/
   //Computes the required stride by analysing every attribute
   size_t  computeStride() const;
   void  growStride( const size_t s ) { if( _strideInBytes < s ) _strideInBytes = s; }


   // Only Managers can create this object.
   VertexBuffer( const BufferFlags flags );
   virtual ~VertexBuffer();

   /*----- data members -----*/

   uint                 _revision;
   size_t               _strideInBytes;  //!< The stride of every element, in bytes
   AttributesContainer  _attributes;  //!< All of the attributes specified

private:
   GFX_MAKE_MANAGERS_FRIENDS();

};

//------------------------------------------------------------------------------
//!
inline const RCP<const VertexBuffer::Attribute>&
VertexBuffer::getAttribute( size_t index ) const
{
   return reinterpret_cast< const RCP<const Attribute>& >(_attributes[index]);
}



}  //namespace Gfx

NAMESPACE_END


#endif //GFX_BUFFER_H
