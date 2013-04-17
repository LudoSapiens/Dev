/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFWORLD_H
#define PLASMA_DFWORLD_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGeometry.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/Probe.h>

#include <CGMath/Ref.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class DFProbeNode;

/*==============================================================================
   CLASS DFEntity
==============================================================================*/

class DFEntity:
   public RCObject
{
public:

   /*----- methods -----*/

   DFEntity();

   const Reff& referential() const     { return _referential; }
   void referential( const Reff& ref ) { _referential = ref; }
   void position( const Vec3f& p )     { _referential.position( p ); }
   void orientation( const Quatf& o )  { _referential.orientation( o ); }
   bool visible() const                { return _visible; }
   void visible( bool v )              { _visible = v; }
   bool castsShadows() const           { return _castsShadows; }
   void castsShadows( bool v )         { _castsShadows = v; }
   bool ghost() const                  { return _ghost; }
   void ghost( bool v )                { _ghost = v; }

   // API for create world entity.
   virtual RCP<Entity> createEntity();

   void initEntity( RigidEntity* );

   /*----- data members -----*/

   int             _bodyType;
   float           _mass;
   float           _friction;
   float           _restitution;
   uint            _exists;
   uint            _senses;
   Reff            _referential;
   ConstString     _id;
   ConstString     _brainProg;
   ConstString     _material;
   RCP<DFGeometry> _geom;
   RCP<Table>      _attributes;
   bool            _visible;
   bool            _castsShadows;
   bool            _ghost;
};

/*==============================================================================
   CLASS DFCamera
==============================================================================*/

//! This class mirrored the definition of the Camera class.

class DFCamera:
   public DFEntity
{
public:

   /*----- methods -----*/

   DFCamera();

   virtual RCP<Entity> createEntity();

   /*----- data members -----*/

   float                  _focalLength;
   float                  _fov;
   float                  _back;
   float                  _front;
   float                  _orthoScale;
   Vec2f                  _shear;
   Camera::ProjectionType _projType;
   Camera::FOVMode        _fovMode;
};

/*==============================================================================
   CLASS DFLight
==============================================================================*/

//! This class mirrored the definition of the Light class.

class DFLight:
   public DFEntity
{
public:

   /*----- methods -----*/

   DFLight();

   virtual RCP<Entity> createEntity();

   /*----- data members -----*/

   Light::Shape  _shape;
   Vec3f         _intensity;

   float         _front;
   float         _back;
   float         _fov;
};

/*==============================================================================
  CLASS DFProbe
==============================================================================*/
class DFProbe:
   public RCObject
{
public:

   /*----- methods -----*/

   inline DFProbe() {}

   inline       Probe::Type  type()     const { return _type; }
   inline       ConstString  id()       const { return _id;   }
   inline      const Vec3f&  position() const { return _pos;  }
   inline              uint  size()     const { return _size; }
   inline const RCP<Image>&  image()    const { return _img;  }

   inline void  type( Probe::Type v )      { _type = v; }
   inline void  id( const ConstString& v ) { _id   = v; }
   inline void  position( const Vec3f& v ) { _pos  = v; }
   inline void  size( uint v )             { _size = v; }
   inline void  image( Image* v )          { _img  = v; }

   inline RCP<Probe>  createProbe() const
   {
      switch( _type )
      {
         case Probe::CUBEMAP:  return new CubemapProbe( _id.cstr(), _pos, _img.ptr() );
         default            :  return nullptr;
      }
   }

   /*----- data members -----*/

   Probe::Type        _type;
   ConstString        _id;
   Vec3f              _pos;
   uint               _size;
   RCP<Image>         _img;  // Use only in some probes.

}; //class DFProbe

/*==============================================================================
   CLASS DFWorld
==============================================================================*/

class DFWorld:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFWorld();

         Vector< RCP<DFEntity> >& entities()       { return _entities; }
   const Vector< RCP<DFEntity> >& entities() const { return _entities; }

         Vector< RCP<DFProbe> >& probes()       { return _probes; }
   const Vector< RCP<DFProbe> >& probes() const { return _probes; }

   PLASMA_DLL_API RCP<World> createWorld();

   PLASMA_DLL_API RCP<DFWorld>  clone() const;

protected:

   /*----- data members -----*/

   Vector< RCP<DFEntity> > _entities;
   Vector< RCP<DFProbe> >  _probes;
};

NAMESPACE_END

#endif
