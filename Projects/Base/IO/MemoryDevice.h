/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MEMORY_DEVICE_H
#define BASE_MEMORY_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/IO/IODevice.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS MemoryDevice
==============================================================================*/
class MemoryDevice:
   public IODevice
{
public:

   /*----- types -----*/
   typedef Vector<uint8_t>  DataContainer;

   /*----- methods -----*/

   BASE_DLL_API MemoryDevice( Mode mode, size_t s = 0 );
   BASE_DLL_API virtual ~MemoryDevice();

   BASE_DLL_API void  reset();

   inline void  reserve( size_t s ) { _data.reserve(s); }

   inline size_t  size() const { return _data.size(); }

protected:

   /*----- data members -----*/

   DataContainer  _data;
   size_t         _pos;

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );
   //BASE_DLL_API virtual bool    doFlush();

private:
}; //class MemoryDevice


NAMESPACE_END

#endif //BASE_MEMORY_DEVICE_H
