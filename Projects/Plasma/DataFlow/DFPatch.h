/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFPATCH_H
#define PLASMA_DFPATCH_H

#include <Plasma/StdDefs.h>

#include <Plasma/DataFlow/DFGeometry.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFPatch
==============================================================================*/

class DFPatch 
{
public:

   /*----- methods -----*/

   // Creation.
   void init( const DFGeometry& geom, const DFGeometry::Patch& p );
   
   // Evaluation.
   void parameters( const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const;
   const Vec3f& position( uint i ) const;
   int flatness( const Vec2f& uv0, const Vec2f& uv1, int edge, float error );
   
   void print() const;

private:

   /*----- members -----*/

   void evalCubic( const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const;
   //void evalTriangle( uint p, const Vec3f& uvw, Vec3f& pos, Vec3f& normal ) const;

   /*----- data members -----*/

   Vec3f _b[16];
};

NAMESPACE_END

#endif
