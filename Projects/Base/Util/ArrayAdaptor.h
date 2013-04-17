/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ARRAY_ADAPTOR_H
#define BASE_ARRAY_ADAPTOR_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ArrayAdaptor
==============================================================================*/
template< typename T >
class ArrayAdaptor
{
public:

   /*----- methods -----*/

   inline ArrayAdaptor() {}
   inline ArrayAdaptor( void* data, size_t stride ): _data( data ), _stride( stride ) {}

   inline       T&  operator[]( size_t idx )       { U u; u._voidData = _data; u._byteData += _stride*idx; return *(u._typeData); }
   inline const T&  operator[]( size_t idx ) const { U u; u._voidData = _data; u._byteData += _stride*idx; return *(u._typeData); }

   inline void* data() const { return _data; }
   inline void data( void* d ) { _data = d; }

   inline size_t stride() const { return _stride; }
   inline void stride( size_t s ) { _stride = s; }

   inline void  set( void* data, size_t stride ) { _data = data; _stride = stride; }

protected:

   /*----- types -----*/
   union U
   {
      void*  _voidData;
      char*  _byteData;
      T*     _typeData;
   };

   /*----- data members -----*/

   void*   _data;
   size_t  _stride;

}; //class ArrayAdaptor


/*==============================================================================
  CLASS ConstArrayAdaptor
==============================================================================*/
template< typename T >
class ConstArrayAdaptor
{
public:

   /*----- methods -----*/

   inline ConstArrayAdaptor() {}
   inline ConstArrayAdaptor( const void* data, size_t stride ): _data( data ), _stride( stride ) {}

   inline const T&  operator[]( size_t idx ) const { U u; u._voidData = _data; u._byteData += _stride*idx; return *(u._typeData); }

   inline void* data() const { return _data; }
   inline void data( void* d ) { _data = d; }

   inline size_t stride() const { return _stride; }
   inline void stride( size_t s ) { _stride = s; }

   inline void  set( const void* data, size_t stride ) { _data = data; _stride = stride; }

protected:

   /*----- types -----*/
   union U
   {
      const void*  _voidData;
      const char*  _byteData;
      const T*     _typeData;
   };

   /*----- data members -----*/

   const void*  _data;
   size_t       _stride;

}; //class ConstArrayAdaptor


NAMESPACE_END

#endif //BASE_ARRAY_ADAPTOR_H
