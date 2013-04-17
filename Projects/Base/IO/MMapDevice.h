/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MMAP_DEVICE_H
#define BASE_MMAP_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/IO/FileSystem.h>
#include <Base/IO/IODevice.h>
#include <Base/IO/Path.h>
#include <Base/Util/Platform.h>

#include <cstdio>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS MMapDevice
==============================================================================*/
class MMapDevice:
   public IODevice
{
public:

   /*----- methods -----*/

   BASE_DLL_API MMapDevice( const FS::Entry& fse, Mode mode );
   BASE_DLL_API virtual ~MMapDevice();

   BASE_DLL_API bool  open( const FS::Entry&, Mode mode );
   BASE_DLL_API void  close();

   inline bool isOpen() const { return _data != NULL; }

   inline       void*  data()       { return _data; }
   inline const void*  data() const { return _data; }

   inline       char*  bytes()       { return (      char*)_data; }
   inline const char*  bytes() const { return (const char*)_data; }

   inline size_t  size() const { return _entry.size(); }

protected:

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );
   BASE_DLL_API virtual bool    doFlush();


   /*----- members -----*/
   FILE*      _file;
#if PLAT_POSIX == 0 && PLAT_WINDOWS != 0
   void*      _mapping; // Only used under Windows.
#endif
   FS::Entry  _entry;
   void*      _data;
   char*      _pos;

private:
}; //class MMapDevice

NAMESPACE_END

#endif //BASE_MMAP_DEVICE_H
