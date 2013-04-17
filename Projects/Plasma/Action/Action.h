/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_ACTION_H
#define PLASMA_ACTION_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/MT/Task.h>
#include <Base/Util/Bits.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class Entity;

/*==============================================================================
  CLASS Action
==============================================================================*/
class Action:
   public RCObject,
   public VMProxy
{
public:

   /*----- types -----*/

   typedef Action* (*CtorFunc)();

   /*----- static methods -----*/

   static PLASMA_DLL_API void  initialize();
   static PLASMA_DLL_API void  terminate();

   static PLASMA_DLL_API void  registerAction( const ConstString& type, CtorFunc ctorFunc );

   static Action*  create( const ConstString& type );

   /*----- methods -----*/

   PLASMA_DLL_API virtual const ConstString&  type() const = 0;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta ) = 0;

   inline bool  enabled() const              { return getbits(_state, 0, 1) != 0; }
   inline void  enabled( bool v )            { setbits(&_state, 0, 1, v?1:0);     }

   inline bool  autoDelete() const           { return getbits(_state, 1, 1) != 0; }
   inline void  autoDelete( bool v )         { setbits(&_state, 1, 1, v?1:0);     }

   inline bool  notifyOnCompletion() const   { return getbits(_state, 2, 1) != 0; }
   inline void  notifyOnCompletion( bool v ) { setbits(&_state, 2, 1, v?1:0);     }

   inline bool  runAfterPhysics() const      { return getbits(_state, 3, 1) != 0; }

   // VM (not needed in subclasses).
   virtual const char*  meta() const;
   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   friend class ActionTask;
   friend class Brain;

   /*----- types -----*/

   enum {
      STATE_ENABLED              = 0x01,
      STATE_AUTO_DELETE          = 0x02,
      STATE_NOTIFY_ON_COMPLETION = 0x04,
      STATE_RUN_AFTER_PHYSICS    = 0x08,
      STATE_DEFAULT              = STATE_ENABLED|STATE_AUTO_DELETE,
   };

   /*----- members -----*/

   /**
    * _state[0]  Enabled (or disabled)
    * _state[1]  Auto-delete when done (or keep around)
    * _state[2]  Notify brain when completed (or do not send any stimulus)
    * _state[3]  Run the action after the physics (but before rendering)
    */
   uint8_t  _state;  //!< A flag indicating if the action is enabled or disabled, .

   /*----- methods -----*/

   Action( int state = STATE_DEFAULT ): _state( state ) {}
   PLASMA_DLL_API virtual ~Action();

   inline bool  canExecute() { return (_state & STATE_ENABLED) == STATE_ENABLED; }

}; //class Action


/*==============================================================================
  CLASS DebugAction
==============================================================================*/
class DebugAction:
   public Action
{
public:

   /*----- methods -----*/

   DebugAction();

   PLASMA_DLL_API static  const ConstString&  actionType();
   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:
   RCP<Table>  _attr;

}; //class DebugAction

/*==============================================================================
  CLASS ParticleAction
==============================================================================*/
class ParticleAction:
   public Action
{
public:

   /*----- methods -----*/

   ParticleAction();

   PLASMA_DLL_API static  const ConstString&  actionType();
   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

protected:

   float  _duration;

};

/*==============================================================================
  CLASS WaitAction
==============================================================================*/
class WaitAction:
   public Action
{
public:

   /*----- methods -----*/

   WaitAction();

   PLASMA_DLL_API static  const ConstString&  actionType();
   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   float  _duration;

}; //class WaitAction


/*==============================================================================
  CLASS ActionTask
==============================================================================*/
class ActionTask:
   public Task
{
public:

   /*----- methods -----*/

   ActionTask( Entity* e, double t, double d, bool afterPhysics );
   virtual void  execute();

protected:

   /*----- data members -----*/

   RCP<Entity>  _entity; //!< A pointer to the entity that has actions to run.
   double       _time;   //!< The current world time.
   double       _delta;  //!< The delta time since the last action call.
   // Note: if _time and _delta take too much memory, we could poll them from the world.
   bool         _afterPhysics; //!< Only handle actions that have a matching runAfterPhysics() value.

private:
}; //class ActionTask


NAMESPACE_END

#endif //PLASMA_ACTION_H
