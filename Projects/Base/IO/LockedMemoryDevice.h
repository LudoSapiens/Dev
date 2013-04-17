/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_LOCKED_MEMORY_DEVICE_H
#define BASE_LOCKED_MEMORY_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/IO/IODevice.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS LockedMemoryDevice
==============================================================================*/
class LockedMemoryDevice:
   public IODevice
{
public:

   /*----- types -----*/
   typedef Vector<uint8_t>  DataContainer;

   /*----- methods -----*/

   BASE_DLL_API LockedMemoryDevice( char* data, size_t size, Mode mode );
   BASE_DLL_API LockedMemoryDevice( const char* data, size_t size );
   BASE_DLL_API virtual ~LockedMemoryDevice();

   BASE_DLL_API void  set( char* data, size_t size );
   BASE_DLL_API void  reset();

   inline size_t  size() const { return _size; }

protected:

   /*----- data members -----*/

   char*   _data;
   size_t  _size;
   size_t  _pos;

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );
   //BASE_DLL_API virtual bool    doFlush();

private:
}; //class LockedMemoryDevice


NAMESPACE_END

#endif //BASE_LOCKED_MEMORY_DEVICE_H
