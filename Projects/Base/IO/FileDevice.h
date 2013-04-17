/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_FILE_DEVICE_H
#define BASE_FILE_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/IO/IODevice.h>
#include <Base/IO/Path.h>

#include <cstdio>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS FileDevice
==============================================================================*/
class FileDevice:
   public IODevice
{
public:

   /*----- methods -----*/

   BASE_DLL_API FileDevice( FILE* file, Mode mode );
   BASE_DLL_API FileDevice( const char* filename, Mode mode );
   BASE_DLL_API FileDevice( const String& filename, Mode mode );
   BASE_DLL_API FileDevice( const Path& path, Mode mode );
   BASE_DLL_API virtual ~FileDevice();

   BASE_DLL_API bool  open( FILE* file, Mode mode );
   BASE_DLL_API bool  open( const char* filename, Mode mode );
   BASE_DLL_API bool  open( const String& filename, Mode mode );
   BASE_DLL_API bool  open( const Path& path, Mode mode );
   BASE_DLL_API void  close();

   inline bool isOpen() const { return _file != NULL; }
   inline bool eof() const { return feof(_file) != 0; }

   inline void skip( size_t n ) { fseek(_file, long(n), SEEK_CUR); }

protected:

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );
   BASE_DLL_API virtual bool    doFlush();


   /*----- members -----*/
   FILE*  _file;
   bool   _skipClose;

private:
}; //class FileDevice

NAMESPACE_END

#endif //BASE_FILE_DEVICE_H
