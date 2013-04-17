/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Intersector.h>
#include <Plasma/Geometry/MetaGeometry.h>
#include <Plasma/Geometry/SurfaceGeometry.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/World/World.h>
#include <Plasma/World/SkeletalEntity.h>

#include <CGMath/CGMath.h>

#include <Base/Dbg/Defs.h>
#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_b, "Intersector" );

//------------------------------------------------------------------------------
//!
bool intersectTri( const Rayf& ray, uint id, float& t, void* data )
{
   Intersector::Hit* hit             = (Intersector::Hit*)data;
   SurfaceGeometry* surface          = (SurfaceGeometry*)hit->_geom;
   SurfaceGeometry::Patch* patch     = surface->patches()[id>>24].ptr();
   const SurfaceGeometry::Face* face = &patch->faces()[id&0xffffff];

   if( Intersector::trace(
            surface->vertex( face->_vID[0] ),
            surface->vertex( face->_vID[1] ),
            surface->vertex( face->_vID[2] ),
            ray,
            *hit
         )
      )
   {
      t          = hit->_t;
      hit->_face = face;
      return true;
   }
   return false;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Intersector
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   World*       world,
   const Rayf&  ray,
   Hit&         hit
)
{
   bool hitSomething = false;

   // Interesect each entity.
   for( uint i = 0; i < world->numEntities(); ++i )
   {
      hitSomething |= trace( world->entity(i), ray, hit );
   }

   return hitSomething;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   World*       world,
   const Rayf&  ray
)
{
   // Interesect each entity.
   for( uint i = 0; i < world->numEntities(); ++i )
   {
      if( trace( world->entity(i), ray ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   Entity*      entity,
   const Rayf&  ray,
   Hit&         hit
)
{
   if( !entity->visible() || entity->ghost() )  return false;

   // Transform ray into object space.
   Mat4f mat( entity->referential().globalToLocal() );
   Rayf tray( mat * ray );

   Geometry* geom = entity->geometry();

   if( entity->type() == Entity::SKELETAL )
   {
      SkeletalEntity* skel = (SkeletalEntity*)entity;
      if( !trace( geom, skel->transforms(), tray, hit ) ) return false;
   }
   else
   {
       if( !trace( geom, tray, hit ) ) return false;
   }

   // We have a hit.
   hit._entity = entity;
   // World space pos and normal.
   hit._pos    = ray.point( hit._t );
   hit._normal = entity->transform() ^ hit._normal;

   return true;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   Entity*      entity,
   const Rayf&  ray
)
{
   if( !entity->visible() || entity->ghost() )  return false;

   // Transform ray into object space.
   Mat4f mat( entity->referential().globalToLocal() );
   Rayf tray( mat * ray );

   Geometry* geom = entity->geometry();
   if( entity->type() == Entity::SKELETAL )
   {
      SkeletalEntity* skel = (SkeletalEntity*)entity;
      return trace( geom, skel->transforms(), tray );
   }
   else
   {
       return trace( geom, tray );
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   Geometry*   geom,
   const Rayf& ray,
   Hit&        hit
)
{
   if( geom == 0 )  return false;

   // Do we intersect the bounding?
   float mint = 0.0f;
   float maxt = hit._t;
   if( !trace( geom->boundingBox(), ray, mint, maxt ) ) return false;

   switch( geom->type() )
   {
      case Geometry::METAGEOMETRY:
         if( !((MetaGeometry*)geom)->trace( ray, hit ) ) return false;
         hit._geom = geom;
         return true;
         break;
      case Geometry::SURFACE:
      {
         SurfaceGeometry* surface = (SurfaceGeometry*)geom;
         // FIXME: Temporary code.
         if( surface->bih().isEmpty() ) surface->computeBIH();

         if( surface->bih().isEmpty() )
         {
            SurfaceGeometry::PatchContainer::ConstIterator pIt  = surface->patches().begin();
            SurfaceGeometry::PatchContainer::ConstIterator pEnd = surface->patches().end();
            for( ; pIt != pEnd; ++pIt )
            {
               // Loop over all triangle.
               SurfaceGeometry::FaceContainer::ConstIterator fIt  = (*pIt)->faces().begin();
               SurfaceGeometry::FaceContainer::ConstIterator fEnd = (*pIt)->faces().end();

               for( ; fIt != fEnd; ++fIt )
               {
                  if( trace(
                        surface->vertex( (*fIt)._vID[0] ),
                        surface->vertex( (*fIt)._vID[1] ),
                        surface->vertex( (*fIt)._vID[2] ),
                        ray,
                        hit
                     )
                  )
                  {
                     hit._face = &(*fIt);
                     hit._geom = surface;

                     // Normal.
                     const Vec3f& n0 = surface->normal( (*fIt)._dID[0] );
                     const Vec3f& n1 = surface->normal( (*fIt)._dID[1] );
                     const Vec3f& n2 = surface->normal( (*fIt)._dID[2] );

                     hit._normal = (n1-n0)*hit._barypos.x + (n2-n0)*hit._barypos.y + n0;
                     hit._normal.normalize();
                  }
               }
            }
            return hit._geom == surface;
         }

         BIH::Hit bhit;
         bhit._t            = hit._t;
         Geometry* prevGeom = hit._geom;
         hit._geom          = surface;

         if( surface->bih().trace( ray, bhit, &intersectTri, &hit ) )
         {
            // Normal.
            const SurfaceGeometry::Face* face = (SurfaceGeometry::Face*)hit._face;
            const Vec3f& n0 = surface->normal( face->_dID[0] );
            const Vec3f& n1 = surface->normal( face->_dID[1] );
            const Vec3f& n2 = surface->normal( face->_dID[2] );

            hit._normal = (n1-n0)*hit._barypos.x + (n2-n0)*hit._barypos.y + n0;
            hit._normal.normalize();
            return true;
         }
         hit._geom = prevGeom;
      }  break;
      case Geometry:: MESH:
      {
         MeshGeometry* mesh = (MeshGeometry*)geom;

         // Only intersect triangles mesh.
         if( mesh->primitiveType() != MeshGeometry::TRIANGLES ) return false;

         const uint32_t* idx = mesh->indices();
         for( uint i = 0; i < mesh->numPrimitives(); ++i, idx += 3 )
         {
            if( trace(
                  mesh->position( idx[0] ),
                  mesh->position( idx[1] ),
                  mesh->position( idx[2] ),
                  ray,
                  hit
               )
            )
            {
               hit._fid  = i;
               hit._geom = mesh;
               // Normal.
               if( mesh->hasNormal() )
               {
                  const Vec3f& n0 = mesh->normal( idx[0] );
                  const Vec3f& n1 = mesh->normal( idx[1] );
                  const Vec3f& n2 = mesh->normal( idx[2] );
                  hit._normal = (n1-n0)*hit._barypos.x + (n2-n0)*hit._barypos.y + n0;
                  hit._normal.normalize();
               }
            }
         }
         return hit._geom == mesh;
      }  break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   Geometry*   geom,
   const Rayf& ray
)
{
   if( geom == 0 )  return false;

   // Do we intersect the bounding?
   float mint = 0.0f;
   float maxt = 1.0f;
   if( !trace( geom->boundingBox(), ray, mint, maxt ) ) return false;

   switch( geom->type() )
   {
      case Geometry::METAGEOMETRY:
         return ((MetaGeometry*)geom)->trace( ray );
         break;
      case Geometry::SURFACE:
      {
         SurfaceGeometry* surface = (SurfaceGeometry*)geom;
         // FIXME: Temporary code.
         if( surface->bih().isEmpty() ) surface->computeBIH();

         if( surface->bih().isEmpty() )
         {
            Hit hit;
            hit._t = 1.0f;
            SurfaceGeometry::PatchContainer::ConstIterator pIt  = surface->patches().begin();
            SurfaceGeometry::PatchContainer::ConstIterator pEnd = surface->patches().end();
            for( ; pIt != pEnd; ++pIt )
            {
               // Loop over all triangle.
               SurfaceGeometry::FaceContainer::ConstIterator fIt  = (*pIt)->faces().begin();
               SurfaceGeometry::FaceContainer::ConstIterator fEnd = (*pIt)->faces().end();

               for( ; fIt != fEnd; ++fIt )
               {
                  if( trace(
                        surface->vertex( (*fIt)._vID[0] ),
                        surface->vertex( (*fIt)._vID[1] ),
                        surface->vertex( (*fIt)._vID[2] ),
                        ray,
                        hit
                     )
                  ) return true;
               }
            }
            return false;
         }

         BIH::Hit bhit;
         bhit._t   = 1.0f;
         Hit hit;
         hit._t    = 1.0f;
         hit._geom = surface;

         return surface->bih().trace( ray, bhit, &intersectTri, &hit );
      }  break;
      case Geometry:: MESH:
      {
         MeshGeometry* mesh = (MeshGeometry*)geom;

         // Only intersect triangles mesh.
         if( mesh->primitiveType() != MeshGeometry::TRIANGLES ) return false;

         const uint32_t* idx = mesh->indices();
         Hit hit;
         hit._t = 1.0f;
         for( uint i = 0; i < mesh->numPrimitives(); ++i, idx += 3 )
         {
            if( trace(
                  mesh->position( idx[0] ),
                  mesh->position( idx[1] ),
                  mesh->position( idx[2] ),
                  ray,
                  hit
               )
            ) return true;
         }
         return false;
      }  break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   Geometry*              geom,
   const Vector< Mat4f >& matrices,
   const Rayf&            ray,
   Hit&                   hit
)
{
   if( geom == 0 ) return false;

   // FIXME: No bounding box test for now.
   // Only accepts SurfaceGeometry.
   SurfaceGeometry* surface = geom->surface();
   if( !surface) return false;

   SurfaceGeometry::PatchContainer::ConstIterator pIt  = surface->patches().begin();
   SurfaceGeometry::PatchContainer::ConstIterator pEnd = surface->patches().end();

   for( ; pIt != pEnd; ++pIt )
   {
      // Loop over all triangle.
      SurfaceGeometry::FaceContainer::ConstIterator fIt  = (*pIt)->faces().begin();
      SurfaceGeometry::FaceContainer::ConstIterator fEnd = (*pIt)->faces().end();

      for( ; fIt != fEnd; ++fIt )
      {
         Vec3f vs0 = surface->wvertex( (*fIt)._vID[0], matrices );
         Vec3f vs1 = surface->wvertex( (*fIt)._vID[1], matrices );
         Vec3f vs2 = surface->wvertex( (*fIt)._vID[2], matrices );

         if( trace( vs0, vs1, vs2, ray, hit ) )
         {
            hit._face = &(*fIt);
            hit._geom = surface;

            // FIXME: should be transformed!!
            // Normal.
            const Vec3f& n0 = surface->normal( (*fIt)._dID[0] );
            const Vec3f& n1 = surface->normal( (*fIt)._dID[1] );
            const Vec3f& n2 = surface->normal( (*fIt)._dID[2] );

            hit._normal =
               ( n1 - n0 )*hit._barypos.x +
               ( n2 - n0 )*hit._barypos.y + n0;
            hit._normal.normalize();
         }
      }
   }

   return hit._geom == surface;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   Geometry*              geom,
   const Vector< Mat4f >& matrices,
   const Rayf&            ray
)
{
   if( geom == 0 ) return false;

   // FIXME: No bounding box test for now.
   // Only accepts SurfaceGeometry.
   SurfaceGeometry* surface = geom->surface();
   if( !surface) return false;

   SurfaceGeometry::PatchContainer::ConstIterator pIt  = surface->patches().begin();
   SurfaceGeometry::PatchContainer::ConstIterator pEnd = surface->patches().end();

   Hit hit;
   hit._t = 1;

   for( ; pIt != pEnd; ++pIt )
   {
      // Loop over all triangle.
      SurfaceGeometry::FaceContainer::ConstIterator fIt  = (*pIt)->faces().begin();
      SurfaceGeometry::FaceContainer::ConstIterator fEnd = (*pIt)->faces().end();

      for( ; fIt != fEnd; ++fIt )
      {
         Vec3f vs0 = surface->wvertex( (*fIt)._vID[0], matrices );
         Vec3f vs1 = surface->wvertex( (*fIt)._vID[1], matrices );
         Vec3f vs2 = surface->wvertex( (*fIt)._vID[2], matrices );

         if( trace( vs0, vs1, vs2, ray, hit ) ) return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   const Vec3f& center,
   const float  radius,
   const Rayf&  ray,
   float&       distance
)
{
   Vec3f dist = ray.origin() - center;
   float a = ray.direction().sqrLength();
   float b = ray.direction().dot( dist );
   float c = dist.sqrLength() - radius*radius;
   float d = b*b - a*c;
   CHECK( !CGM::equal(a, 0.0f) );  // The ray's directon is too small
   if( d > 0.0f )
   {
      d = CGM::sqrt( d );
      distance = -b > d ? (-b-d)/a : (-b+d)/a;
      return true;
   }
   else
   if( CGM::equal(d, 0.0f) )
   {
      distance = -b/a;
      return true;
   }
   else
   {
      distance = CGConstf::infinity();
      return false;
   }
}

//------------------------------------------------------------------------------
//! Ray-sphere intersection returning 0, 1, or 2 intersections.
uint
Intersector::trace(
   const Vec3f& center,
   const float  radius,
   const Rayf&  ray,
   float&       d1,
   float&       d2
)
{
   Vec3f dist = ray.origin() - center;
   float a = ray.direction().sqrLength();
   float b = ray.direction().dot( dist );
   float c = dist.sqrLength() - radius*radius;
   float d = b*b - a*c;
   CHECK( !CGM::equal(a, 0.0f) );  // The ray's directon is too small
   if( d > 0.0f )
   {
      d = CGM::sqrt( d );
      d1 = (-b-d)/a;
      d2 = (-b+d)/a;
      return 2;
   }
   else if( CGM::equal(d, 0.0f) )
   {
      d1 = d2 = -b/a;
      return 1;
   }
   else
   {
      d1 = d2 = CGConstf::infinity();
      return 0;
   }
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   const Vec3f& p0,
   const Vec3f& p1,
   const Vec3f& p2,
   const Rayf&  ray,
   Hit&         hit
)
{
   DBG_BLOCK( os_b, "Intersector::trace" );
   // Compute triangle vectors
   Vec3f e1 = p1 - p0;
   Vec3f e2 = p2 - p0;

   // Determinant.
   Vec3f pv  = ray.direction().cross(e2);
   float det = e1.dot( pv );

   DBG_MSG( os_b, "e1: " << e1 << " e2: " << e2 << " pv: " << pv << " det: " << det );

   if( det > -1e-15 && det < 1e-15 ) return false;

   float invd = 1.0f / det;

   // Compute distance.
   Vec3f tv = ray.origin() - p0;

   DBG_MSG( os_b, "tv: " << tv );

   // Compute first barycentric.
   float u = tv.dot( pv ) * invd;
   if( u < -1e-5 || u > 1+1e-5 ) return false;

   // Compute second barycentric.
   Vec3f qv = tv.cross(e1);
   float v  = ray.direction().dot(qv) * invd;
   DBG_MSG( os_b, "qv: " << qv );
   if( v < -1e-5 || u + v > 1+1e-5 ) return false;

   // Compute intersection.
   float t = (float)e2.dot(qv)*invd;
   if( t < hit._tmin || t > hit._t ) return false;

   hit._t         = t;
   hit._pos       = ray.point( t );
   hit._barypos.x = u;
   hit._barypos.y = v;
   return true;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::getBarycentric(
   const Vec3f& p0,
   const Vec3f& p1,
   const Vec3f& p2,
   const Rayf&  ray,
   Hit&         hit
)
{
   // Compute triangle vectors
   Vec3f e1 = p1 - p0;
   Vec3f e2 = p2 - p0;

   // Determinant.
   Vec3f pv   = ray.direction().cross(e2);
   float det = e1.dot( pv );

   if( det > -1e-15 && det < 1e-15 )  return false;

   float invd = 1.0f / det;

   // Compute distance.
   Vec3f tv = ray.origin() - p0;

   // Compute first barycentric.
   float u = tv.dot( pv ) * invd;

   // Compute second barycentric.
   Vec3f qv = tv.cross(e1);
   float v  = ray.direction().dot(qv) * invd;

   hit._barypos.x = u;
   hit._barypos.y = v;

   return true;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   const Planef& plane,
   const Rayf&   ray,
   float&        distance
)
{
   float a = plane.d() + plane.direction().dot( ray.origin() );
   float b = plane.direction().dot( ray.direction() );

   // Ray in triangle plane?
   if( fabs( b ) < 0.0001f ) return false;

   float r = -a / b;

   // Intersection behind ray.
   if( r < 0.0f || r > distance ) return false;

   distance = r;

   return true;
}

//------------------------------------------------------------------------------
//!
bool
Intersector::trace(
   const AABBoxf& bb,
   const Rayf&    ray,
   float&         minDistance,
   float&         maxDistance
)
{
   float minDist = minDistance;
   float maxDist = maxDistance;
   float tmin;
   float tmax;

   // Test x slabs.
   float divx = 1.0f / ray.direction().x;
   tmin = (bb.slabX()(0) - ray.origin().x) * divx;
   tmax = (bb.slabX()(1) - ray.origin().x) * divx;

   if( divx >= 0.0f )
   {
      if( tmin > minDist ) minDist = tmin;
      if( tmax < maxDist ) maxDist = tmax;
      if( minDist > maxDist ) return false;
   }
   else
   {
      if( tmax > minDist ) minDist = tmax;
      if( tmin < maxDist ) maxDist = tmin;
      if( minDist > maxDist ) return false;
   }

   // Test y slabs.
   float divy = 1.0f / ray.direction().y;
   tmin = (bb.slabY()(0) - ray.origin().y) * divy;
   tmax = (bb.slabY()(1) - ray.origin().y) * divy;

   if( divy >= 0.0f )
   {
      if( tmin > minDist ) minDist = tmin;
      if( tmax < maxDist ) maxDist = tmax;
      if( minDist > maxDist ) return false;
   }
   else
   {
      if( tmax > minDist ) minDist = tmax;
      if( tmin < maxDist ) maxDist = tmin;
      if( minDist > maxDist ) return false;
   }

   // Test z slabs.
   float divz = 1.0f / ray.direction().z;
   tmin = (bb.slabZ()(0) - ray.origin().z) * divz;
   tmax = (bb.slabZ()(1) - ray.origin().z) * divz;

   if( divz >= 0.0f )
   {
      if( tmin > minDist ) minDist = tmin;
      if( tmax < maxDist ) maxDist = tmax;
      if( minDist > maxDist ) return false;
   }
   else
   {
      if( tmax > minDist ) minDist = tmax;
      if( tmin < maxDist ) maxDist = tmin;
      if( minDist > maxDist ) return false;
   }
   minDistance = minDist;
   maxDistance = maxDist;
   return true;
}

//------------------------------------------------------------------------------
//!
bool 
Intersector::trace(
   const Boundary& boundary,
   const Rayf&     ray,
   float&          distance
)
{
   bool result = false;
   
   float mint = 0.0f;
   float maxt = distance;
   if( !trace( boundary.boundingBox(), ray, mint, maxt ) )
   {
      return false;
   }

   for( uint i = 0; i < boundary.numFaces(); ++i )
   {
      // Intersect plane.
      float t = distance;
      const Boundary::Face& face = boundary.face(i);
      Planef plane( face.normal(), boundary.vertex(i,0) );
      
      if( ray.direction().dot( plane.direction() ) < 0.0f  && trace( plane, ray, t ) )
      {
         // Intersect polygon.
         Vec3f pt = ray.point(t);
         
         // Projection.
         uint x = 0;
         uint y = 1;
         if( CGM::abs(face.normal().x) > CGM::abs(face.normal().y) )
         {
            if( CGM::abs(face.normal().x) > CGM::abs(face.normal().z) )
            {
               x = 2;
               y = 1;
            }
         }
         else
         {
            if( CGM::abs(face.normal().y) > CGM::abs(face.normal().z) )
            {
               x = 0;
               y = 2;
            }
         }
         
         // Test inclusion.
         bool in = false;
         uint i0, i1;
         for( i0 = face._size-1, i1 = 0; i1 < face._size; i0 = i1++ )
         {
            Vec3f v0 = boundary.vertex(i,i0);
            Vec3f v1 = boundary.vertex(i,i1);
            
            if( ((v0(y) <= pt(y)) && (v1(y) >  pt(y))) || ((v0(y) >  pt(y)) && (v1(y) <= pt(y))) )
            {
               float vt = (pt(y)-v0(y)) / (v1(y)-v0(y));
               if( pt(x) < v0(x) + vt * (v1(x)-v0(x)) )
               {
                  in = !in;
               }
            }
         }
         
         if( in )
         {
            result   = true;
            distance = t;
         }
      }
   }
   
   return result;
}

/*==============================================================================
   TRIANGLE-TRIANGLE INTERSECTION
==============================================================================*/
//! Based on a paper and code by Akenine-Möller.

//------------------------------------------------------------------------------
//! 
inline uint sort2( Vec2f& a )
{
   if( a.x > a.y )
   {
      CGM::swap( a.x, a.y );
      return 1;
   }
   return 0;
}

//------------------------------------------------------------------------------
//! 
inline bool computeIntervalsIsectline(
   float        vv0,
   float        vv1,
   float        vv2,
   float        d0,
   float        d1,
   float        d2,
   float        d0d1,
   float        d0d2,
   Vec2f&       isect
)
{
   if( d0d1 > 0.0f )
   {
      // Here we know that d0d2<=0.0.
      // That is d0, d1 are on the same side, D2 on the other or on the plane.
      isect.x = vv2+(vv0-vv2)*d2/(d2-d0);
      isect.y = vv2+(vv1-vv2)*d2/(d2-d1);
   }
   else if( d0d2 > 0.0f )
   {
      // Here we know that d0d1<=0.0.      
      isect.x = vv1+(vv0-vv1)*d1/(d1-d0);
      isect.y = vv1+(vv2-vv1)*d1/(d1-d2);
   }
   else if( d1*d2 > 0.0f || d0 != 0.0f )
   {
      // Here we know that d0d1<=0.0 or that d0!=0.0.
      isect.x = vv0+(vv1-vv0)*d0/(d0-d1);
      isect.y = vv0+(vv2-vv0)*d0/(d0-d2);
   }
   else if( d1 != 0.0f )
   {
      isect.x = vv1+(vv0-vv1)*d1/(d1-d0);
      isect.y = vv1+(vv2-vv1)*d1/(d1-d2);
   }
   else if( d2 != 0.0f )
   {
      isect.x = vv2+(vv0-vv2)*d2/(d2-d0);
      isect.y = vv2+(vv1-vv2)*d2/(d2-d1);
   }
   else
   {                                                   
      // Triangles are coplanar.
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//! 
bool 
Intersector::triTriIntersect(
   const Vec3f& v0,
   const Vec3f& v1,
   const Vec3f& v2,
   const Vec3f& u0,
   const Vec3f& u1,
   const Vec3f& u2
)
{
   // Compute plane equation of triangle(v0,v1,v2).
   Vec3f e1 = v1-v0;
   Vec3f e2 = v2-v0;
   Vec3f n1 = e1.cross( e2 );
   float d1 = -n1.dot( v0 );

   // Compute distance of u0, u1, u2 from triangle plane (v0,v1,v2).
   float du0 = n1.dot( u0 ) + d1;
   float du1 = n1.dot( u1 ) + d1;
   float du2 = n1.dot( u2 ) + d1;

   // Coplanarity robustness check.
   if( CGM::abs(du0) < 0.0001f ) du0 = 0.0f;
   if( CGM::abs(du1) < 0.0001f ) du1 = 0.0f;
   if( CGM::abs(du2) < 0.0001f ) du2 = 0.0f;

   float du0du1 = du0*du1;
   float du0du2 = du0*du2;

   // Same side and not coplanar.
   if( du0du1>0.0f && du0du2>0.0f ) return 0;

   // compute plane of triangle (u0,u1,u2).
   e1       = u1-u0;
   e2       = u2-u0;
   Vec3f n2 = e1.cross( e2 );
   float d2 = -n2.dot( u0 );

   // Compute distance of v0, v1, v2 from triangle plane (u0,u1,u2).
   float dv0 = n2.dot( v0 ) + d2;
   float dv1 = n2.dot( v1 ) + d2;
   float dv2 = n2.dot( v2 ) + d2;

   // Coplanarity robustness check.
   if( CGM::abs(dv0) < 0.0001f ) dv0 = 0.0f;
   if( CGM::abs(dv1) < 0.0001f ) dv1 = 0.0f;
   if( CGM::abs(dv2) < 0.0001f ) dv2 = 0.0f;
   
   float dv0dv1 = dv0*dv1;
   float dv0dv2 = dv0*dv2;
        
   // Same side and not coplanar.
   if( dv0dv1>0.0f && dv0dv2>0.0f ) return 0;

   // Compute direction of intersection line.
   Vec3f d = n1.cross( n2 );

   // Compute an index to the largest component of D.
   uint index = d.maxComponent();

   // This is the simplified projection onto L.
   float vp0 = v0( index );
   float vp1 = v1( index );
   float vp2 = v2( index );
  
   float up0 = u0( index );
   float up1 = u1( index );
   float up2 = u2( index );

   // Compute interval for triangle 1.
   Vec2f isect1;
   bool coplanar = computeIntervalsIsectline( vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, isect1 );

   if( coplanar ) return false;

   // Compute interval for triangle 2.
   Vec2f isect2;
   computeIntervalsIsectline( up0, up1, up2, du0, du1, du2, du0du1, du0du2, isect2 );

   sort2( isect1 );
   sort2( isect2 );

   if( isect1.y < isect2.x || isect2.y < isect1.x ) return false;

   // At this point, we know that the triangles intersect.
   return true;
}

NAMESPACE_END
