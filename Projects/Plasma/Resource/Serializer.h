/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SERIALIZER_H
#define PLASMA_SERIALIZER_H

#include <Plasma/StdDefs.h>

#include <Fusion/Resource/Resource.h>

#include <Base/IO/BinaryStream.h>

NAMESPACE_BEGIN

class CollisionShape;
class Geometry;
class MeshGeometry;

namespace Serializer
{

PLASMA_DLL_API bool  dumpGeometry( const Geometry& geom, BinaryStream& os );
PLASMA_DLL_API bool  dumpMeshGeometry( const MeshGeometry& geom, BinaryStream& os );
PLASMA_DLL_API bool  dumpPatches( const Geometry& geom, BinaryStream& os );
PLASMA_DLL_API bool  dumpCollisionShape( const Geometry& geom, BinaryStream& os );
PLASMA_DLL_API bool  dumpCollisionShape( const CollisionShape& geom, BinaryStream& os );

PLASMA_DLL_API bool  loadGeometry( BinaryStream& is, Resource<Geometry>* res );
PLASMA_DLL_API bool  loadMeshGeometry( BinaryStream& is, Resource<Geometry>* res );

PLASMA_DLL_API bool  loadPatches( BinaryStream& is, MeshGeometry& geom );
PLASMA_DLL_API bool  loadCollisionShape( BinaryStream& is, Geometry& geom );
PLASMA_DLL_API RCP<CollisionShape>  loadCollisionShape( BinaryStream& is );

} // namespace Serializer

NAMESPACE_END

#endif //PLASMA_SERIALIZER_H
