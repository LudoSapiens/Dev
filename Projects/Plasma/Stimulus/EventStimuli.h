/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_EVENT_STIMULI_H
#define PLASMA_EVENT_STIMULI_H

#include <Plasma/StdDefs.h>

#include <Plasma/Action/Action.h>
#include <Plasma/Stimulus/Stimulus.h>
#include <Plasma/World/Entity.h>

#include <Fusion/VM/VM.h>

NAMESPACE_BEGIN

namespace EventStimuli
{

   PLASMA_DLL_API void initialize();
   PLASMA_DLL_API void terminate();

}


/*==============================================================================
  CLASS ActionCompleted
==============================================================================*/
//! Indicates that an entity's action completed.
class ActionCompleted:
   public Stimulus
{
public:

   /*----- methods -----*/

   ActionCompleted() {}
   ActionCompleted( Action* action ): _action( action ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const RCP<Action>&  action() const { return _action; }
   inline void  action( const RCP<Action>& a ) { _action = a; }

protected:

   /*----- data members -----*/

   RCP<Action>  _action;  //!< The action that completed.

private:
};


/*==============================================================================
  CLASS BeginStimulus
==============================================================================*/
//! Indicates that the brain has started processing.
class BeginStimulus:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};


/*==============================================================================
  CLASS ContactBeginStimulus
==============================================================================*/
//! Indicates that 2 entities start touching one another, along with a location and force.
class ContactBeginStimulus:
   public Stimulus
{
public:

   /*----- methods -----*/

   ContactBeginStimulus() {}
   ContactBeginStimulus( Entity* receiving, Entity* colliding, const Vec3f& location, float force ):
      _receiving( receiving ), _colliding( colliding ), _location( location ), _force( force ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const RCP<Entity>&  receiving() const { return _receiving; }
   inline void  receiving( const RCP<Entity>& e ) { _receiving = e; }

   inline const RCP<Entity>&  colliding() const { return _colliding; }
   inline void  colliding( const RCP<Entity>& e ) { _colliding = e; }

   inline const Vec3f&  location() const { return _location; }
   inline void  location( const Vec3f& v ) { _location = v; }

   inline float  force() const { return _force; }
   inline void  force( float v ) { _force = v; }

protected:

   /*----- data members -----*/

   RCP<Entity>  _receiving;
   RCP<Entity>  _colliding;
   Vec3f        _location;
   float        _force;

private:
};


/*==============================================================================
  CLASS ContactEndStimulus
==============================================================================*/
//! Indicate 2 entities stop touching each other.
class ContactEndStimulus:
   public Stimulus
{
public:

   /*----- methods -----*/

   ContactEndStimulus() {}
   ContactEndStimulus( Entity* receiving, Entity* colliding ):
      _receiving( receiving ), _colliding( colliding ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const RCP<Entity>&  receiving() const { return _receiving; }
   inline void  receiving( const RCP<Entity>& e ) { _receiving = e; }

   inline const RCP<Entity>&  colliding() const { return _colliding; }
   inline void  colliding( const RCP<Entity>& e ) { _colliding = e; }

protected:

   /*----- data members -----*/

   RCP<Entity>  _receiving;
   RCP<Entity>  _colliding;

private:
};

/*==============================================================================
  CLASS FallStimulus
==============================================================================*/
class FallStimulus:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};


/*==============================================================================
  CLASS LandStimulus
==============================================================================*/
class LandStimulus:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};


NAMESPACE_END

#endif //PLASMA_EVENT_STIMULI_H
