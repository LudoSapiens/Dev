/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_CONSTRAINT_H
#define MOTION_CONSTRAINT_H

#include <Motion/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


class MotionWorld;

/*==============================================================================
   CLASS Constraint
==============================================================================*/
//! Base class for handling constraints on rigid bodies.

class MOTION_DLL_API Constraint
   : public RCObject
{

public: 

   /*----- methods -----*/

   virtual void prePositionStep( double step ) = 0;
   virtual bool solvePosition( double step ) = 0;

   virtual void preVelocitiesStep() = 0;
   virtual bool solveVelocities() = 0;

protected: 
   
   /*----- methods -----*/

   Constraint( MotionWorld* w = 0 );
   virtual ~Constraint();

   friend class MotionWorld;
   void connect( MotionWorld* w );
   void disconnect();

   /*----- data members -----*/

   MotionWorld* _world;
};

NAMESPACE_END

#endif
