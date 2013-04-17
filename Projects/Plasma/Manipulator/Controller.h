/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_CONTROLLER_H
#define PLASMA_CONTROLLER_H

#include <Plasma/StdDefs.h>

#include <Plasma/Manipulator/EventDispatcher.h>
#include <Plasma/Manipulator/Manipulator.h>

NAMESPACE_BEGIN

class Entity;
class RigidEntity;

/*==============================================================================
  CLASS Controller
==============================================================================*/
class Controller:
   public Manipulator
{
public:

   /*----- methods -----*/

   static void  initialize();
   static void  terminate();

   PLASMA_DLL_API Controller();
   PLASMA_DLL_API virtual ~Controller();

   inline       EventDispatcher&  dispatcher()       { return _dispatcher; }
   inline const EventDispatcher&  dispatcher() const { return _dispatcher; }

   //PLASMA_DLL_API virtual void  onCameraChange();
   //PLASMA_DLL_API virtual void  onViewportChange();

   PLASMA_DLL_API virtual bool  onPointerPress( const Event& ev );
   PLASMA_DLL_API virtual bool  onPointerRelease( const Event& ev );
   PLASMA_DLL_API virtual bool  onPointerMove( const Event& ev );
   PLASMA_DLL_API virtual bool  onPointerCancel( const Event& ev );
   PLASMA_DLL_API virtual bool  onPointerScroll( const Event& ev );
   PLASMA_DLL_API virtual bool  onKeyPress( const Event& ev );
   PLASMA_DLL_API virtual bool  onKeyRelease( const Event& ev );
   PLASMA_DLL_API virtual bool  onChar( const Event& ev );
   PLASMA_DLL_API virtual bool  onAccelerate( const Event& ev );

   PLASMA_DLL_API void  enableHID();
   PLASMA_DLL_API void  disableHID();
   PLASMA_DLL_API virtual void  onHIDEvent( const Event& ev );


   // VM.
   //PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   //PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   /*----- data members -----*/

   EventDispatcher  _dispatcher;  //!< An event dispatcher.

private:
}; //class Controller


/*==============================================================================
  CLASS EntityController
==============================================================================*/
class EntityController:
   public Controller
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API EntityController();
   PLASMA_DLL_API virtual ~EntityController();

   inline       RigidEntity*  entity()       { return _entity; }
   inline const RigidEntity*  entity() const { return _entity; }
   inline void  entity( RigidEntity* e ) { _entity = e; }

   // VM.
   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

   PLASMA_DLL_API void  entityPickEvent();
   PLASMA_DLL_API void  anyEvent() const;

   PLASMA_DLL_API virtual void  onViewportChange();

protected:

   /*----- data members -----*/

   RigidEntity*  _entity;  //!< The entity being controlled.

   /*----- methods -----*/

private:
}; //class EntityController


NAMESPACE_END

#endif //PLASMA_CONTROLLER_H
