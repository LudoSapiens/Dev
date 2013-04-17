/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/Geometry.h>
#include <Plasma/World/WorldVM.h>

#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

const char* _geometry_str = "geometry";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Geometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Geometry::initialize()
{
   VMRegistry::add( _geometry_str, NULL, geometry_get, geometry_set, VM_CAT_APP );
}

//------------------------------------------------------------------------------
//!
Geometry::Geometry( int type ):
   _type( type ), _collisionType( MANUAL ),
   _bbox( AABBoxf::empty() ), _com( Reff::identity() ), _inertiaTensor(1.0f),
   _rgeomIsOK( false )
{
}

//------------------------------------------------------------------------------
//!
Geometry::~Geometry()
{
}

//------------------------------------------------------------------------------
//!
void
Geometry::collisionShape( CollisionShape* shape )
{
   _shape = shape;
   updateShape();
}

//------------------------------------------------------------------------------
//!
void
Geometry::updateShape()
{
   if( _shape.isValid() && _shape->type() == CollisionShape::GROUP )
   {
      CollisionGroup* group = (CollisionGroup*)_shape.ptr();
      group->inertiaTensor( _inertiaTensor );
      group->centerOfMass( _com );
   }
}

//------------------------------------------------------------------------------
//!
const char*
Geometry::meta() const
{
   return _geometry_str;
}

//------------------------------------------------------------------------------
//!
void
Geometry::print( TextStream& os ) const
{
   switch( type() )
   {
      case SURFACE:
         os << "Type: SURFACE" << nl;
         break;
      case METAGEOMETRY:
         os << "Type: METAGEOMETRY" << nl;
         break;
      case MESH:
         os << "Type: MESH" << nl;
         break;
      case SILHOUETTE:
         os << "Type: SILHOUETTE" << nl;
         break;
      default:
         os << "Type: UNKNOWN" << nl;
         break;
   }
   printPatches( os );
}

//------------------------------------------------------------------------------
//!
void
Geometry::printPatches( TextStream& os ) const
{
   uint n = numPatches();
   os << n << " patches:" << nl;
   for( uint i = 0; i < n; ++i )
   {
      const PatchInfo& info = patchInfo( i );
      os << "[" << i << "] start=" << info.rangeStart() << " size=" << info.rangeSize() << " mat=" << info.materialID() << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
Geometry::computeBonesWeights()
{
}

NAMESPACE_END
