/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Collision/CollisionGroup.h>


NAMESPACE_BEGIN


/*==============================================================================
   CLASS CollisionGroup
==============================================================================*/

//------------------------------------------------------------------------------
//!
CollisionGroup::CollisionGroup():
   CollisionShape( GROUP )
{
}

//------------------------------------------------------------------------------
//!
CollisionGroup::~CollisionGroup()
{
}

//------------------------------------------------------------------------------
//!
Vec3f
CollisionGroup::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   Vec3f bestPos( CGConstf::NaN() );
   float bestDist = -1.0f;
   for( uint i = 0; i < numShapes(); ++i )
   {
      Vec3f tmpPos  = shape(i)->getFarthestPointAlong( ref * shapeRef(i), dir );
      float tmpDist = sqrLength( tmpPos );
      if( tmpDist > bestDist )
      {
         bestPos  = tmpPos;
         bestDist = tmpDist;
      }
   }
   return bestPos;
}

NAMESPACE_END
