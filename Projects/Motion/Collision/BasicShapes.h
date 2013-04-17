/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_BASICSHAPES_H
#define MOTION_BASICSHAPES_H

#include <Motion/StdDefs.h>
#include <Motion/Collision/CollisionShape.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN


/*==============================================================================
   CLASS SphereShape
==============================================================================*/
//!
class SphereShape
   : public CollisionShape
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API SphereShape( float r );

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

   // Attributes.
   inline float radius() const;

protected: 
   
   /*----- methods -----*/

   virtual ~SphereShape();

   /*----- data members -----*/

   float _radius;
};

//------------------------------------------------------------------------------
//!
inline float
SphereShape::radius() const
{
   return _radius;
}

/*==============================================================================
   CLASS BoxShape
==============================================================================*/
//!
class BoxShape
   : public CollisionShape
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API BoxShape( float x, float y, float z );
   MOTION_DLL_API BoxShape( const Vec3f& s );

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

   // Attributes.
   inline const Vec3f& size() const;

protected: 
   
   /*----- methods -----*/

   virtual ~BoxShape();

   void updateMargin();

   /*----- data members -----*/

   Vec3f _size;
};

//------------------------------------------------------------------------------
//!
inline const Vec3f& 
BoxShape::size() const
{
   return _size;
}

/*==============================================================================
   CLASS CylinderShape
==============================================================================*/
//! Radius is in plane XZ.
class CylinderShape
   : public CollisionShape
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API CylinderShape( float r, float height );

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

   // Attributes.
   inline float radius() const;

   inline float height() const;

protected: 
   
   /*----- methods -----*/

   virtual ~CylinderShape();

   /*----- data members -----*/

   float _radius;
   float _height;
};

//------------------------------------------------------------------------------
//!
inline float
CylinderShape::radius() const
{
   return _radius;
}

//------------------------------------------------------------------------------
//!
inline float 
CylinderShape::height() const
{
   return _height;
}

/*==============================================================================
   CLASS ConeShape
==============================================================================*/
//!
class ConeShape
   : public CollisionShape
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API ConeShape( float r, float h );

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

   // Attributes.
   inline float radius() const;

   inline float height() const;

protected: 
   
   /*----- methods -----*/

   virtual ~ConeShape();

   inline void updateSinApex();

   /*----- data members -----*/

   float _radius;
   float _height;
   float _sinApex;
};

//------------------------------------------------------------------------------
//!
inline float
ConeShape::radius() const
{
   return _radius;
}

//------------------------------------------------------------------------------
//!
inline float 
ConeShape::height() const
{
   return _height;
}

//------------------------------------------------------------------------------
//!
inline void 
ConeShape::updateSinApex()
{
   _sinApex = _radius / CGM::sqrt( _radius*_radius + _height*_height );
}

/*==============================================================================
   CLASS ConvexHullShape
==============================================================================*/
//!
class ConvexHullShape
   : public CollisionShape
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API ConvexHullShape();

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

   // Building.
   inline void addVertex( const Vec3f& );
   inline void reserveVertices( uint qty );   
   inline void clearVertices();

   // Accessors.
   inline const Vec3f& vertex( uint id ) const;
   inline uint numVertices() const;

protected: 
   
   /*----- methods -----*/

   virtual ~ConvexHullShape();

   /*----- data members -----*/

   Vector<Vec3f> _vertices;
};

//------------------------------------------------------------------------------
//!
inline void
ConvexHullShape::addVertex( const Vec3f& pt )
{
   if( _vertices.empty() )
   {
      _bbox.set( pt );
   }
   else
   {
      _bbox |= pt;
   }
   _vertices.pushBack( pt );
}

//------------------------------------------------------------------------------
//!
inline void
ConvexHullShape::reserveVertices( uint qty )
{
   _vertices.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
ConvexHullShape::clearVertices()
{
   _vertices.clear();
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
ConvexHullShape::vertex( uint id ) const
{
   return _vertices[id];
}

//------------------------------------------------------------------------------
//!
inline uint
ConvexHullShape::numVertices() const
{
   return (uint)_vertices.size();
}

/*==============================================================================
   CLASS SphereHullShape
==============================================================================*/
//!
class SphereHullShape
   : public CollisionShape
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API SphereHullShape();
   MOTION_DLL_API SphereHullShape( const float* sphereData, const uint nSpheres );

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

   // Building.
   inline void addSphere( const Vec3f&, float radius );
   inline void addSphere( const Vec4f& );
   inline void reserveSpheres( uint qty );   
   inline void clearSpheres();

   // Accessors.
   inline const Vec4f& sphere( uint id ) const;
   inline uint numSpheres() const;

protected: 
   
   /*----- methods -----*/

   virtual ~SphereHullShape();

   /*----- data members -----*/

   Vector<Vec4f> _spheres;
};

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::addSphere( const Vec3f& pt, float radius )
{
   if( _spheres.empty() )
   {
      _bbox.set( pt.x-radius, pt.x+radius, pt.y-radius, pt.y+radius, pt.z-radius, pt.z+radius );
   }
   else
   {
      _bbox |= AABBoxf( pt.x-radius, pt.x+radius, pt.y-radius, pt.y+radius, pt.z-radius, pt.z+radius );
   }
   
   _spheres.pushBack( Vec4f( pt, radius ) );
   CGM::clampMax( _margin, radius );
}

//------------------------------------------------------------------------------
//! 
inline void
SphereHullShape::addSphere( const Vec4f& sphere )
{
   addSphere( Vec3f(sphere.x, sphere.y, sphere.z), sphere.w );
}

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::reserveSpheres( uint qty )
{
   _spheres.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::clearSpheres()
{
   _spheres.clear();
   _margin = CGConstf::max();
}

//------------------------------------------------------------------------------
//!
inline const Vec4f&
SphereHullShape::sphere( uint id ) const
{
   return _spheres[id];
}

//------------------------------------------------------------------------------
//!
inline uint
SphereHullShape::numSpheres() const
{
   return (uint)_spheres.size();
}

/*==============================================================================
  CLASS TriangleMeshShape
==============================================================================*/
class TriangleMeshShape:
   public CollisionShape
{
public:

   /*----- methods -----*/

   MOTION_DLL_API TriangleMeshShape(
      uint numTriangles, const uint*  indices,  size_t iStride,
      uint numVertices,  const float* vertices, size_t vStride
   );

   MOTION_DLL_API void  setIndices( uint numTriangles, const uint* indices, size_t stride );
   MOTION_DLL_API void  setVertices( uint numVertices, const float* vertices, size_t stride );

   MOTION_DLL_API virtual Vec3f getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

protected:

   /*----- data members -----*/

   Vector<uint>   _indices;
   Vector<Vec3f>  _vertices;

   /*----- methods -----*/

   virtual ~TriangleMeshShape();

private:
}; //class TriangleMeshShape

NAMESPACE_END

#endif
