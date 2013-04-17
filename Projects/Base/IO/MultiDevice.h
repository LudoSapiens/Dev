/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MULTI_DEVICE_H
#define BASE_MULTI_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/IO/IODevice.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS MultiDevice
==============================================================================*/
//! Adaptor class to allow a TextStream to work on a String in memory.
class MultiDevice:
   public IODevice
{
public:

   /*----- types -----*/
   typedef Vector< RCP<IODevice> >    DeviceContainer;
   typedef DeviceContainer::Iterator  Iterator;

   /*----- methods -----*/

   BASE_DLL_API MultiDevice();
   BASE_DLL_API virtual ~MultiDevice();

   inline void  add( IODevice* device ) { _devices.pushBack(device); addMode( device->mode() ); }

   const DeviceContainer&  devices() const { return _devices; }

protected:

   /*----- data members -----*/

   DeviceContainer  _devices;  //!< All of the devices.

   /*----- methods -----*/

   BASE_DLL_API virtual bool    doSeek( size_t pos );

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );
   BASE_DLL_API virtual bool    doFlush();

private:
}; //class MultiDevice


NAMESPACE_END

#endif //BASE_MULTI_DEVICE_H
