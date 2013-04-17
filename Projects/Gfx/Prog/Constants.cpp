/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Prog/Constants.h>

#include <Base/IO/TextStream.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

typedef struct { float _[2]; } Float2;
typedef struct { float _[3]; } Float3;
typedef struct { float _[4]; } Float4;
typedef struct { float _[4]; } Mat2;
typedef struct { float _[9]; } Mat3;
typedef struct { float _[16]; } Mat4;

UNNAMESPACE_END


/*==============================================================================
   CLASS ConstantBuffer
==============================================================================*/

//------------------------------------------------------------------------------
//!
ConstantBuffer::ConstantBuffer( size_t size )
{
   if( size != 0 )
   {
      _buffer = new uchar[size];
   }
   else
   {
      _buffer = NULL;
   }
}

//------------------------------------------------------------------------------
//!
ConstantBuffer::ConstantBuffer( const Container& constants, size_t size )
{
   if( size == 0 ) size = getSize(constants);

   if( size != 0 )
   {
      _buffer = new uchar[size];
   }
   else
   {
      _buffer = NULL;
   }

   _constants = constants;
}

//------------------------------------------------------------------------------
//!
ConstantBuffer::~ConstantBuffer()
{
   delete[] _buffer;
}

//------------------------------------------------------------------------------
//!
uint
ConstantBuffer::addConstant(
   const ConstString& name, 
   ConstantType       type, 
   size_t             offset,
   ShaderType         shaderType
)
{
   _constants.pushBack( Constant( name, type, offset, 0, shaderType ) );
   return uint(_constants.size()-1);
}

//------------------------------------------------------------------------------
//!
uint
ConstantBuffer::addConstant(
   const ConstString& name, 
   ConstantType       type, 
   size_t             offset,
   ShaderType         shaderType, 
   const void*        value
)
{
   _constants.pushBack( Constant( name, type, offset, 0, shaderType ) );
   setConstant( &(_constants.back()), value );
   return uint(_constants.size()-1);
}

//------------------------------------------------------------------------------
//!
void
ConstantBuffer::addConstantArray(
   const ConstString& name, 
   ConstantType       type, 
   uint               count,
   size_t             offset,
   ShaderType         shaderType
)
{
   _constants.pushBack( Constant( name, type, offset, count, shaderType ) );
}

//------------------------------------------------------------------------------
//!
void
ConstantBuffer::addConstantArray(
   const ConstString& name, 
   ConstantType       type, 
   uint               count,
   size_t             offset,
   ShaderType         shaderType,
   const void*        value
)
{
   _constants.pushBack( Constant( name, type, offset, count, shaderType ) );
   setConstant( &(_constants.back()), value );
}

//------------------------------------------------------------------------------
//!
void
ConstantBuffer::setConstant( const ConstString& name, const void* value )
{
   const Constant* constant = getConstant( name );
   if( constant != NULL )
   {
      setConstant( constant, value );
   }
   else
   {
      StdErr << "ERROR - Constant '" << name.cstr() << "' not found." << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
ConstantBuffer::setConstant( const Constant* constant, const void* value )
{
   if( constant->count() == 0 )
   {
      switch( constant->type() )
      {
         case CONST_FLOAT : *(float*)(_buffer + constant->offset())  = *(float*)value;  break;
         case CONST_FLOAT2: *(Float2*)(_buffer + constant->offset()) = *(Float2*)value; break;
         case CONST_FLOAT3: *(Float3*)(_buffer + constant->offset()) = *(Float3*)value; break;
         case CONST_FLOAT4: *(Float4*)(_buffer + constant->offset()) = *(Float4*)value; break;
         case CONST_MAT2  : *(Mat2*)(_buffer + constant->offset())   = *(Mat2*)value;   break;
         case CONST_MAT3  : *(Mat3*)(_buffer + constant->offset())   = *(Mat3*)value;   break;
         case CONST_MAT4  : *(Mat4*)(_buffer + constant->offset())   = *(Mat4*)value;   break;
      }
   }
   else
   {
      *(const void**)(_buffer + constant->offset()) = value;
   }
}

//------------------------------------------------------------------------------
//! 
void 
ConstantBuffer::setConstant( int constantId, const void* value )
{
   setConstant( &_constants[constantId], value );
}

//------------------------------------------------------------------------------
//!
void
ConstantBuffer::getConstant( const ConstString& name, void* value )
{
   const Constant* constant = getConstant( name );
   CHECK(constant != NULL);
   getConstant( constant, value );
}

//------------------------------------------------------------------------------
//!
void
ConstantBuffer::getConstant( const Constant* constant, void* value )
{
   if( constant->count() == 0 )
   {
      switch( constant->type() )
      {
         case CONST_FLOAT : *(float* )value = *(float* )(_buffer + constant->offset());  break;
         case CONST_FLOAT2: *(Float2*)value = *(Float2*)(_buffer + constant->offset());  break;
         case CONST_FLOAT3: *(Float3*)value = *(Float3*)(_buffer + constant->offset());  break;
         case CONST_FLOAT4: *(Float4*)value = *(Float4*)(_buffer + constant->offset());  break;
         case CONST_MAT2  : *(Mat2*  )value = *(Mat2*  )(_buffer + constant->offset());  break;
         case CONST_MAT3  : *(Mat3*  )value = *(Mat3*  )(_buffer + constant->offset());  break;
         case CONST_MAT4  : *(Mat4*  )value = *(Mat4*  )(_buffer + constant->offset());  break;
      }
   }
   else
   {
      *(const void**)value = (_buffer + constant->offset());
   }
}

//------------------------------------------------------------------------------
//!
const ConstantBuffer::Constant*
ConstantBuffer::getConstant( const ConstString& name ) const
{
   Container::ConstIterator it  = _constants.begin();
   Container::ConstIterator end = _constants.end();
   for( ; it != end; ++it )
   {
      // OpenGL reports 'myVar[0]' for arrays, so only compare 'myVar' then
      // guarantee it is either the full name, or the next char is '['.
      if( name == (*it).name() )
      {
         return &(*it);
      }
      else
      {
         uint s = (*it).name().size();
         if( (*it).name()[s-1] == ']' && (strncmp( name.cstr(), (*it).name().cstr(), s-3 ) == 0) )
         {
            return &(*it);
         }
      }
   }
   return NULL;
}


//------------------------------------------------------------------------------
//!
void
ConstantBuffer::print() const
{
   printf("Constant buffer contains %d constants\n", (int)_constants.size());
   Container::ConstIterator cur = _constants.begin();
   Container::ConstIterator end = _constants.end();
   for( ; cur != end; ++cur )
   {
      switch( (*cur).type() )
      {
         case CONST_FLOAT:
         {
            float* v = (float*)(_buffer + (*cur).offset());
            printf("%s: type=float offset=%d data=%f\n",
                   (*cur).name().cstr(), (int)(*cur).offset(), v[0]);
         } break;
         case CONST_FLOAT2:
         {
            float* v = (float*)(_buffer + (*cur).offset());
            printf("%s: type=float offset=%d data=%f,%f\n",
                   (*cur).name().cstr(), (int)(*cur).offset(), v[0], v[1]);
         } break;
         case CONST_FLOAT3:
         {
            float* v = (float*)(_buffer + (*cur).offset());
            printf("%s: type=float offset=%d data=%f,%f,%f\n",
                   (*cur).name().cstr(), (int)(*cur).offset(), v[0], v[1], v[2]);
         } break;
         case CONST_FLOAT4:
         {
            float* v = (float*)(_buffer + (*cur).offset());
            printf("%s: type=float offset=%d data=%f,%f,%f,%f\n",
                   (*cur).name().cstr(), (int)(*cur).offset(), v[0], v[1], v[2], v[3]);
         } break;
         default:
         {
            float* v = (float*)(_buffer + (*cur).offset());
            printf("%s: type=%d offset=%d data=%f,%f,%f,%f...\n",
                   (*cur).name().cstr(), (*cur).type(), (int)(*cur).offset(), v[0], v[1], v[2], v[3]);
         }
      }
   }
}


/*----- static utility routines -----*/

//------------------------------------------------------------------------------
//!
size_t
ConstantBuffer::getSize( const Container& constants )
{
   size_t maxSize = 0;
   Container::ConstIterator cur = constants.begin();
   Container::ConstIterator end = constants.end();
   for( ; cur != end; ++cur )
   {
      size_t tmp = (*cur).offset();
      if( (*cur).count() == 0 )
      {
         tmp += toBytes( (*cur).type() );
      }
      else
      {
         // Does D3D require the actual size, not the shadow buffer one?
         //tmp += toBytes( (*cur).type() ) * (*cur).count();
         tmp += sizeof(void*);
      }

      if( maxSize < tmp ) maxSize = tmp;
   }
   return maxSize;
}



/*==============================================================================
   CLASS ConstantList
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<ConstantList>
ConstantList::create( const RCP<ConstantBuffer>& constantBuffer )
{
   RCP<ConstantList> tmp( new ConstantList() );
   if( constantBuffer.isValid() ) tmp->addBuffer( constantBuffer );
   return tmp;
}

//------------------------------------------------------------------------------
//!
ConstantList::ConstantList()
{
}

//------------------------------------------------------------------------------
//!
ConstantList::~ConstantList()
{
}
