/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLANEPROJECTOR_H
#define PLASMA_PLANEPROJECTOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Renderable/Renderable.h>

NAMESPACE_BEGIN

class Camera;
class Material;

/*==============================================================================
   CLASS PlaneProjector
==============================================================================*/

class PlaneProjector :
   public Renderable
{

public:

   /*----- types and enumerations ----*/

   /*----- methods -----*/

   PlaneProjector();

   virtual void render( const RCP<Gfx::Pass>& ) const;

   inline void orientation( Quatf& );
   inline void cameraSpace( bool );
   inline bool cameraSpace() const;

   inline void position( const Vec3f& pos );
   inline const Vec3f& position() const;

   inline void pickPosition( const Vec3f& pos );
   inline const Vec3f& pickPosition() const;

   inline void camera( const RCP<Camera>& cam );

   Vec3f project( const Vec2i& pos ) const;

protected:

   /*----- methods -----*/

   virtual ~PlaneProjector();

private: 

   /*----- data members -----*/

   bool                       _cameraSpace;
   Vec3f                      _pick;
   Reff                       _ref;
   RCP<Camera>                _camera;
   mutable Mat4f              _transform;
   mutable RCP<Gfx::Geometry> _geom;
   mutable RCP<Material>      _visMaterial;
   mutable RCP<Material>      _hiddenMaterial;
};

//------------------------------------------------------------------------------
//!
inline void
PlaneProjector::orientation( Quatf& orient )
{
   _ref.orientation( orient );
}

//------------------------------------------------------------------------------
//!
inline void
PlaneProjector::cameraSpace( bool enable )
{
   _cameraSpace = enable;
}

//------------------------------------------------------------------------------
//!
inline bool
PlaneProjector::cameraSpace() const
{
   return _cameraSpace;
}

//------------------------------------------------------------------------------
//!
inline void 
PlaneProjector::position( const Vec3f& pos ) 
{ 
   _ref.position( pos );
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
PlaneProjector::position() const
{
   return _ref.position();
}

//------------------------------------------------------------------------------
//!
inline void 
PlaneProjector::pickPosition( const Vec3f& pos ) 
{ 
   _pick = pos;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
PlaneProjector::pickPosition() const
{
   return _pick;
}

//------------------------------------------------------------------------------
//!
inline void
PlaneProjector::camera( const RCP<Camera>& cam )
{
   _camera = cam;
}


NAMESPACE_END

#endif
