/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PARTICLE_GEOMETRY_H
#define PLASMA_PARTICLE_GEOMETRY_H

#include <Plasma/StdDefs.h>

#include <Plasma/Geometry/Geometry.h>

NAMESPACE_BEGIN

class ParticleEntity;

/*==============================================================================
  CLASS ParticleGeometry
==============================================================================*/
class ParticleGeometry:
   public Geometry
{
public:

   /*----- static methods -----*/

   /*----- methods -----*/

   PLASMA_DLL_API ParticleGeometry( ParticleEntity* e );

   inline const ParticleEntity* entity() const { return _entity; }

   PLASMA_DLL_API bool  updateRenderData();

protected:

   /*----- methods -----*/

   virtual void  computeRenderableGeometry();

   /*----- members -----*/

   ParticleEntity*  _entity;

private:
};


NAMESPACE_END

#endif //PLASMA_PARTICLE_GEOMETRY_H
