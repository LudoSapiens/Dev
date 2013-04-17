/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_NULL_DEVICE_H
#define BASE_NULL_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/IO/IODevice.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS NullDevice
==============================================================================*/
class NullDevice:
   public IODevice
{
public:
   /*----- methods -----*/

   BASE_DLL_API NullDevice( Mode mode = IODevice::MODE_READ_WRITE );
   BASE_DLL_API virtual ~NullDevice();

protected:

   /*----- methods -----*/

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );

private:
}; //class NullDevice

NAMESPACE_END

#endif //BASE_NULL_DEVICE_H
