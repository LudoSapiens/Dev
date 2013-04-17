/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_GEOMETRY_H
#define PLASMA_GEOMETRY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/MaterialSet.h>
#include <Plasma/Animation/Skeleton.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/CollisionGroup.h>
#else
#include <Motion/Collision/CollisionGroup.h>
#endif

#include <Fusion/VM/VM.h>

#include <Gfx/Pass/Pass.h>

NAMESPACE_BEGIN

class DFGeometry;
class LineMeshGeometry;
class MeshGeometry;
class MetaGeometry;
class SilhouetteGeometry;
class SurfaceGeometry;

/*==============================================================================
   CLASS Geometry
==============================================================================*/

class Geometry:
   public RCObject,
   public VMProxy
{
public:

   /*----- types and enumerations ----*/

   enum
   {
      DFGEOMETRY,
      LINEMESH,
      MESH,
      METAGEOMETRY,
      PARTICLE,
      SILHOUETTE,
      SURFACE,
   };

   enum CollisionType
   {
      MANUAL,
      AUTO_BOX,
      AUTO_SPHERE,
      AUTO_CONVEX_HULL,
      AUTO_TRIANGLE_MESH,
   };

   class PatchInfo
   {
   public:

      /*----- methods -----*/

      PatchInfo( uint start, uint size, uint matID ):
         _rangeStart( start ), _rangeSize( size ), _materialID( matID ) {}

      inline uint  rangeStart() const   { return _rangeStart; }
      inline uint  rangeSize()  const   { return _rangeSize;  }
      inline const uint*  range() const { return &_rangeStart; }
      inline uint  materialID() const   { return _materialID; }

      inline void  rangeStart( uint v ) { _rangeStart = v; }
      inline void  rangeSize ( uint v ) { _rangeSize  = v; }
      inline void  materialID( uint v ) { _materialID = v; }

   protected:

      /*----- members -----*/

      uint  _rangeStart;
      uint  _rangeSize;
      uint  _materialID;
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   // Specific geometry types.
   inline int type() const                       { return _type; }
   inline MetaGeometry* metaGeometry() const     { return _type == METAGEOMETRY ? (MetaGeometry*)this : nullptr; }
   inline SurfaceGeometry* surface() const       { return _type == SURFACE ? (SurfaceGeometry*)this : nullptr; }
   inline LineMeshGeometry* linemesh() const     { return _type == LINEMESH ? (LineMeshGeometry*)this : nullptr; }
   inline MeshGeometry* mesh() const             { return _type == MESH ? (MeshGeometry*)this : (_type == LINEMESH ? (MeshGeometry*)this : nullptr); }
   inline SilhouetteGeometry* silhouette() const { return _type == SILHOUETTE ? (SilhouetteGeometry*)this : nullptr; }
   inline DFGeometry* dfGeometry() const         { return _type == DFGEOMETRY ? (DFGeometry*)this : nullptr; }

   // Update.
   inline void guaranteeRenderableGeometry();
   inline void invalidateRenderableGeometry();

   // Collision approximation.
   CollisionType collisionType() const           { return _collisionType; }
   void collisionType( CollisionType t )         { _collisionType = t; }
   inline CollisionShape* collisionShape() const { return _shape.ptr(); }
   void collisionShape( CollisionShape* shape );

   // Rendering.
   inline void render( Gfx::Pass& pass );
   inline void renderPatch( Gfx::Pass& pass, uint );

   inline Material* material( const MaterialSet* m, int patchID ) const;

   // Attributes.
   inline const AABBoxf& boundingBox() const                { return _bbox; }
   inline const Reff& centerOfMass() const                  { return _com; }
   inline const Vec3f& inertiaTensor() const                { return _inertiaTensor; }
   inline uint  numPatches() const                          { return uint(_patches.size()); }
   inline const PatchInfo&  patchInfo( uint patchID ) const { return _patches[patchID]; }

   // Skeleton.
   inline Skeleton* skeleton() const   { return _skeleton.ptr(); }
   inline void skeleton( Skeleton* s ) { _skeleton = s; }
   virtual void computeBonesWeights();

   // VM.
   virtual const char* meta() const;

   PLASMA_DLL_API void print( TextStream& os = StdErr ) const;
   PLASMA_DLL_API void printPatches( TextStream& os = StdErr ) const;

protected:

   /*----- methods -----*/

   Geometry( int type );
   virtual ~Geometry();

   virtual void computeRenderableGeometry() = 0;

   void  clearPatches() { _patches.clear(); }
   void  addPatch( uint start, uint size, uint matID ) { _patches.pushBack( PatchInfo(start, size, matID) ); }

   void updateShape();

   /*----- data members -----*/

   int                 _type;
   CollisionType       _collisionType;
   Vector<PatchInfo>   _patches;
   RCP<Gfx::Geometry>  _rgeom;
   AABBoxf             _bbox;
   Reff                _com;
   Vec3f               _inertiaTensor;
   bool                _rgeomIsOK;

private:

   /*----- data members -----*/

   RCP<CollisionShape> _shape;
   RCP<Skeleton>       _skeleton;
};

//------------------------------------------------------------------------------
//!
inline void
Geometry::invalidateRenderableGeometry()
{
   _rgeomIsOK = false; // Cannot assign _rgeom to nullptr because it might deallocate some Gfx stuff.
}

//------------------------------------------------------------------------------
//!
inline void
Geometry::guaranteeRenderableGeometry()
{
   if( !_rgeomIsOK )
   {
      _rgeom = nullptr;
      computeRenderableGeometry();
      _rgeomIsOK = true;
   }
}

//------------------------------------------------------------------------------
//!
inline void
Geometry::render( Gfx::Pass& pass )
{
   // FIXME: this test could be moved into a manager that could be query once
   // per frame and not once per geometry.
   guaranteeRenderableGeometry();
   pass.execGeometry( _rgeom );
}

//------------------------------------------------------------------------------
//!
inline void
Geometry::renderPatch( Gfx::Pass& pass, uint patch )
{
   guaranteeRenderableGeometry();
   pass.execRangeGeometryPtr( _rgeom, _patches[patch].range() );
}

//------------------------------------------------------------------------------
//!
inline Material*
Geometry::material( const MaterialSet* m, int patchID ) const
{
   return m->material( _patches[patchID].materialID() );
}


NAMESPACE_END

#endif
