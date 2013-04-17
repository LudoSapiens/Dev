/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLASMASCREEN_H
#define PLASMA_PLASMASCREEN_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Viewport.h>

#include <Fusion/Widget/HotspotContainer.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

class ManipulatorGroup;
class CameraManipulator;
class Renderer;
class World;

/*==============================================================================
   CLASS PlasmaScreen
==============================================================================*/

class PlasmaScreen:
   public HotspotContainer
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaScreen();
   virtual ~PlasmaScreen();

   PLASMA_DLL_API void renderSize( const Vec2i& );

   inline World* world()                   { return _viewport.world(); }
   PLASMA_DLL_API void world( World* );

   PLASMA_DLL_API void camera( int );

   inline       Viewport& viewport()       { return _viewport; }
   inline const Viewport& viewport() const { return _viewport; }

   inline       ManipulatorGroup* manipulators()       { return _manips.ptr(); }
   inline const ManipulatorGroup* manipulators() const { return _manips.ptr(); }

   // Widget API.
   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>& );
   PLASMA_DLL_API virtual void performSetGeometry();
   PLASMA_DLL_API virtual void performSetPosition();

   // Events.
   PLASMA_DLL_API virtual void onPointerPress( const Event& );
   PLASMA_DLL_API virtual void onPointerRelease( const Event& );
   PLASMA_DLL_API virtual void onPointerMove( const Event& );
   PLASMA_DLL_API virtual void onPointerCancel( const Event& );
   PLASMA_DLL_API virtual void onPointerScroll( const Event& );
   PLASMA_DLL_API virtual void onKeyPress( const Event& );
   PLASMA_DLL_API virtual void onKeyRelease( const Event& );
   PLASMA_DLL_API virtual void onChar( const Event& );
   PLASMA_DLL_API virtual void onAccelerate( const Event& );

   // VM.
   const char* meta() const;
   PLASMA_DLL_API void init( VMState* vm );
   PLASMA_DLL_API bool performGet( VMState* );
   PLASMA_DLL_API bool performSet( VMState* );
   PLASMA_DLL_API virtual bool isAttribute( const char* ) const;

   // Callbacks.
   void closeWorldCb( Resource<World>* );

protected:

   /*----- members -----*/

   void initCameras();
   void setCamera();

   /*----- data members -----*/

   bool                   _autoSize;
   int                    _cameraId;
   Viewport               _viewport;
   RCP<Renderer>          _renderer;
   RCP<ManipulatorGroup>  _manips;
   Vector< RCP<Camera> >  _cameras;
};

NAMESPACE_END

#endif

