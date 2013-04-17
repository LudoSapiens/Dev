/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_STRING_DEVICE_H
#define BASE_STRING_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/IO/IODevice.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS StringDevice
==============================================================================*/
//! Adaptor class to allow a TextStream to work on a String in memory.
class StringDevice:
   public IODevice
{
public:

   /*----- methods -----*/

   BASE_DLL_API StringDevice( String& str, Mode mode = MODE_READ_WRITE );
   BASE_DLL_API virtual ~StringDevice();

   //      String&  str()       { return _string; }
   const String&  str() const { return *_string; }
   void  str( String& str ) { _string = &str; _pos = 0; }

   BASE_DLL_API void  reset();

protected:

   /*----- data members -----*/

   String*  _string;
   size_t   _pos;

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );
   BASE_DLL_API virtual size_t  doPos() const;

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );

private:
}; //class StringDevice


NAMESPACE_END

#endif //BASE_STRING_DEVICE_H
