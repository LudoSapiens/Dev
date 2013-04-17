/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BINARY_RESOURCE_TASK_H
#define PLASMA_BINARY_RESOURCE_TASK_H

#include <Plasma/StdDefs.h>

#include <Plasma/Geometry/MeshGeometry.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/String.h>
#include <Base/IO/BinaryStream.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/GZippedFileDevice.h>
#include <Base/Msg/Delegate.h>
#include <Base/MT/Task.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS BinaryResourceTask
==============================================================================*/
template< typename T >
class BinaryResourceTask:
   public Task
{
public:

   /*----- types -----*/

   typedef Delegate2< BinaryStream&, Resource<T>*, bool >  ResourceLoadDelegate;

   /*----- methods -----*/

   BinaryResourceTask( 
      Resource<T>*                resource, 
      const ResourceLoadDelegate& loadDel, 
      const String&               path, 
      const Table&                table
   );

protected:

   /*----- data members -----*/

   RCP< Resource<T> >    _resource;
   ResourceLoadDelegate  _loadDel;
   String                _path;
   //RCP<const Table>      _params;

   /*----- methods -----*/

   virtual void execute();

private:
}; //class BinaryResourceTask

//------------------------------------------------------------------------------
//!
template< typename T >
BinaryResourceTask<T>::BinaryResourceTask(
   Resource<T>*                resource,
   const ResourceLoadDelegate& loadDel,
   const String&               path,
   const Table&                /*params*/
):
   _resource( resource ),
   _loadDel( loadDel ),
   _path( path )
   //_params( &params )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > void
BinaryResourceTask<T>::execute()
{
   BinaryStream is = BinaryStream( new GZippedFileDevice( _path.cstr(), IODevice::MODE_READ ) );
   _loadDel( is, _resource.ptr() );
}

// Forward-declaration of the types we care about.
typedef BinaryResourceTask<Geometry>  BinaryGeometryTask;

NAMESPACE_END

#endif //PLASMA_BINARY_RESOURCE_TASK_H
