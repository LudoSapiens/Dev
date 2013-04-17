/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BOUNDARYPOLYGON
#define PLASMA_BOUNDARYPOLYGON

#include <Plasma/StdDefs.h>

#include <CGMath/Polygon.h>
#include <CGMath/AABBox.h>
#include <CGMath/AARect.h>

#include <Base/Util/RCObject.h>
#include <Base/ADT/ConstString.h>

NAMESPACE_BEGIN

class Boundary;

/*==============================================================================
   CLASS BoundaryPolygon
==============================================================================*/

class BoundaryPolygon:
   public Polygonf
{
public:

   /*----- static methods -----*/

   static RCP<BoundaryPolygon> create( const AARectf& box, const Mat4f& );
   static RCP<BoundaryPolygon> create( const AABBoxf&, int f, const Mat4f& );
   static RCP<BoundaryPolygon> create( const Boundary&, int f );
   static RCP<BoundaryPolygon> create( const BoundaryPolygon&, const Vec3f&, const Reff& );
   static void create( const AABBoxf&, const Mat4f&, Vector< RCP<BoundaryPolygon> >& );
   static void create( const Boundary&, Vector< RCP<BoundaryPolygon> >& );

   /*----- methods -----*/

   BoundaryPolygon();
   virtual ~BoundaryPolygon();

   inline void id( const ConstString& str ) { _id = str; }
   inline const ConstString& id() const     { return _id; }

   /*----- methods -----*/

protected:

   /*----- data members -----*/

   ConstString _id;
};

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& stream, const BoundaryPolygon& poly )
{
   stream << poly.id().cstr() << " p: " << poly.plane() << " v: ";
   for( uint i = 0; i < poly.numVertices(); ++i )
   {
      stream << poly.vertex(i);
   }
   return stream;
}

NAMESPACE_END

#endif
