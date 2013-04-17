/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_ATTRACTOR_H
#define MOTIONBULLET_ATTRACTOR_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/World/RigidBody.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN


class MotionWorld;

/*==============================================================================
   CLASS Attractor
==============================================================================*/
//!
class Attractor
   : public RCObject
{

public:

   /*----- methods -----*/
   MOTION_DLL_API Attractor();

   MOTION_DLL_API virtual void addForce( const Vector< RCP<RigidBody> >& ) = 0;

   // Attraction category.
   inline uint  attractionMask() const         { return _attractionMask; }
   inline void  attractionMask( uint c )       { _attractionMask = c; }
   inline void  addAttractionWith( uint c )    { _attractionMask |= c; }
   inline void  removeAttractionWith( uint c ) { _attractionMask &= ~c; }

   inline bool attracts( const RCP<RigidBody>& body ) const { return (_attractionMask & body->attractionCategories()) != 0; }

   inline MotionWorld* world() const { return _world; }

protected:

   /*----- methods -----*/

   MOTION_DLL_API virtual ~Attractor();

   friend class MotionWorld;
   inline void connect( MotionWorld* w ) { _world = w; }
   inline void disconnect() { _world = NULL; }

   /*----- data members -----*/

   MotionWorld* _world;
   uint         _attractionMask;
};

NAMESPACE_END

#endif
