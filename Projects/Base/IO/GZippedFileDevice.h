/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_GZIPPED_FILE_DEVICE_H
#define BASE_GZIPPED_FILE_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/IO/IODevice.h>
#include <Base/IO/Path.h>

#include <cstdio>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS GZippedFileDevice
==============================================================================*/
class GZippedFileDevice:
   public IODevice
{
public:

   /*----- methods -----*/

   BASE_DLL_API GZippedFileDevice( const char* filename, Mode mode );
   BASE_DLL_API virtual ~GZippedFileDevice();

   BASE_DLL_API bool  open( const char* filename, Mode mode );
   BASE_DLL_API void  close();

   inline bool isOpen() const { return _file != NULL; }
   BASE_DLL_API bool eof() const;

   BASE_DLL_API void skip( size_t n );

protected:

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );
   BASE_DLL_API virtual bool    doFlush();


   /*----- members -----*/
   void*  _file;
   bool   _skipClose;

private:
}; //class GZippedFileDevice

NAMESPACE_END

#endif //BASE_GZIPPED_FILE_DEVICE_H
