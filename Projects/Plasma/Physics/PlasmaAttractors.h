/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLASMAATTRACTORS_H
#define PLASMA_PLASMAATTRACTORS_H

#include <Plasma/StdDefs.h>

#if MOTION_BULLET
#include <MotionBullet/Attractor/BasicAttractors.h>
#include <MotionBullet/Attractor/GravitationalAttractor.h>
#else
#include <Motion/Attractor/BasicAttractors.h>
#include <Motion/Attractor/GravitationalAttractor.h>
#endif

#include <Fusion/VM/VMObject.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>


NAMESPACE_BEGIN


namespace Attractors
{

   PLASMA_DLL_API void initialize( VMState* );

} // Attractors


/*==============================================================================
  CLASS PlasmaDirectionalAttractor
==============================================================================*/
class PlasmaDirectionalAttractor:
   public DirectionalAttractor
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaDirectionalAttractor();
   PLASMA_DLL_API PlasmaDirectionalAttractor( const Vec3f& acceleration );

   // VM.
   PLASMA_DLL_API void  init( VMState* );
   PLASMA_DLL_API bool  performGet( VMState* );
   PLASMA_DLL_API bool  performSet( VMState* );

protected: 

   /*----- methods -----*/

   virtual ~PlasmaDirectionalAttractor();

}; //PlasmaDirectionalAttractor

//------------------------------------------------------------------------------
//! VM Section
VMOBJECT_TRAITS( PlasmaDirectionalAttractor, directional )
typedef VMObject< PlasmaDirectionalAttractor >  PlasmaDirectionalAttractorVM;


/*==============================================================================
  CLASS PlasmaGravitationalAttractor
==============================================================================*/
class PlasmaGravitationalAttractor:
   public GravitationalAttractor
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaGravitationalAttractor();
   PLASMA_DLL_API PlasmaGravitationalAttractor( const float threshold );

   // VM.
   PLASMA_DLL_API void  init( VMState* );
   PLASMA_DLL_API bool  performGet( VMState* );
   PLASMA_DLL_API bool  performSet( VMState* );

protected: 

   /*----- methods -----*/

   virtual ~PlasmaGravitationalAttractor();

}; //PlasmaGravitationalAttractor

//------------------------------------------------------------------------------
//! VM Section
VMOBJECT_TRAITS( PlasmaGravitationalAttractor, gravitational )
typedef VMObject< PlasmaGravitationalAttractor >  PlasmaGravitationalAttractorVM;


NAMESPACE_END

#endif //PLASMA_PLASMAATTRACTORS_H
