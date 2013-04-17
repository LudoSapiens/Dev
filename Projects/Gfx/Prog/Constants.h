/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_CONSTANTS_H
#define GFX_CONSTANTS_H

#include <Gfx/StdDefs.h>
#include <Gfx/Prog/Program.h>

#include <Base/Util/RCObject.h>
#include <Base/ADT/ConstString.h>
#include <Base/Dbg/Defs.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

namespace Gfx
{
   
/*----- types -----*/
   
enum ConstantType
{
   CONST_FLOAT,
   CONST_FLOAT2,
   CONST_FLOAT3,
   CONST_FLOAT4,
   CONST_MAT2,
   CONST_MAT3,
   CONST_MAT4
};

//------------------------------------------------------------------------------
//! Returns the size of every constant type enumerated.
inline size_t  toBytes( const ConstantType type )
{
   switch( type )
   {
      case CONST_FLOAT : return   1*sizeof(float);
      case CONST_FLOAT2: return   2*sizeof(float);
      case CONST_FLOAT3: return   3*sizeof(float);
      case CONST_FLOAT4: return   4*sizeof(float);
      case CONST_MAT2  : return 2*2*sizeof(float);
      case CONST_MAT3  : return 3*3*sizeof(float);
      case CONST_MAT4  : return 4*4*sizeof(float);
      default: CHECK(false); return 0;
   }
}

/*==============================================================================
   CLASS ConstantBuffer
==============================================================================*/
class ConstantBuffer:
   public RCObject
{

public:

   /*----- classes -----*/

   /*==============================================================================
     CLASS Constant
   ==============================================================================*/
   class Constant
   {
   public:

      /*----- members -----*/

      Constant( 
         const ConstString& name, 
         ConstantType       type, 
         size_t             offset, 
         uint               count = 0, 
         ShaderType         shaderType = FRAGMENT_SHADER 
      ):
         _name( name ), _type( type ), _offset( offset ), 
         _count( count ), _shaderType( shaderType )
      {
      }

      const ConstString&  name() const { return _name; }
      ConstantType  type() const       { return _type; }
      size_t  offset() const           { return _offset; }
      uint  count() const              { return _count; }
      ShaderType  shaderType() const   { return _shaderType; }

   protected:

      /*----- data members -----*/

      ConstString  _name;
      ConstantType _type;
      size_t       _offset;
      uint         _count;
      ShaderType   _shaderType;
   }; //Constant

   /*----- types and enumerations ----*/
   
   typedef Vector< Constant > Container;

   /*----- methods -----*/

   GFX_DLL_API uint addConstant( 
      const ConstString& name, 
      ConstantType       type, 
      size_t             offset, 
      ShaderType         shaderType = FRAGMENT_SHADER 
   );
   GFX_DLL_API uint addConstant(
      const ConstString& name, 
      ConstantType       type, 
      size_t             offset,
      ShaderType         shaderType, 
      const void*        value
   );
   inline uint addConstant( 
      const ConstString& name, 
      ConstantType       type, 
      size_t             offset,
      const void*        value
   )
   { 
      return addConstant( name, type, offset, FRAGMENT_SHADER, value );
   }

   GFX_DLL_API void addConstantArray( 
      const ConstString& name, 
      ConstantType       type, 
      uint               count, 
      size_t             offset,
      ShaderType         shaderType = FRAGMENT_SHADER
   );
   GFX_DLL_API void addConstantArray(
      const ConstString& name, 
      ConstantType       type,
      uint               count,
      size_t             offset,
      ShaderType         shaderType,
      const void*        value
   );
   inline void addConstantArray(
      const ConstString& name,
      ConstantType       type, 
      uint               count,
      size_t             offset,
      const void*        value
   )
   { 
      addConstantArray( name, type, count, offset, FRAGMENT_SHADER, value );
   }

   GFX_DLL_API void setConstant( const ConstString& name, const void* value );
   GFX_DLL_API void setConstant( const Constant* constant, const void* value );
   GFX_DLL_API void setConstant( int constantId, const void* value );

   GFX_DLL_API void getConstant( const ConstString& name, void* value );
   GFX_DLL_API void getConstant( const Constant* constant, void* value );

   GFX_DLL_API const Constant* getConstant( const ConstString& name ) const;

   const Container& constants() const { return _constants; }

   GFX_DLL_API void  print() const;

   /*----- static utility routines -----*/

   static GFX_DLL_API size_t  getSize( const Container& constants );

protected: 

   /*----- methods -----*/

   ConstantBuffer( size_t size );
   ConstantBuffer( const Container& constants, size_t size = 0 );
   virtual ~ConstantBuffer();


   /*----- data members -----*/

   uchar*    _buffer;
   Container _constants;

private: 
   
   GFX_MAKE_MANAGERS_FRIENDS();

};


/*==============================================================================
  CLASS ConstantList
==============================================================================*/
class ConstantList:
   public RCObject
{
public:

   /*----- types -----*/

   typedef Vector< RCP<ConstantBuffer> >  ContainerType;


   /*----- static methods -----*/

   static GFX_DLL_API RCP<ConstantList>  create( const RCP<ConstantBuffer>& constantBuffer );

   /*----- methods -----*/

   GFX_DLL_API ConstantList();
   GFX_DLL_API virtual ~ConstantList();

   void reserve( uint i )                                 { _buffers.reserve(i); }
   void addBuffer( const RCP<ConstantBuffer>& buffer )    { _buffers.pushBack( buffer ); }
   void removeBuffer( const RCP<ConstantBuffer>& buffer ) { _buffers.remove( buffer ); }

         ContainerType&  buffers()       { return _buffers; }
   const ContainerType&  buffers() const { return _buffers; }

   inline uint size() const                                     { return (uint)_buffers.size(); }
   inline const RCP<ConstantBuffer>& operator[]( uint i ) const { return _buffers[i]; }

   inline void clear() { _buffers.clear(); }

   inline const ConstantBuffer::Constant* getConstant( const ConstString& name, ConstantBuffer*& buffer ) const;


protected:

   /*----- data members -----*/

   ContainerType  _buffers;

private:

   GFX_MAKE_MANAGERS_FRIENDS();

}; //class ConstantList

//------------------------------------------------------------------------------
//!
inline const ConstantBuffer::Constant*
ConstantList::getConstant( const ConstString& name, ConstantBuffer*& buffer ) const
{
   for( ContainerType::ConstIterator cur = _buffers.begin(); cur != _buffers.end(); ++cur )
   {
      const ConstantBuffer::Constant* c = (*cur)->getConstant( name );
      if( c != NULL )
      {
         buffer = (*cur).ptr();
         return c;
      }
   }
   return NULL;
}

}  //namespace Gfx

NAMESPACE_END


#endif //GFX_CONSTANTS_H
