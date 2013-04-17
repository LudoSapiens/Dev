/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BRAIN_H
#define PLASMA_BRAIN_H

#include <Plasma/StdDefs.h>

#include <Plasma/Action/Action.h>
#include <Plasma/Action/Command.h>
#include <Plasma/Stimulus/Stimulus.h>
#include <Plasma/World/Receptor.h>

#include <Fusion/Resource/Resource.h>
#include <Fusion/VM/VM.h>

#include <CGMath/Variant.h>

#include <Base/ADT/Vector.h>
#include <Base/Dbg/Defs.h>
#include <Base/Msg/Delegate.h>
#include <Base/MT/Lock.h>
#include <Base/MT/Task.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class Entity;
//class Stimulus;
class World;

typedef Delegate1< Entity* >  BrainCallback;

/*==============================================================================
  CLASS StimulusQueue
==============================================================================*/
// A thread-safe queue for posting stimuli to brains.
class StimulusQueue
{
public:

   typedef Vector< RCP<Stimulus> >  Container;

   /*----- methods -----*/

   PLASMA_DLL_API  StimulusQueue();
   PLASMA_DLL_API  ~StimulusQueue();

   PLASMA_DLL_API void  post( Stimulus* s );

   PLASMA_DLL_API void  drain( Container& dst );

   inline bool  empty() const { return _incoming.empty(); }

protected:

   /*----- data members -----*/

   Container  _incoming; // The stimuli posted for the next brain processing.
   Lock       _lock;

private:
}; //class StimulusQueue


/*==============================================================================
  CLASS BrainProgram
==============================================================================*/
class BrainProgram:
   public RCObject
{
public:

   /*----- methods -----*/

   inline BrainProgram() {}
   //inline BrainProgram( const VMByteCode& byteCode ): _byteCode(byteCode) {}
   inline BrainProgram( const BrainCallback& progFunc ): _func(progFunc) {}

   inline const VMByteCode&  byteCode() const { return _byteCode; }

   inline const BrainCallback&  func() const { return _func; }

protected:

   friend class BrainProgramLoadTask;

   /*----- data members -----*/

   VMByteCode     _byteCode;  //!< The cached byte code.
   BrainCallback  _func;      //!< A delegate for a C++ callback.

private:
}; //class BrainProgram


/*==============================================================================
  CLASS BrainProgramLoadTask
==============================================================================*/
class BrainProgramLoadTask:
   public Task
{
public:

   /*----- methods -----*/

   inline BrainProgramLoadTask( Resource<BrainProgram>* progRes, const String& path ):
      _progRes( progRes ), _path( path ) {}

   virtual void execute();

protected:

   /*----- data members -----*/

   RCP< Resource<BrainProgram> >  _progRes; //!< A pointer to the brain program.
   String                         _path;    //!< The path of the brain program's source.

private:
}; //class BrainTask


/*==============================================================================
  CLASS Brain
==============================================================================*/
class Brain:
   public RCObject
{
public:

   /*----- types -----*/

   typedef Vector< RCP<Action> >    ActionContainer;
   typedef Vector< RCP<Receptor> >  ReceptorContainer;

   /*----- static methods -----*/

   static void  initialize();
   static void  initVMs();
   static void  termVMs();
   static void  handlePostedCommands( World* world );
   static void  markEntitiesForAction( World* world );

   /*----- methods -----*/

   inline Brain() {}
   PLASMA_DLL_API  ~Brain();

   inline const BrainProgram*  program() const { return _prog.ptr(); }
   inline BrainProgram*  program()             { return _prog.ptr(); }
   inline void program( BrainProgram* p )      { _prog = p; }

   inline       ReceptorContainer&  receptors()       { return _receptors; }
   inline const ReceptorContainer&  receptors() const { return _receptors; }

   //inline       StimulusQueue&  stimuli()       { return _stimuli; }
   //inline const StimulusQueue&  stimuli() const { return _stimuli; }
   inline bool  hasStimuli() const { return !_stimuli.empty(); }

   inline       ActionContainer&  actions()          { return _actions; }
   inline const ActionContainer&  actions()    const { return _actions; }
   inline                   bool  hasActions() const { return !_actions.empty(); }

   PLASMA_DLL_API Action*  findAction( const ConstString& str ) const;

   void  postBegin();

protected:

   friend class BrainTask;
   friend class Entity;
   friend class World;

   /*----- data members -----*/

   ReceptorContainer  _receptors; //!< The receptors connected to the brain.
   StimulusQueue      _stimuli;   //!< The stimuli currently posted for the brain.
   RCP<BrainProgram>  _prog;      //!< The program doing the brain's processing.
   ActionContainer    _actions;   //!< The actions currently managed by the brain.

private:
}; //class Brain


/*==============================================================================
  CLASS BrainTask
==============================================================================*/
class BrainTask:
   public Task
{
public:

   /*----- methods -----*/

   BrainTask( Entity* e );

   virtual void execute();

protected:

   /*----- data members -----*/

   RCP<Entity>  _entity;  //!< A pointer to the entity having the brain.

private:
}; //class BrainTask


/*==============================================================================
  CLASS BrainTaskContext
==============================================================================*/
class BrainTaskContext
{
public:

   /*----- types -----*/

   typedef Vector< Command >  CommandContainer;
   typedef Vector< Entity* >  EntityContainer;

   /*----- methods -----*/

   inline       Entity*  entity()       { return _entity; }
   inline const Entity*  entity() const { return _entity; }

   inline       CommandContainer&  commands()       { return _commands; }
   inline const CommandContainer&  commands() const { return _commands; }

   inline       EntityContainer&  entitiesWithActionsBefore()       { return _entitiesWithActionsBefore; }
   inline const EntityContainer&  entitiesWithActionsBefore() const { return _entitiesWithActionsBefore; }

   inline       EntityContainer&  entitiesWithActionsAfter()       { return _entitiesWithActionsAfter; }
   inline const EntityContainer&  entitiesWithActionsAfter() const { return _entitiesWithActionsAfter; }

   inline       EntityContainer&  entitiesWithActions( bool after )       { return after ? _entitiesWithActionsAfter : _entitiesWithActionsBefore; }
   inline const EntityContainer&  entitiesWithActions( bool after ) const { return after ? _entitiesWithActionsAfter : _entitiesWithActionsBefore; }

protected:

   friend class BrainTask;

   inline void  entity( Entity* e ) { _entity = e; }

   /*----- data members -----*/

   Entity*           _entity;
   CommandContainer  _commands;
   EntityContainer   _entitiesWithActionsBefore;
   EntityContainer   _entitiesWithActionsAfter;

private:
}; //class BrainTask


NAMESPACE_END

#endif //PLASMA_BRAIN_H
