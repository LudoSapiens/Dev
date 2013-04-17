/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/Constraint/Constraint.h>
#include <MotionBullet/World/MotionWorld.h>

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS Constraint
==============================================================================*/

//------------------------------------------------------------------------------
//!
Constraint::Constraint() :
   _world(0)
{
}

//------------------------------------------------------------------------------
//!
Constraint::~Constraint()
{
}

//------------------------------------------------------------------------------
//!
void
Constraint::connect( MotionWorld* w )
{
   _world = w;
   btTypedConstraint* constraint = btConstraint();
   if( constraint )
   {
      _world->_world->addConstraint( btConstraint() );
   }
}

//------------------------------------------------------------------------------
//!
void
Constraint::disconnect()
{
   btTypedConstraint* constraint = btConstraint();
   if( constraint )
   {
      _world->_world->removeConstraint( btConstraint() );
   }
   _world = 0;
}

//------------------------------------------------------------------------------
//! 
btRigidBody* 
Constraint::btBody( RigidBody* body )
{
   return body->_body;
}

//------------------------------------------------------------------------------
//! 
btDiscreteDynamicsWorld* 
Constraint::btWorld()
{
   return _world->_world;
}

//------------------------------------------------------------------------------
//! 
void 
Constraint::preStep( double )
{
}

//------------------------------------------------------------------------------
//! 
void 
Constraint::postStep( double )
{
}

NAMESPACE_END
