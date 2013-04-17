/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLASMA_H
#define PLASMA_PLASMA_H

#include <Plasma/StdDefs.h>

#include <Fusion/Fusion.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/Application.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class TaskQueue;
class SoundManager;
class World;

/*==============================================================================
  CLASS Plasma
==============================================================================*/

//!

class Plasma
{

public:

   /*----- static methods -----*/

   PLASMA_DLL_API static void init();
   PLASMA_DLL_API static void exec();
   PLASMA_DLL_API static void term();

   // Sound API.
   PLASMA_DLL_API static const RCP<SoundManager>&  soundManager();

   PLASMA_DLL_API Plasma( int argc, char* argv[] );
   PLASMA_DLL_API virtual ~Plasma();

   static inline TaskQueue*  dispatchQueue()     { return _dispatchQueue; }

   // Global parameters.
   static inline bool  showWireframe()           { return _wireframe; }
   static inline void  showWireframe( bool v )   { _wireframe = v; }

   static inline bool  debugGeometry()           { return _dbgGeom; }
   static inline void  debugGeometry( bool v )   { _dbgGeom = v; }

   static inline float geometricError()          { return _geometricError; }
   static inline void  geometricError( float e ) { _geometricError = e; }

   static inline float physicsFPS()              { return _physicsFPS; }
   static inline void  physicsFPS( float v )     { _physicsFPS = v; }

   static inline float renderingFPS()            { return _renderingFPS; }
   static inline void  renderingFPS( float v )   { _renderingFPS = v; }

private:

   static TaskQueue*  _dispatchQueue;
   static bool        _wireframe;
   static bool        _dbgGeom;
   static float       _geometricError;
   static float       _physicsFPS;
   static float       _renderingFPS;
};


/*==============================================================================
  CLASS PlasmaApp
==============================================================================*/
class PlasmaApp:
   public FusionApp
{
public:

   /*----- methods -----*/

   static PLASMA_DLL_API RCP<PlasmaApp>  create( int argc, char* argv[] );

   PLASMA_DLL_API ~PlasmaApp();

   PLASMA_DLL_API virtual void  printInfo( TextStream& os ) const;

protected:

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaApp( int argc, char* argv[], const String& name = "Plasma" );

private:
}; //class PlasmaApp



NAMESPACE_END

#endif

